#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include "rotary.c"

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

//------------------------------MAIN------------------------------------
int main(void)
{
	unsigned char output[16];
	unsigned char inSel = 0x10;

	DDRA = 0x00;
	DDRB = 0x00;
	DDRC = 0xff;			//LCD
	DDRD = 0x00;

	PORTB = 0xff;

	lcd_init(LCD_DISP_ON);  	//Initialisieren
	PORTC |= (1<<PC7);

	
	while(1)
	{
		usrInput(PINB);
		
		lcd_gotoxy(0,0);
		sprintf(output, " 0x%x: "BYTE_TO_BINARY_PATTERN, inSel, BYTE_TO_BINARY(_SFR_IO8(inSel)));
		lcd_puts(output);

		if(rotary.right && (inSel < 0x19))
		{
			inSel += 3;
		}
		else if(rotary.left && (inSel > 0x10))
		{
			inSel -= 3;
		}

		!(PINB & (1<<PB2)) ? (PORTC &= ~(1<<PC7)) : (PORTC |= (1<<PC7));
		
//------------------------------------------
	}//end of while
}//end of main
