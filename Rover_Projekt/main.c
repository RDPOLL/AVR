#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
//#include <avr/interrupt.h>
//#include "lcd.h"
//#include "serial.c"
#include "rover.c"


//------------------------------MAIN------------------------------------
int main(void)
{	
	initRover();
	
	while(1)
	{
		moveRover(FORWARD, 255, FORWARD, 255);
		_delay_ms(5000);
		moveRover(FORWARD, 255,BACKWARD, 255);
		_delay_ms(1000);
		moveRover(BACKWARD, 255,FORWARD, 255);
		_delay_ms(3000);
	}//end of while
}//end of main
