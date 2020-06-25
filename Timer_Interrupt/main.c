#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include "serial.c"
#include "rotary.c"

volatile unsigned short i = 0;
volatile unsigned short cntr = 0;

ISR(TIMER0_COMPA_vect)
{
	i++;
	if(i == 625)
	{
		i = 0;
		cntr++;
	}
}

//------------------------------MAIN------------------------------------
int main(void)
{	
	DDRD = 0x02;			//Schalterport
	DDRB = 0xff;			//LEDsport
	DDRC = 0xff;			//LCDdaten
	DDRA = 0xf0;			//LCDctrl & ADC

	sei();

	char output[16];

//-------------------------------------

	TCCR0A = 0x02;
	TCCR0A |= (1<<COM0A1);
	TCCR0B = 0x05;
	TIMSK0 = 0x02;

	OCR0A = 24;
	OCR0B = 24;

	TCNT0 = 0;

//-------------------------------------

	lcd_init(LCD_DISP_ON);  	//Initialisieren
	
	while(1)
	{
		usrInput(PIND);
		
//----------OUTPUT---------------------

		sprintf(output, "%05dsec", cntr);
		lcd_gotoxy(0,0);
		lcd_puts(output);
		
//------------------------------------------
	}//end of while
}//end of main
