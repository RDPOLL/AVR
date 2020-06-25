#include <stdlib.h>
#include <avr/io.h>
#include "lcd.h"   //" = Lib in akt Projektverzeichnis
#include <avr/interrupt.h>

#ifndef F_CPU
#define F_CPU    8000000
#endif

uint32_t x;
uint8_t y, richtung;

int main(void)
{
	
	DDRD = 0xFF;			//Port D Ausgang 
	PORTD = 0x00;
	
	DDRC = 0xFF;			//Port C Ausgang 
	PORTC = 0x00;
	
	DDRB = 0XFF;			//Port B Eingang
	PORTB = 0XFF;

				
				
	x=0;	
	y=0;	
	richtung = 1;	
				
	lcd_init(LCD_DISP_ON);  // initialisieren
    lcd_clrscr();
    lcd_gotoxy(0,0);
	lcd_puts("Mikrokontroller");
	lcd_gotoxy(0,1);
	lcd_puts("Programmierkurs NKF");


	
	
							
	while(1)
	{
	
	

	}//end of while
	return 0;
}//end of main


