#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include "lcd.h"
#include "serial.c"
#include "rotary.c"

volatile unsigned char cntr = 0;

ISR(INT1_vect)
{
	cntr++;
}

//------------------------------MAIN------------------------------------
int main(void)
{	
	DDRD = 0x00;			//Schalterport
	DDRB = 0Xff;			//LEDsport
	DDRC = 0xff;			//LCDdaten
	DDRA = 0xff;			//LCDctrl

	char output[16];

	EICRA = 0x08;
	EIMSK = 0x02;
	
	sei();
	
	lcd_init(LCD_DISP_ON);  	//Initialisieren

	
	while(1)
	{
		sprintf(output,"%d", cntr);
		lcd_gotoxy(0,0);
		lcd_puts(output);
	}//end of while
}//end of main
