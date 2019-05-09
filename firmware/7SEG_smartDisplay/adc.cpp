/*
 * adc.cpp
 * SEVEN SEGMENT MULTIFUNCTION DISPLAY
 *
 * Author: martin@embedblog.eu
 * http://embedblog.eu/?p=282
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "defines.h"

volatile uint32_t adc_data;
volatile uint16_t adc_samples;

void adc_init()
{
	//AVcc as reference, channel 
	ADMUX = (1 << REFS0) | (1 << MUX1);

	//prescaler of 64, interrupt enable, start the ADC
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADIE);

	//start the ADC
	ADCSRA |= (1 << ADSC);
}

ISR(ADC_vect)
{
	adc_data += ADC;

	adc_samples++;
	
	//start the next conversion
	ADCSRA |= (1 << ADSC);
}

uint16_t adc_getResult()
{
	float temp = adc_data / adc_samples;
	adc_data = 0;
	adc_samples = 0;
	return (uint16_t)(temp * VIN / 1024);
}
