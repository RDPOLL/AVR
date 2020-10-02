#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
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

	lcd_init(LCD_DISP_ON);  	//Initialisieren
	
	while(1)
	{
		lcd_gotoxy(0,0);
		lcd_puts("Hello World");
//------------------------------------------
	}//end of while
}//end of main
