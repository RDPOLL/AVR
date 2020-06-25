#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>

#define PULS 100UL			//PULS in MS
#define PERIODE 200UL		//Periode in MS
#define BlinkPin 0xFF		//Hex value of BlinkPin(s)

//------------------------------MAIN------------------------------------
int main(void)
{	
	DDRD = 0x00;			//Schalterport
	DDRB = 0xff;			//LEDsport
	//DDRC = 0xff;			//LCDdaten
	//DDRA = 0xf0;			//LCDctrl & ADC

	unsigned char blinklicht = 0;
	unsigned long blinkCntr = 0;
	
	while(1)
	{
		if(PIND & (1<<PD0))
		{
//==================================================
			blinkCntr++;
			if(blinklicht == 1)
			{
				if(blinkCntr >= (PULS * 700))
				{
					blinkCntr = 0;
					blinklicht = 0;
				}
			}
			else
			{
				if(blinkCntr >= ((PERIODE - PULS) * 700))
				{
					blinkCntr = 0;
					blinklicht = 1;
				}
			}
			
//----------OUTPUT----------------------------------

			if(blinklicht)
				PORTB |= BlinkPin;
			else
				PORTB &= ~BlinkPin;

			//PORTB = (blinklicht == 1) ? PORTB | BlinkPin: PORTB & (~BlinkPin);
//==================================================
		}
	}//end of while
}//end of main
