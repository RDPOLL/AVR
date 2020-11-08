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
	char Input[8][160];
	char Output[20];
	float x, y, speed, course;
	float time, ret;
	char flag, xf, yf;
	int buf_cnt=0;
	int c=0;
	int a=0;

	DDRD = 0x02;			//Schalterport
	DDRB = 0xff;			//LEDsport
	DDRC = 0xff;			//LCDdaten
	DDRA = 0xf0;			//LCDctrl & ADC

	lcd_init(LCD_DISP_ON);  	//Initialisieren
	USART_Init(103);

	while(1)
	{
		/*
		if(USART_check_RX())
		{
		*/
		for(c=0; c<8; c++)
			USART_Receive_STRING(Input[c]);

			
		for( c=0;c<8; c++)
		{
			USART_Transmit_STRING(Input[c]);
			USART_Transmit_STRING("\n\r");
		
			if(strncmp("$GPRMC", Input[c],6)==0)
			{
				sscanf(Input[c], "$GPRMC,%f,%c,%f,%c,%f,%c,%f,,%d,,,%*s", &time,&flag,&x,&xf,&y,&yf,&speed,&course);

				if(time < 230000)
					time += 10000;
				else
					time -= 230000;
					
				sprintf(Output, "%06.0f, %3.1f",time, speed);
				lcd_gotoxy(0,0);
				lcd_puts(Output);
			}
		}
//------------------------------------------
	}//end of while
}//end of main
