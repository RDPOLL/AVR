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

	unsigned char Output[16];
	unsigned char output = 0;
	unsigned long cntr = 0;
	unsigned char x = 0;
	//volatile unsigned long i = 0;

	//initialiting USART
	USART_Init(103);

	USART_Transmit_STRING("\n\rStartup complete");

	lcd_init(LCD_DISP_ON);  	//Initialisieren
	
	while(1)
	{
		usrInput(PIND);
//----------OUTPUT---------------------

		cntr++;

		sprintf(Output, "\n\r%d", cntr);
		USART_Transmit_STRING(Output);

		if(USART_check_RX())
		{
			cntr = 0;
			output = USART_Receive();
			
			if((x >= 0) && (x <= 15))
			{
				if(output == '\b')
				{
					x--;
					lcd_gotoxy(x, 0);
					lcd_putc(' ');
				}
				else if(output == '\r')
				{
					x = 0;
					lcd_clrscr();
				}
				else
				{
					lcd_gotoxy(x, 0);
					lcd_putc(output);
					
					x++;
				}
			}
			else
			{
				lcd_clrscr();
				x = 0;
			}
		}

		_delay_ms(5000);	//for VT420

//------------------------------------------
	}//end of while
}//end of main
