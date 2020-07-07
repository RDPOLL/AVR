/*
 * This is a Libary for the HC-12 radio module. The libary only works
 * with the serial.c libary (Uart)
 *
 * in this libary are some initializing function. for sending and
 * reciving data, use the normal USART_... functions in the serial.c file.
 */

#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>

#define SETPORT PORTB		//PORT of HC-12 SET
#define SETPIN PB0			//Pin of HC-12 SET



void HC_sendCommand(unsigned char commandS[15])
{	
	SETPORT &= ~(1<<SETPIN);

	USART_Transmit_STRING(commandS);
	
	SETPORT |= (1<<SETPIN);
}

void HC_setBaud(unsigned long baud)
{
	unsigned char baudS[7];
	sprintf(baudS, "AT+B%d", baud);
	HC_sendCommand(baudS);
}

void HC_setChannel(unsigned char channel)
{
	unsigned char channelS[5];
	sprintf(channelS, "AT+C%03d", channel);
	HC_sendCommand(channelS);
}



