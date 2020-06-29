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

//------------------------------MAIN------------------------------------
int main(void)
{	
	DDRD = 0x02;			//Schalterport
	DDRB = 0xff;			//LEDsport
	DDRC = 0xff;			//LCDdaten
	DDRA = 0xf0;			//LCDctrl & ADC

	unsigned char Output[16];
	//unsigned char output = 0;
	//volatile unsigned long i = 0;

	//initialiting USART
	USART_Init(103);

	USART_Transmit_STRING("\n\rStartup completed");

	lcd_init(LCD_DISP_ON);  	//Initialisieren
	
	while(1)
	{
//----------OUTPUT---------------------

		USART_Receive_STRING(Output);
	
		if(((strcmp(Output, "clear")) == 0))
			lcd_clrscr();
		else
		{
			lcd_gotoxy(0, 0);
			lcd_puts(Output);
		}
//------------------------------------------
	}//end of while
}//end of main
