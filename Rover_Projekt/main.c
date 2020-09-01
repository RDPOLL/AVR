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
	unsigned char i = 0;
	
	lcd_init(LCD_DISP_ON);
	rover_init();
	
	while(1)
	{
		for(i = 0; i < 255; i++)
		{
			rover_straight(FORWARD, i);
			_delay_ms(30);
		}
		 
		rover_stop();
		_delay_ms(2000);
		
		rover_straight(BACKWARD, MAXSPEED);
		_delay_ms(5000);
	}//end of while
}//end of main
