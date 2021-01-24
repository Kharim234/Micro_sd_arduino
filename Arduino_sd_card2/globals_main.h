/*
 * globals_main.h
 *
 * Created: 24.01.2021 15:08:44
 *  Author: malin
 */ 


#ifndef GLOBALS_MAIN_H_
#define GLOBALS_MAIN_H_

#define CYCLE_BUFFER_SIZE 20

struct cycle_Buffer {
	uint8_t start_c;
	uint8_t end_c;
	uint16_t adc_result_buffer[CYCLE_BUFFER_SIZE];
	uint32_t licznik_buffer[CYCLE_BUFFER_SIZE];
};

volatile struct cycle_Buffer Cycle_Buffer_1;
volatile uint8_t flag_adc_conversion_done;
volatile uint16_t adc_result;
volatile uint32_t licznik_32bit;

void put_on_Cycle_buffer (uint16_t adc_result_f, uint32_t licznik_f,volatile struct cycle_Buffer * Cycle_Buffer_f);
uint8_t get_from_Cycle_buffer (uint16_t * adc_result_f, uint32_t * licznik_f,volatile struct cycle_Buffer * Cycle_Buffer_f);
void Clear_Cycle_buffer (volatile struct cycle_Buffer * Cycle_Buffer_f);
uint16_t conversion_result (void);




#endif /* GLOBALS_MAIN_H_ */