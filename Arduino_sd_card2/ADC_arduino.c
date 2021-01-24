/*
 * ADC_arduino.c
 *
 * Created: 24.01.2021 12:56:58
 *  Author: malin
 */ 

#include <avr/io.h>	/* Device specific declarations */
#include <util/delay.h>
#include <ADC_arduino.h>



void set_ADC_channel (uint8_t ADC_CH){
	ADMUX  &= ~(MUX0 | MUX1 | MUX2 | MUX3);
	ADMUX  |= (ADC_CH);
}