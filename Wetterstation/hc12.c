/*
 * This is a Libary for the HC-12 radio module. The libary only works
 * with the serial.c libary (Uart)
 *
 * in this libary are some initializing function. For sending and
 * reciving data, use the normal USART_... functions in the serial.c file.
 *
 * The SET pin of the HC-12 modul shall be connected to a output of the uP
 * Change the two defines accordingly
 */

#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>

#define SETPORT PORTB		//PORT of HC-12 SET
#define SETPIN PB1			//Pin of HC-12 SET


//Send instruction to HC-12
void HC_sendCommand(unsigned char hcCommandS[15])
{	
	SETPORT &= ~(1<<SETPIN);
	_delay_ms(40);
	
	USART_Transmit_STRING(hcCommandS);

	SETPORT |= (1<<SETPIN);
	_delay_ms(200);
}

//reset all settings to default
void HC_default(void)
{
	HC_sendCommand("AT+DFAULT");
}

//go into Sleep mode (no TX/RX)
void HC_goSleep(void)
{
	HC_sendCommand("AT+SLEEP");
}

//wake HC-12 up
void HC_wakeUp(void)
{
	HC_sendCommand("");
}

//test, module returns: OK
void HC_test(void)
{
	HC_sendCommand("AT");
}

//set the Baudrate (baud-100) (12, 24, 48, 96)
void HC_setBaud(unsigned char hcBaud)
{
	unsigned char hcBaudS[7];
	sprintf(hcBaudS, "AT+B%d00", hcBaud);
	HC_sendCommand(hcBaudS);
}

//change the channel (1-100)
void HC_setChannel(unsigned char hcChannel)
{
	unsigned char hcChannelS[5];
	sprintf(hcChannelS, "AT+C%03d", hcChannel);
	HC_sendCommand(hcChannelS);
}

//set the output Power (1-8, acording to the datasheet)
void HC_setPower(unsigned char hcPowerOut)
{
	unsigned char hcPowerOutS[5];
	sprintf(hcPowerOutS, "AT+P%d", hcPowerOut);
	HC_sendCommand(hcPowerOutS);
}

//change between the four modes (datasheet)
void HC_setMode(unsigned char hcMode)
{
	unsigned char hcModeS[8];
	sprintf(hcModeS, "AT+FU%d", hcMode);
	HC_sendCommand(hcMode);
}

//Initialize the HC-12 with one command
void HC_init(unsigned short hcBaudInit, unsigned char hcChannelInit, unsigned char hcPowerOutInit)
{
	HC_setBaud(hcBaudInit);
	HC_setChannel(hcChannelInit);
	HC_setPower(hcPowerOutInit);
}




