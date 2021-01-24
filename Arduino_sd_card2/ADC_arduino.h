/*
 * ADC_arduino.h
 *
 * Created: 24.01.2021 12:57:26
 *  Author: malin
 */ 


#ifndef ADC_ARDUINO_H_
#define ADC_ARDUINO_H_

//#define ADC_CURRENT ADCH3
#define ADC_CURRENT 3 //ADC3

void set_ADC_channel (uint8_t ADC_CH);
void start_measure_current_ADC(void);
void run_next_step_state_func_ADC (void);
//init_and_start_state_machine_ADC();





#endif /* ADC_ARDUINO_H_ */