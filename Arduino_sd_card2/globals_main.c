/*
 * globals_main.c
 *
 * Created: 24.01.2021 15:16:41
 *  Author: malin
 */ 
#include <avr/io.h>	
#include "globals_main.h"



void put_on_Cycle_buffer (uint16_t adc_result_f, uint32_t licznik_f,volatile struct cycle_Buffer * Cycle_Buffer_f){
	Cycle_Buffer_f->adc_result_buffer[Cycle_Buffer_f->end_c] = adc_result_f;
	Cycle_Buffer_f->licznik_buffer[Cycle_Buffer_f->end_c] = licznik_f;
	Cycle_Buffer_f->end_c ++;
	if(Cycle_Buffer_f->end_c >= CYCLE_BUFFER_SIZE){
		Cycle_Buffer_f->end_c = 0;
	}
	
	if( Cycle_Buffer_f->end_c == Cycle_Buffer_f->start_c ){
		Cycle_Buffer_f->start_c ++;
		if(Cycle_Buffer_f->start_c >= CYCLE_BUFFER_SIZE){
			Cycle_Buffer_f->start_c = 0;
		}
	}
}

uint8_t get_from_Cycle_buffer (uint16_t * adc_result_f, uint32_t * licznik_f,volatile struct cycle_Buffer * Cycle_Buffer_f){
	if(Cycle_Buffer_f->start_c == Cycle_Buffer_f->end_c)
	return 1; // Error 1 -> Empty Cycle buffer
	
	* adc_result_f = Cycle_Buffer_f->adc_result_buffer[Cycle_Buffer_f->start_c];
	* licznik_f = Cycle_Buffer_f->licznik_buffer[Cycle_Buffer_f->start_c];
	
	Cycle_Buffer_f->start_c++;
	if(Cycle_Buffer_f->start_c >= CYCLE_BUFFER_SIZE){
		Cycle_Buffer_f->start_c = 0;
	}
	
	return 0;
}
void Clear_Cycle_buffer (volatile struct cycle_Buffer * Cycle_Buffer_f){
	Cycle_Buffer_f->start_c = Cycle_Buffer_f->end_c;
}

uint16_t conversion_result (void){
	uint16_t conv_result;
	
	//conv_result = (ADCH << 8) | (ADCL);
	conv_result = ADCW;
	return conv_result;
}