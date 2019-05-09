/*
 * 7SEG_smartDisplay.cpp
 * SEVEN SEGMENT MULTIFUNCTION DISPLAY	
 *
 * Author: martin@embedblog.eu
 * http://embedblog.eu/?p=282
 */ 

#include <avr/io.h>
#include "defines.h"
#include <util/delay.h>
#include <avr/interrupt.h>
#include "adc.h"
#include "uart.h"
#include "i2c.h"

const static float ADCL_GAIN = (R11_VALUE + R12_VALUE) / R11_VALUE;
const static float ADCH_GAIN = (R11_VALUE + R12_VALUE + R13_VALUE) / (10 * R11_VALUE);

volatile uint8_t display_digits[4];
volatile uint8_t display_pos;
volatile uint8_t decimal_pos;	//four LSBs correspond to the DPs

volatile uint8_t mode;
volatile bool mode_changed = true;
volatile bool hold;
volatile bool s1_pressed;

extern volatile uint8_t uart_output[4];
extern volatile uint8_t uart_outputDecimal;

extern volatile uint8_t i2c_output[4];
extern volatile uint8_t i2c_outputDecimal;

volatile uint8_t counter_25ms;

int main(void)
{
    //GPIO setup
	DDRB = 0xFF;
	DDRD = (1 << DDD4) | (1 << DDD5) | (1 << DDD6) | (1 << DDD7);

	//ADC setup
	adc_init();

	//UART setup
	uart_init();

	//I2C setup
	i2c_init();

	//INT setup
	PORTD = (1 << PD2) | (1 << PD3);
	EICRA = (1 << ISC11) | (1 << ISC01);
	EIMSK = (1 << INT1) | (1 << INT0);
	
	//TIMER0 setup - used to multiplex the display at around 500 Hz
	TCCR0B = (1 << CS00) | (1 << CS01);
	TIMSK0 = (1 << TOIE0);

	//TIMER2 setup - used to keep time (interrupt every 25 ms)
	TCCR2B = (1 << WGM21) | (1 << CS20) | (1 << CS21) | (1 << CS22);
	OCR2A = 195;
	TIMSK2 = (1 << OCIE2A);

	sei();

    while (1) 
    {
		uint16_t adc_result = adc_getResult();
		switch (mode)
		{
			case MODE_ADCL:
			{
				if (mode_changed)
				{
					decimal_pos = 0x00;
					mode_changed = false;
					SET_DISPLAY(CHAR_A, CHAR_D, CHAR_C, CHAR_L);
					_delay_ms(TRANSITION_TIME);
				}
				
				decimal_pos = 0b1000;
				adc_result *= ADCL_GAIN;
				for (int8_t i = 3; i >= 0; i --)
				{
					display_digits[i] = adc_result % 10;
					adc_result /= 10;
				}

				while (hold);
				break;
			}
			case MODE_ADCH:
			{
				if (mode_changed)
				{
					decimal_pos = 0x00;
					mode_changed = false;
					SET_DISPLAY(CHAR_A, CHAR_D, CHAR_C, CHAR_H);
					_delay_ms(TRANSITION_TIME);
				}
				
				decimal_pos = 0b0100;
				adc_result *= ADCH_GAIN;
				for (int8_t i = 3; i >= 0; i --)
				{
					display_digits[i] = adc_result % 10;
					adc_result /= 10;
				}

				while (hold);
				break;
			}
			case MODE_UART:
			{
				if (mode_changed)
				{
					decimal_pos = 0x00;
					mode_changed = false;
					SET_DISPLAY(CHAR_U, CHAR_A, CHAR_R, CHAR_T);
					_delay_ms(TRANSITION_TIME);
				}

				if (s1_pressed)
				{
					s1_pressed = false;
					decimal_pos = 0b0010;
					switch (uart_changeBaud())
					{
						case 0: {SET_DISPLAY(CHAR_BLANK, CHAR_BLANK, 9, 6); break;}
						case 1: {SET_DISPLAY(CHAR_BLANK, 1, 9, 2); break;}
						case 2: {SET_DISPLAY(CHAR_BLANK, 3, 8, 4); break;}
						case 3: {SET_DISPLAY(CHAR_BLANK, 5, 7, 6); break;}
						case 4: {SET_DISPLAY(1, 1, 5, 2); break;}
						case 5: {SET_DISPLAY(1, 0, 0, 0); decimal_pos = 0x00; break;}
					}

					_delay_ms(TRANSITION_TIME);
				}

				decimal_pos = uart_outputDecimal;
				SET_DISPLAY(uart_output[0], uart_output[1], uart_output[2], uart_output[3]);
				break;
			}
			case MODE_I2C:
			{
				if (mode_changed)
				{
					decimal_pos = 0x00;
					mode_changed = false;
					SET_DISPLAY(CHAR_BLANK, CHAR_I, 2, CHAR_C);
					_delay_ms(TRANSITION_TIME);
				}
				
				decimal_pos = i2c_outputDecimal;
				SET_DISPLAY(i2c_output[0], i2c_output[1], i2c_output[2], i2c_output[3]);
				break;
			}
		}

		_delay_ms(1000 / REFRESH_RATE);
    }
}

//timer 0 - used to multiplex the display
ISR(TIMER0_OVF_vect)
{
	display_pos++;
	if (display_pos > 3)
		display_pos = 0;

	PORTD |= 0b11110000;
	switch (display_pos)
	{
		case 0:
		{
			PORTB = charMap[display_digits[0]];
			if ((decimal_pos >> 3) & 0x01)
				PORTB |= 0x01;
			PORTD &=~(1 << PD7);
			break;
		}
		case 1:
		{
			PORTB = charMap[display_digits[1]];
			if ((decimal_pos >> 2) & 0x01)
				PORTB |= 0x01;
			PORTD &=~(1 << PD5);
			break;
		}
		case 2:
		{
			PORTB = charMap[display_digits[2]];
			if ((decimal_pos >> 1) & 0x01)
				PORTB |= 0x01;
			PORTD &=~(1 << PD6);
			break;
		}
		case 3:
		{
			PORTB = charMap[display_digits[3]];
			if (decimal_pos & 0x01)
				PORTB |= 0x01;
			PORTD &=~(1 << PD4);
			break;
		}
	}
}

//interrupt every 25 ms
ISR(TIMER2_COMPA_vect)
{
	counter_25ms++;
}

//button S1
ISR(INT0_vect)
{
	hold = !hold;
	s1_pressed = true;
}

//button S2
ISR(INT1_vect)
{
	mode_changed = true;
	mode++;
	if (mode >= MODES_NUMBER)
		mode = 0;
}



