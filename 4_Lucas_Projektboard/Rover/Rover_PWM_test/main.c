#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/sleep.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/interrupt.h>
#include "adc.c"
#include "lcd.h"
#include "rotary.c"
#include "rover.c"
#include "serial.c"
#include "hc12.c"
#include "usound.c"


//------------------------------MAIN------------------------------------
int main(void)
{
	unsigned char speed = 0;
	char output[16];

	DDRA = 0x00;
	//DDRB = 0xff;
	DDRC = 0xff;			//LCD
	DDRD |= (1<<PD4);

	PORTB = 0x07;

//Lib Initialisieren
	lcd_init(LCD_DISP_ON);  	
	PORTC |= (1<<PC7);

	rover_init();

//-------------------
	
	while(1)
	{		
		usrInput(PINB);

		//Setting the Speed
		if(rotary.right && (speed < 255))
		{
			speed++;
		}
		else if(rotary.left && (speed > 0))
		{
			speed--;
		}
		else if(rotary.press)
		{
			speed = 0;
		}
		
		rover_straight(BACKWARD, speed);
	
		sprintf(output, "Speed: %03d", speed);
		lcd_gotoxy(0,0);
		lcd_puts(output);

//------------------------------------------
	}//end of while
}//end of main
