/*
 * i2c.cpp
 * SEVEN SEGMENT MULTIFUNCTION DISPLAY
 *
 * Author: martin@embedblog.eu
 * http://embedblog.eu/?p=282
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "i2c.h"
#include "defines.h"

volatile uint8_t i2c_pos;
volatile uint8_t i2c_input[4];
volatile uint8_t i2c_inputDecimal;

volatile uint8_t i2c_output[4] = {CHAR_UNDERSCORE, CHAR_UNDERSCORE, CHAR_UNDERSCORE, CHAR_UNDERSCORE};
volatile uint8_t i2c_outputDecimal;

void i2c_init()
{
	//enable pullups on PC0 and PC1
	DDRC &=~(1 << DDC0) | (1 << DDC1);
	PORTC |= (1 << PC0) | (1 << PC1);
	uint8_t address = I2C_ADDRESS | (!PINC &0b11);
	
	//set slave (own) address
	TWAR = address;

	//enable ACK, enable TWI/I2C, enable interrupt
	TWCR = (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
}

ISR(TWI_vect)
{
	switch (TWSR & 0b11111000)
	{
		case 0x60:						//own address received, acknowledge and clear flag
		{
			TWCR |= (1 << TWEA) | (1 << TWINT);
			break;
		}
		case 0x80:						//data received, store data and acknowledge
		{
			uint8_t input = TWDR;

			//check for valid characters
			if (input <= 9)
				i2c_input[i2c_pos++] = input;
			else
			{
				switch (input)
				{
					case 45: {i2c_input[i2c_pos++] = CHAR_DASH; break;}
					case 32: {i2c_input[i2c_pos++] = CHAR_BLANK; break;}
					case 95: {i2c_input[i2c_pos++] = CHAR_UNDERSCORE; break;}
					case 46: {i2c_inputDecimal |= (0b10000 >> i2c_pos); break;}
				}
			}

			TWCR |= (1 << TWEA) | (1 << TWINT);
			break;
		}
		case 0xA0:						//a STOP condition has been received
		{
			TWCR |= (1 << TWEA) | (1 << TWINT);

			i2c_pos = 0;
			i2c_outputDecimal = i2c_inputDecimal;
			i2c_inputDecimal = 0x00;

			for (uint8_t i = 0; i < 4; i ++)
			{
				i2c_output[i] = i2c_input[i];
				i2c_input[i] = CHAR_BLANK;
			}

			break;
		}
		default: TWCR |= (1 << TWINT);	//set TWINT flag
	}
}