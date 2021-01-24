/*
 * ADC_arduino.c
 *
 * Created: 24.01.2021 12:56:58
 *  Author: malin
 */ 

#include <avr/io.h>	/* Device specific declarations */
#include <util/delay.h>
#include "ADC_arduino.h"
#include "globals_main.h"

#define STATE_DO_NOTHING 0
#define RECEIVE_MEASURE_CURRENT_AND_START_BATT_VCC_MEASURE 1
#define RECEIVE_BATT_VCC_MEASURE 2



uint8_t state_adc;
void state_function_ADC (uint8_t *state);

void start_conversion_asynchro_adc (void){
	
	ADCSRA |= (1<<ADIE);
	ADCSRA |= (1<<ADSC);
	
}

void set_ADC_channel (uint8_t ADC_CH){
	uint8_t admux_buffer = 0;
	admux_buffer = ADMUX & ~( (1<<MUX0) | (1<<MUX1) | (1<<MUX2) | (1<<MUX3) );
	admux_buffer |= ADC_CH;
	ADMUX  = admux_buffer;
}



void start_measure_current_ADC(void){
	set_ADC_channel(ADC_CURRENT);
	start_conversion_asynchro_adc();
	state_adc = RECEIVE_MEASURE_CURRENT_AND_START_BATT_VCC_MEASURE;
}

void run_next_step_state_func_ADC (void){
	state_function_ADC (&state_adc);
}

void state_function_ADC (uint8_t *state){
	switch (*state)
	{
	case RECEIVE_MEASURE_CURRENT_AND_START_BATT_VCC_MEASURE:
		adc_result = conversion_result();
		//flag_adc_conversion_done = 1;
		put_on_Cycle_buffer(adc_result, licznik_32bit, &Cycle_Buffer_1);
		
		set_ADC_channel(ADC_BATT_VCC);
		start_conversion_asynchro_adc();
		*state = RECEIVE_BATT_VCC_MEASURE;
		break;
		
	case RECEIVE_BATT_VCC_MEASURE:
	
		adc_result2 = conversion_result();
		flag_adc_conversion_done = 1;
		*state = STATE_DO_NOTHING;
		break;
		
	
		
	case STATE_DO_NOTHING:
		break;
	default:
		break;

	}
	
}