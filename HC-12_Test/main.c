#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <string.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include "serial.c"
#include "rotary.c"
#include "hc12.c"

//------------------------------MAIN------------------------------------
int main(void)
{	
	DDRD = 0x02;			//Schalterport
	DDRB = 0xff;			//LEDsport
	DDRC = 0xff;			//LCDdaten
	DDRA = 0xf0;			//LCDctrl & ADC

	unsigned char input[10];
	//unsigned char output = 0;
	//volatile unsigned long i = 0;

	//initialiting USART
	USART_Init(103);

	lcd_init(LCD_DISP_ON);  	//Initialisieren

	HC_setBaud(9600);
	
	HC_setChannel(10);
	
	while(1)
	{


//------------------------------------------
	}//end of while
}//end of main
