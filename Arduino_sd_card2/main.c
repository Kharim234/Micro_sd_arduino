/*----------------------------------------------------------------------*/
/* Foolproof FatFs sample project for AVR              (C)ChaN, 2014    */
/*----------------------------------------------------------------------*/

#include <avr/io.h>	/* Device specific declarations */
#include <util/delay.h>
#include "ff.h"		/* Declarations of FatFs API */
#include <stdlib.h>
#include <avr/interrupt.h>

FATFS FatFs;		/* FatFs work area needed for each volume */
FIL Fil;			/* File object needed for each open file */

#define LED_PIN (1<<PB5)
#define INIT_LED_PIN() DDRB |= LED_PIN
#define SET_LED_ON() (PORTB |= LED_PIN)
#define SET_LED_OFF() (PORTB &= ~LED_PIN)

volatile uint16_t test = 0;

uint16_t licznik;

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
OCR0A = 156; //156 -> 20,1ms
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
	uint16_t result;
	result = conversion_result();
		
	//ADCSRA |= (1<<ADIF);
		
	char b[ 32 ];
	itoa( result, b, 10 );
	uart_puts("ADC asynchro ");
	uart_puts_rn(b);
	
}
ISR(TIMER0_COMPA_vect)
{
	// user code here
	licznik++;
	if(licznik > 100){
		//uart_puts("IT works");
		licznik = 0;
		start_conversion_asynchro();
		//start_conversion_synchro();
	}
	
}

int main (void) // clock 16 Mhz
{

	//CLKPR  = 1<<CLKPCE; //change clock to 8 Mhz
	//CLKPR  = 1<<CLKPS0;
	USART_Init_Baud_Rate( 9600 ); // do not work at very high baud rate
	//USART_Init(1); //Use this when you want to get very very high baud rate
	
	
	uart_puts_rn("Arduino Booted");
	adc_init();
	//start_conversion_synchro();
	//start_conversion_synchro();
	uart_puts_rn("Arduino Booted2");
	//start_conversion_asynchro();
// 	while(1){
// 		_delay_ms(500);
// 		start_conversion_synchro();
// 	}
	
init_timer();
//while(1);

	//INIT_LED_PIN();
	//SET_LED_ON();
	_delay_ms(500);
	//SET_LED_OFF();

	FRESULT fr;
	f_mount(&FatFs, "", 0);		/* Give a work area to the default drive */
//					Lorem_ipsum.txt
//					newfile.txt



//
UINT licznik = 0;
do 
{
	fr = f_open(&Fil, "WRITE.TXT", FA_WRITE | FA_CREATE_ALWAYS );	/* Create a file */
	Sent_error_message(fr, "File open WRITE.TXT");
	licznik ++;
	_delay_ms(50);
} while (fr!=FR_OK&& licznik < 10);

if (fr == FR_OK) {
	UINT Bytes_Written;
	BYTE Bytes_to_write[512];
	for(int i = 0; i < 512;i++)
		Bytes_to_write[i] = i;
	for(int i = 0; i < 100;i++){
		fr = f_write(&Fil, Bytes_to_write, 512, &Bytes_Written);	/* Write data to the file */
		Sent_error_message(fr, "File write WRITE.TXT");
	}
	fr = f_close(&Fil);
	Sent_error_message(fr, "Close file WRITE.TXT");
}



	fr = f_open(&Fil, "PANTAD.TXT", FA_WRITE | FA_OPEN_APPEND | FA_READ);	/* Create a file */
	Sent_error_message(fr, "File open PANTAD.TXT");
	if (fr == FR_OK) {
		//SET_LED_ON();
		
		//UINT Bytes_Written;
	//	fr = f_write(&Fil, "It works!\r\n", 11, &Bytes_Written);	/* Write data to the file */
								/* Close the file */
		//if (fr == FR_OK && Bytes_Written == 11) {		/* Lights green LED if data written well */
		//	uart_puts_rn("Write to file succesfully");
			//DDRB |= (1<<5); PORTB |= (1<<5);	/* Set PB4 high */
		//}
		
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

	for (;;) {

	}
}


