#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "lcd.h"

#define F_CPU    	8000000

unsigned char input = 0;

//------------------------------Flankentriggerung-----------------------

struct
{
	unsigned char act;					//Variabeln fuer die Flankentriggerung
	unsigned char old;
	unsigned char flk;
	unsigned char pos_flk;
	unsigned char neg_flk;
}flk = {0};

void flk_t(unsigned char in)			//PORT in uebergabe wert angeben
{
	flk.act = in;
	flk.flk = flk.act ^ flk.old;
	flk.neg_flk = flk.flk & flk.old;
	flk.pos_flk = flk.flk & flk.act;
	flk.old = flk.act;

	return;
}
//------------------------------END-------------------------------------

//------------------------------Rotary----------------------------------

struct
{
	unsigned char right;
	unsigned char left;
	unsigned char press;
}rotary = {0};

void rotary_f(void)
{	
	rotary.right = 0;
	rotary.left = 0;
	rotary.press = 0;
	
	if(flk.neg_flk & (1 << PD3)) rotary.press = 1;
	if((flk.neg_flk & (1 << PD7)) && (input & (1 << PD6))) rotary.right = 1;
	if((flk.neg_flk & (1 << PD6)) && (input & (1 << PD7))) rotary.left = 1;
	
	return;
}

//------------------------------END-------------------------------------

//------------------------------MAIN------------------------------------
int main(void)
{	
	DDRD = 0x00;			//Schalterport
	DDRB = 0XFF;			//LEDsport
	DDRC = 0xFF;			//LCDdaten

	unsigned char output[16];
	unsigned short hori = 8;
	unsigned short taster = 0;
	volatile unsigned long i = 0;
	
	lcd_init(LCD_DISP_ON);  // initialisieren
				
	while(1)
	{
		input = PIND;
		flk_t(input);
		rotary_f();
		
		if(rotary.right)
		{
			 hori++;
			 lcd_clrscr();
		}
		if(rotary.left)
		{
			 hori--;
			 lcd_clrscr();
		}
		if(rotary.press)
		{
			taster ^= 1;
			lcd_clrscr();
		}
		
//----------OUTPUT---------------------
		PORTB = input;

		lcd_gotoxy(0,0);
		lcd_puts("STD AVR PROJEKT");
		
		lcd_gotoxy(hori,1);
		lcd_putc('*');

		sprintf(output, "%0d", taster);
		lcd_gotoxy(0,1);
		lcd_puts(output);

		sprintf(output, "%02d", hori);
		lcd_gotoxy(14,1);
		lcd_puts(output);
//------------------------------------------
	}//end of while
}//end of main
