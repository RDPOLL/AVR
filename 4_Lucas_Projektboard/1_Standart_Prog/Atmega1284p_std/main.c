#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include "rotary.c"

//------------------------------MAIN------------------------------------
int main(void)
{
//pinsetup
	DDRC = 0xff;			//LCDdaten

//Pullup
	PORTB = 0x07;

//init
	PORTC |= (1<<PC7); 			//Backlight
	lcd_init(LCD_DISP_ON);


	while(1)
	{
		usrInput(PORTB);
		lcd_gotoxy(0,0);
		lcd_puts("Hello World");

//------------------------------------------
	}//end of while
}//end of main
