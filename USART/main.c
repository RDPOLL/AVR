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
	DDRD = 0x02;			//Schalterport
	DDRB = 0xff;			//LEDsport
	DDRC = 0xff;			//LCDdaten
	DDRA = 0xf0;			//LCDctrl & ADC

	unsigned char output[16];
	unsigned char x = 0;
	//volatile unsigned long i = 0;

	//initialiting USART and SPI
	USART_Init(103);

	USART_Transmit_STRING("\n\rStartup complete");

	lcd_init(LCD_DISP_ON);  	//Initialisieren
	
	while(1)
	{
		usrInput(PIND);
//----------OUTPUT---------------------
		
		sprintf(output, "%c", USART_Receive());

		USART_Transmit_STRING("\n\rRX");

		lcd_gotoxy(x, 0);
		lcd_puts(output);
		
		if((x >= 0) && (x <= 15))
		{			
			x++;
		}
		else
		{
			lcd_clrscr();
			x = 0;
		}
		
//------------------------------------------
	}//end of while
}//end of main
