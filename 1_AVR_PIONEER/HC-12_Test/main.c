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

	unsigned char input[16];
	unsigned short l = 0;
	unsigned char r = 0;
	unsigned char output[16];
	//volatile unsigned long i = 0;
 
	//initialiting USART
	USART_Init(103);

	lcd_init(LCD_DISP_ON);  	//Initialisieren

	HC_init(96,1,1);
	
	while(1)
	{
		USART_Receive_STRING(input);
		sscanf(input, "r:%03d", r);

		sprintf(output, "r:%03d",l ,r);
		lcd_gotoxy(0,0);
		lcd_puts(output);
//------------------------------------------
	}//end of while
}//end of main
