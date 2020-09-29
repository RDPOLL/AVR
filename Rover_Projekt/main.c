#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include "serial.c"
#include "rover.c"


//------------------------------MAIN------------------------------------
int main(void)
{
	//unsigned char i = 0;
	
	lcd_init(LCD_DISP_ON);
	rover_init();
	
	while(1)
	{
		rover_stop();
		
		while(PIND & (1<<PD0))
		{
			if(PIND & (1<<PD5))
				rover_straight(FORWARD, MAXSPEED);
			else if(PIND & (1<<PD6))
				rover_straight(BACKWARD, MAXSPEED);
			else if(PIND & (1<<PD7))
				rover_turn_right(MAXSPEED);
			else if(PIND & (1<<PD4))
				rover_turn_left(MAXSPEED);
			else
				rover_stop();
		}
	}//end of while
}//end of main
