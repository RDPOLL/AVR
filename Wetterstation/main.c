#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include "serial.c"
#include "rotary.c"


//------------------------------MAIN------------------------------------
int main(void)
{
	unsigned char Input[16];
	unsigned char String[16];
	short temperature = 0;
	unsigned char tempKomma = 0;
	
	DDRD = 0x02;			//Schalterport
	DDRB = 0xff;			//LEDsport
	DDRC = 0xff;			//LCDdaten
	DDRA = 0xf0;			//LCDctrl & ADC

	lcd_init(LCD_DISP_ON);  	//Initialisieren
	USART_Init(103);
	
	while(1)
	{
		if(USART_check_RX())
		{	
			USART_Receive_STRING(Input);
			temperature = atoi(Input);

			if(temperature > 0)
			{
				tempKomma = temperature % 10;
				temperature /= 10;
			}
			else
			{
				temperature *= -1;
				tempKomma = temperature % 10;
				temperature /=  10;
				temperature *= -1;
			}
			sprintf(String, "%03d,%01dC", temperature, tempKomma);
			
			lcd_gotoxy(0, 0);
			lcd_puts(String);
		}
//------------------------------------------
	}//end of while
}//end of main