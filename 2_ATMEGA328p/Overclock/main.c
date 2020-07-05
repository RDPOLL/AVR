#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#define F_CPU 38000000UL
#include <util/delay.h>

/*
 *	Der Test hat bis 38MHz funktioniert. Allerdings nicht zuverlässig. 
 */

//------------------------------MAIN------------------------------------
int main(void)
{	
	DDRC = 0x00;
	DDRB = 0xFF;
	
	while(1)
	{
		PORTB |= (1<<PB7);
		_delay_ms(500);
		PORTB &= ~(1<<PB7);
		_delay_ms(500);
//------------------------------------------
	}//end of while
}//end of main
