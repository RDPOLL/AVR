#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include "rotary.c"
#include "servo.c"

//------------------------------MAIN------------------------------------
int main(void)
{
	char output[16];
	short servo = 0;

	//pinsetup
	DDRC = 0xff;			//LCDdaten
	DDRD |= (1<<PD4);

	//pullup
	PORTB = 0x07;

	//init
	PORTC |= (1<<PC7); 			//Backlight
	lcd_init(LCD_DISP_ON);

	servo_init();


	while(1)
	{
		usrInput(PINB);

		if(rotary.right)
			servo += 1;
			
		if(rotary.left)
			servo -= 1;

		servo_setDegree(servo);
			
		sprintf(output, "%05d", servo);
		lcd_gotoxy(0,0);
		lcd_puts(output);

//------------------------------------------
	}//end of while
}//end of main
