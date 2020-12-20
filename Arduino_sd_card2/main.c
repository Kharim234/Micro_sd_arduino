/*----------------------------------------------------------------------*/
/* Foolproof FatFs sample project for AVR              (C)ChaN, 2014    */
/*----------------------------------------------------------------------*/

#include <avr/io.h>	/* Device specific declarations */
#include <util/delay.h>
#include "ff.h"		/* Declarations of FatFs API */
#include <stdlib.h>
#include <avr/interrupt.h>
#include <string.h>

FATFS FatFs;		/* FatFs work area needed for each volume */
FIL Fil;			/* File object needed for each open file */

#define LED_PIN (1<<PB5)
#define INIT_LED_PIN() DDRB |= LED_PIN
#define SET_LED_ON() (PORTB |= LED_PIN)
#define SET_LED_OFF() (PORTB &= ~LED_PIN)

char Buffer_string[256];
volatile uint16_t test = 0;
volatile uint32_t licznik_32bit = 0;
volatile uint16_t licznik;
volatile uint8_t flag= 0;
volatile uint16_t adc_result;

void USART_Init( unsigned int ubrr)
{
	/*Set baud rate */
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	
	UCSR0A |= (1<<U2X0);
	/*Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	/* Set frame format: 8data, 2stop bit */
	UCSR0C = /*(1<<USBS0)|*/(3<<UCSZ00);
}

void USART_Init_Baud_Rate (unsigned  long Baud_rate){
	unsigned int UBRR;
	UBRR =  ((F_CPU / 8UL) /  Baud_rate ) - 1UL;
	USART_Init(UBRR);
}
void USART_Transmit(  char data )
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) )
	;
	/* Put data into buffer, sends the data */
	UDR0 = data;
}


void uart_puts(char *str)
{
	while(*str)
	{ // Loop through string, sending each character
		USART_Transmit(*str++);
	}
}




void uart_puts_with_length(char *str, UINT length)
{
	while(*str && length > 0)
	{ // Loop through string, sending each character
		USART_Transmit(*str++);
		length--;
	}
}


void uart_puts_rn (char *str){
	uart_puts(str);
	uart_puts("\r\n");
}

void uart_puts_rn_with_length (char *str, UINT length){
	uart_puts_with_length(str, length);
	uart_puts("\r\n");
}


void Sent_error_message(FRESULT fr, char *message){
	if(!fr){
		uart_puts(message);
		uart_puts_rn(" - succes Error code 0");
	}
	else{
		char fr_string[ 4 ];
		
		itoa( fr, fr_string, 10 );
		uart_puts(message);
		uart_puts(" - FAIL Error code: ");
		uart_puts_rn(fr_string);
	}
	
}

void init_timer (void){
TCCR0A = 	(1<< WGM01); // Mode Count to clear
TCCR0B = (1<<CS02) | (1<<CS00); // clkI/O/1024 (from prescaler)
TIMSK0 = (1<<OCIE0A); //  Timer/Counter Output Compare Match B Interrupt Enable


// Time [s] = {2 x Prescaler x ( 1 + OCR0A )} / F_CPU (16 000 000)
	
//OCR0A = {( Time x F_CPU ) / ( 2 x Prescaler )} - 1
OCR0A = 124; //156 -> 10,05ms, 124 -> 8ms
sei();

}
void adc_init (void){
	ADMUX = (1<<REFS1)|(1<<REFS0)					// set internal 1.1V reference voltage
			//| (1<<MUX3) | (1<<MUX2) | (1<<MUX1);	// set MUX to 1.1V
			| (1<<MUX3) ;	// set MUX to temperature sensor
			//| (1<<MUX2) | (1<<MUX0);	// set MUX to ADC5
	ADCSRA = (1<<ADEN) // enable adc
			| (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);// set prescaler to 128
}
uint16_t conversion_result (void){
	uint16_t conv_result;
	
	//conv_result = (ADCH << 8) | (ADCL);
	conv_result = ADCW;
	return conv_result - 337;
}

void start_conversion_synchro (void){
	uint16_t result = 0;
	//adc_init();
	//ADCSRA &= ~(1<<ADIE);
	ADCSRA |= (1<<ADSC);
	
	while(	!(ADCSRA & (1<<ADIF)) );
	
	result = conversion_result();
	
	ADCSRA |= (1<<ADIF);
	//test++;
	char b[ 32 ];
	itoa( result, b, 10 );
	//itoa( test, b, 10 );
	uart_puts_rn(b);
}

void start_conversion_asynchro (void){
	//uint16_t result;
	ADCSRA |= (1<<ADIE);
	ADCSRA |= (1<<ADSC);
	
	//while(	!(ADCSRA & (1<<ADIF)) );
	
	//result = conversion_result();
	
	//ADCSRA |= (1<<ADIF);
	
	//char b[ 32 ];
	//itoa( result, b, 10 );
	//uart_puts_rn(b);
}



ISR(ADC_vect)
{
	
	adc_result = conversion_result();
		
	//ADCSRA |= (1<<ADIF);
	flag = 1;
		

	
}
ISR(TIMER0_COMPA_vect)
{
	// user code here
	licznik++;
	licznik_32bit++;
	if(licznik >= 125){ //dopelnienie do 1sec
		//uart_puts("IT works");
		licznik = 0;
		start_conversion_asynchro();
		//start_conversion_synchro();
	}
	
}

void append_string(char *string, char  *string_to_append){
	
	char *p = string + strlen(string);
	strcpy(p, string_to_append);
	
}

void delete_string(char *string){
	
	//char *p = string + strlen(string);
	//strcpy(p, string_to_append);
	
	*string = '\0';
	
}

uint8_t append_string_with_limits(char *string, char *string_to_append, char max_size){
	
	char *p = string + strlen(string);
	strcpy(p, string_to_append);
	return 1;
}


int main (void) // clock 16 Mhz
{

	//CLKPR  = 1<<CLKPCE; //change clock to 8 Mhz
	//CLKPR  = 1<<CLKPS0;
	USART_Init_Baud_Rate( 38400 ); // do not work at very high baud rate
	//USART_Init(1); //Use this when you want to get very very high baud rate
	
	
	uart_puts_rn("Arduino Booted");
	adc_init();
	//start_conversion_synchro();
	//start_conversion_synchro();
	//uart_puts_rn("Arduino Booted2");
	//start_conversion_asynchro();
// 	while(1){
// 		_delay_ms(500);
// 		start_conversion_synchro();
// 	}
	
init_timer();
//while(1);

	//INIT_LED_PIN();
	//SET_LED_ON();
	_delay_ms(5000);
	//SET_LED_OFF();

	FRESULT fr;
	f_mount(&FatFs, "", 0);		/* Give a work area to the default drive */
//					Lorem_ipsum.txt
//					newfile.txt



//
//UINT licznik = 0;
// do 
// {
// 	fr = f_open(&Fil, "WRITE2.TXT", FA_WRITE | FA_CREATE_ALWAYS );	/* Create a file */
// 	Sent_error_message(fr, "File open WRITE2.TXT");
// 	licznik ++;
// 	_delay_ms(50);
// } while (fr!=FR_OK&& licznik < 10);
// 
// if (fr == FR_OK) {
// 	UINT Bytes_Written;
// 	BYTE Bytes_to_write[512];
// 	for(int i = 0; i < 512;i++)
// 		Bytes_to_write[i] = i;
// 	for(int i = 0; i < 100;i++){
// 		fr = f_write(&Fil, Bytes_to_write, 512, &Bytes_Written);	/* Write data to the file */
// 		Sent_error_message(fr, "File write WRITE.TXT");
// 	}
// 	fr = f_close(&Fil);
// 	Sent_error_message(fr, "Close file WRITE.TXT");
// }
// 
// 
// 
 	fr = f_open(&Fil, "WRITE2.TXT", FA_WRITE | FA_OPEN_APPEND | FA_READ);	/* Create a file */
	// fr = f_open(&Fil, "WRITE2.TXT", FA_WRITE | FA_CREATE_ALWAYS );
 	Sent_error_message(fr, "File open WRITE2.TXT");
 	if (fr == FR_OK) {
// 		//SET_LED_ON();
// 		
// 		//UINT Bytes_Written;
// 	//	fr = f_write(&Fil, "It works!\r\n", 11, &Bytes_Written);	/* Write data to the file */
// 								/* Close the file */
// 		//if (fr == FR_OK && Bytes_Written == 11) {		/* Lights green LED if data written well */
// 		//	uart_puts_rn("Write to file succesfully");
// 			//DDRB |= (1<<5); PORTB |= (1<<5);	/* Set PB4 high */
// 		//}
// 		
 		fr = f_rewind(&Fil);
		Sent_error_message(fr, "Rewind file");
 		
		UINT Bytes_to_read = 512;
		UINT Bytes_readed = 0;
		do 
		{
			
			BYTE Buff[520];

			fr = f_read ( &Fil, Buff, Bytes_to_read, &Bytes_readed);
			//Sent_error_message(fr, "Read file");
			if(fr) Sent_error_message(fr, "Read file");
			
			if (fr == FR_OK){
				//uart_puts("Number of readed bytes: ");
				char b[ 32 ];
				itoa( Bytes_readed, b, 10 );
				//uart_puts_rn(b);
				
				//uart_puts_rn("Bytes Readed: ");
				uart_puts_with_length((char*)(Buff), Bytes_readed);
				//uart_puts_rn("");
				if(Bytes_to_read != Bytes_readed){
					uart_puts_rn("End of file");
				}
			}
			
		} while (Bytes_to_read == Bytes_readed && fr == FR_OK);
		
		
		fr = f_close(&Fil);	
		Sent_error_message(fr, "Close file");
	}

// 	for (;;) {
// 
// 	}
	while(1){
		if(flag){
				flag = 0;
				char adc_result_string[ 32 ];
				char licznik_32bit_string[ 32 ];
				char string_size[ 32 ];
				utoa( adc_result, adc_result_string, 10 );
				utoa( licznik_32bit, licznik_32bit_string, 10 );
				UINT string_size_int = strlen(licznik_32bit_string);
				utoa( string_size_int, string_size, 10 );
				
				/*uart_puts("String size: ");
				uart_puts(string_size);
				uart_puts(" Licznik_32bit: ");
				uart_puts(licznik_32bit_string);
				uart_puts(" ADC asynchro ");
				uart_puts_rn(adc_result_string);*/
				char string_to_sd[64];
				string_to_sd[0]= '\0';
				
				/*strcpy(string_to_sd, "String size: ");
				char *p = string_to_sd + strlen(string_to_sd);
				strcpy(p, licznik_32bit_string);*/
				append_string(string_to_sd, "String size: ");
				append_string(string_to_sd, string_size);
				append_string(string_to_sd, " Licznik_32bit: ");
				append_string(string_to_sd, licznik_32bit_string);
				append_string(string_to_sd, " ADC asynchro: ");
				append_string(string_to_sd, adc_result_string);
				append_string(string_to_sd, "\r\n");
				//uart_puts_rn(string_to_sd);
				uart_puts(string_to_sd);
				
				append_string(Buffer_string, string_to_sd);
				uint16_t Buffer_string_size;
				
				Buffer_string_size = strlen(Buffer_string);
				//Buffer_string_size = 201;
				if(Buffer_string_size > 200){
								
				fr = f_open(&Fil, "WRITE2.TXT", FA_WRITE | FA_OPEN_APPEND );	/* Create a file */
				Sent_error_message(fr, "File open WRITE2.TXT");
				if (fr == FR_OK) {
					UINT Bytes_Written;
					BYTE Bytes_to_write[512];
					//for(int i = 0; i < 512;i++)
					//Bytes_to_write[i] = i;
					//for(int i = 0; i < 100;i++){
						fr = f_write(&Fil, (BYTE*)(Buffer_string), strlen(Buffer_string), &Bytes_Written);	/* Write data to the file */
						Sent_error_message(fr, "File write WRITE2.TXT");
					//}
					fr = f_close(&Fil);
					Sent_error_message(fr, "Close file WRITE2.TXT");
				
				}
				if (fr == FR_DISK_ERR){
					//fr = f_close(&Fil);
					//Sent_error_message(fr, "File open WRITE2.TXT");
					f_mount(&FatFs, "", 0);
					}
			delete_string(Buffer_string);
			uart_puts("Buffer_string deleted\r\n");	
			}
		}
		
	}
}


