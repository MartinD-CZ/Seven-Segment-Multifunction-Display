/*
 * defines.h
 * SEVEN SEGMENT MULTIFUNCTION DISPLAY
 *
 * Author: martin@embedblog.eu
 * http://embedblog.eu/?p=282
 */


#ifndef DEFINES_H_
#define DEFINES_H_

//======================================================
//USER SETTINGS

#define REFRESH_RATE		5				//in Hz
#define TRANSITION_TIME		1200			//how long (in ms) a transistion message is shown

#define R11_VALUE			9.993			//resistance (in kohms)
#define R12_VALUE			14.931
#define R13_VALUE			99.97
#define VIN					2504

#define I2C_ADDRESS			0b010100		//default I2C address

//END OF SETTINGS
//======================================================

#define F_CPU 8000000UL

const uint8_t charMap[] = {
	0x7E,		//0
	0x60,		//1
	0xAE,		//2
	0xEA,		//3
	0xF0,		//4
	0xDA,		//5
	0xDE,		//6
	0x68,		//7
	0xFE,		//8
	0xFA,		//9
	0xFC,		//CHAR_A
	0xE6,		//CHAR_D
	0x86,		//CHAR_C
	0x16,		//CHAR_L
	0xF4,		//CHAR_H
	0x76,		//CHAR_U
	0x96,		//CHAR_T
	0x84,		//CHAR_R
	0x00,		//CHAR_BLANK
	0x80,		//CHAR_DASH
	0x02,		//CHAR_UNDERSCORE
	0x40		//CHAR_I
};

#define CHAR_A			10
#define CHAR_D			11
#define CHAR_C			12
#define CHAR_L			13
#define CHAR_H			14
#define CHAR_U			15
#define CHAR_T			16
#define CHAR_R			17
#define CHAR_BLANK		18
#define CHAR_DASH		19
#define CHAR_UNDERSCORE	20
#define CHAR_I			21

#define MODE_ADCL		0
#define MODE_ADCH		1
#define MODE_UART		2
#define MODE_I2C		3

#define MODES_NUMBER	4

#define SET_DISPLAY(x, y, z, a)		display_digits[0] = x; display_digits[1] = y; display_digits[2] = z; display_digits[3] = a

#endif /* DEFINES_H_ */