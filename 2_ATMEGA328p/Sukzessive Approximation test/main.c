#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#define F_CPU 8000000UL
#include <util/delay.h>
#include "rotary.c"

#define ENTPRELLUNG 500


//------------------------------MAIN------------------------------------
int main(void)
{
	unsigned char adcCntr = 0;
	unsigned char adcValue = 0;

	DDRB = 0x02;
	PORTB = 0x80;
	DDRD = 0xFF;

	TCCR1A = 0xA1;			//PWM initialisierung 8bit
	TCCR1B = 0x09;			//Datenblatt: s.131
	
	while(1)
	{
		if((~PINB) & (1<<PB7))
		{
			_delay_ms(ENTPRELLUNG);
			adcCntr++;
		}

		switch(adcCntr)
		{
			case 0:
				OCR1A = 0x80;
				_delay_ms(50);
				if(PINB & (1<<PB2))
					adcValue |= (1<<7);
				else
					adcValue &= ~(1<<7);
			break;

			case 1:
				OCR1A = adcValue;
				OCR1A |= (1<<6);
				_delay_ms(50);
				if(PINB & (1<<PB2))
					adcValue |= (1<<6);
				else
					adcValue &= ~(1<<6);
			break;

			case 2:
				OCR1A = adcValue;
				OCR1A |= (1<<5);
				_delay_ms(50);
				if(PINB & (1<<PB2))
					adcValue |= (1<<5);
				else
					adcValue &= ~(1<<5);
			break;

			case 3:
				OCR1A = adcValue;
				OCR1A |= (1<<4);
				_delay_ms(50);
				if(PINB & (1<<PB2))
					adcValue |= (1<<4);
				else
					adcValue &= ~(1<<4);
			break;

			case 4:
				OCR1A = adcValue;
				OCR1A |= (1<<3);
				_delay_ms(50);
				if(PINB & (1<<PB2))
					adcValue |= (1<<3);
				else
					adcValue &= ~(1<<3);
			break;

			case 5:
				OCR1A = adcValue;
				OCR1A |= (1<<2);
				_delay_ms(50);
				if(PINB & (1<<PB2))
					adcValue |= (1<<2);
				else
					adcValue &= ~(1<<2);
			break;
			
			case 6:
				OCR1A = adcValue;
				OCR1A |= (1<<1);
				_delay_ms(50);
				if(PINB & (1<<PB2))
					adcValue |= (1<<1);
				else
					adcValue &= ~(1<<1);
			break;
			
			case 7:
				OCR1A = adcValue;
				OCR1A |= (1<<0);
				_delay_ms(50);
				if(PINB & (1<<PB2))
					adcValue |= (1<<0);
				else
					adcValue &= ~(1<<0);
			break;

			case 8:
				PORTD = 0xff;
				_delay_ms(500);
				PORTD = 0x00;
				_delay_ms(500);
				adcCntr++;
			break;

			case 9:
			break;

			default:
				adcCntr = 0;
				adcValue = 0;
			break;
		}

		PORTD = adcValue;
//------------------------------------------
	}//end of while
}//end of main
