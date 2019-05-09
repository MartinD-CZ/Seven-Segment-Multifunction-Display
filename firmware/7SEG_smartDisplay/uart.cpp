/*
 * uart.cpp
 * SEVEN SEGMENT MULTIFUNCTION DISPLAY
 *
 * Author: martin@embedblog.eu
 * http://embedblog.eu/?p=282
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "uart.h"
#include "defines.h"

//baud rate values to be put into UBRR0: 9600, 19200, 38400, 57600, 115200, 1000000 
const static uint8_t baud_register[] = {  103,    51,    25,    16,      8,      0};
uint8_t baud_index;

volatile uint8_t uart_pos;
volatile uint8_t uart_input[4];
volatile uint8_t uart_inputDecimal;

volatile uint8_t uart_output[4] = {CHAR_UNDERSCORE, CHAR_UNDERSCORE, CHAR_UNDERSCORE, CHAR_UNDERSCORE};
volatile uint8_t uart_outputDecimal;

void uart_init()
{
	//set GPIO - should be input by default

	//set double speed
	UCSR0A = (1 << U2X0);

	//8 data bits, no parity, 1 stop bit
	UCSR0C = (1 << UCSZ00) | (1 << UCSZ01);

	uart_setBaud(baud_index);

	//enable RX interrupt and the receiver
	UCSR0B = (1 << RXCIE0) | (1 << RXEN0);
}

void uart_setBaud(uint8_t index)
{
	UBRR0H = 0x00;
	UBRR0L = baud_register[index];
}

uint8_t uart_changeBaud()
{
	baud_index++;

	if (baud_index > 5)
		baud_index = 0;
	
	uart_setBaud(baud_index);

	return baud_index;
}

ISR(USART_RX_vect)
{
	int8_t input = UDR0;
	input -= 48;

	//check for valid characters
	if ((input >= 0) & (input <= 9))
		uart_input[uart_pos++] = (uint8_t)input;
	else if (input == -3)		//dash
		uart_input[uart_pos++] = CHAR_DASH;
	else if (input == -16)		//space
		uart_input[uart_pos++] = CHAR_BLANK;
	else if (input == 47)		//underscore
		uart_input[uart_pos++] = CHAR_UNDERSCORE;
	else if (input == -2)		//dot
		uart_inputDecimal |= (0b10000 >> uart_pos);
	else if ((input == -38) | (input == -35))		//carriage return/line feed - forced display
		uart_pos = 4;
	

	if (uart_pos > 3)
	{
		uart_pos = 0;
		uart_outputDecimal = uart_inputDecimal;
		uart_inputDecimal = 0;

		for (uint8_t i = 0; i < 4; i ++)
		{
			uart_output[i] = uart_input[i];
			uart_input[i] = CHAR_BLANK;
		}
	}
}