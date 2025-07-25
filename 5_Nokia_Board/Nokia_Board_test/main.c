#include <avr/io.h>
#include "glcd/glcd.h"
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include "glcd/fonts/Liberation_Sans15x21_Numbers.h"
#include "glcd/fonts/font5x7.h"
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <limits.h>


#define F_CPU 16000000UL  // 1 MHz

void borders(unsigned char x, unsigned char y)
	{
	
	
	
	}
	
int main(void)
{	
	/* Backlight pin PL3, set as output, set high for 100% output */
	DDRB |= (1<<PB2);
	PORTB |= (1<<PB2);
	
    sei();
    // enable interrupts
	
	glcd_init();
	
	glcd_clear();
	glcd_write();
		
	// Display
	GLCD_TEXT_INIT();
	glcd_clear_buffer();

	unsigned short x = 0;
	unsigned char y = 0;

	while(1) 
		{
		glcd_clear_buffer();
		glcd_fill_circle(32, x++, 2, 1);
		glcd_write();
		delay_ms(100);
		}
		

}
