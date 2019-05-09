/*
 * uart.h
 * SEVEN SEGMENT MULTIFUNCTION DISPLAY
 *
 * Author: martin@embedblog.eu
 * http://embedblog.eu/?p=282
 */


#ifndef UART_H_
#define UART_H_

void uart_init();
void uart_setBaud(uint8_t index);
uint8_t uart_changeBaud();

#endif /* UART_H_ */