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
	char Input[80];
	char Output[20];
	long x, y, speed, course;
	long time, flag, xf, yf, ret;
	
	DDRD = 0x02;			//Schalterport
	DDRB = 0xff;			//LEDsport
	DDRC = 0xff;			//LCDdaten
	DDRA = 0xf0;			//LCDctrl & ADC

	lcd_init(LCD_DISP_ON);  	//Initialisieren
	USART_Init(109);
	
	while(1)
	{
		if(USART_check_RX)
			USART_Receive_STRING(Input);
		
		sscanf(Input,"$GPRMC,%d,%c,%f,%c,%f,%f,%*d,,,%*s", &time,&flag,&x,&xf,&y,&yf,speed,&course);

		sprintf(Output, "%d, %d",time, speed);
		lcd_gotoxy(0,0);
		lcd_puts(Output);
//------------------------------------------
	}//end of while
}//end of main
