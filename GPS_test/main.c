#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include "serial.c"
//#include "rotary.c"


//------------------------------MAIN------------------------------------
int main(void)
{
	char Input[50];
	char Output[16];
	short speed, time;
	
	DDRD = 0x02;			//Schalterport
	DDRB = 0xff;			//LEDsport
	DDRC = 0xff;			//LCDdaten
	DDRA = 0xf0;			//LCDctrl & ADC

	_delay_ms(500);

	lcd_init(LCD_DISP_ON);  	//Initialisieren
	USART_Init(103);
	
	while(1)
	{
		PORTB = PIND;
		
		USART_Receive_STRING(Input);
			
		sscanf(Input,"$GPRMC,%4d,,,,,,%d", &time, &speed);

		sprintf(Output, "%d, %03d", (time + 200), speed);
		
		lcd_gotoxy(0,0);
		lcd_puts(Output);
//------------------------------------------
	}//end of while
}//end of main
