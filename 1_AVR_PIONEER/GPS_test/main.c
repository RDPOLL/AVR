#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <string.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include "serial.c"
//#include "rotary.c"

unsigned char rxChar = 0;
unsigned char rxCommand = 0;
unsigned char buffer[100];
char GPSinput[7][100];

ISR(USART0_RX_vect)
{
	buffer[rxChar] = UDR0;
	
	if(buffer[rxChar] == '\n')
	{
		buffer[rxChar] = '\0';
		rxChar = 0;
		
		if(strcmp("$GPRMC", buffer) == 0)
		{
			strcpy(GPSinput[0], buffer);
		}
		else if(strcmp("$GPVTG", buffer) == 0)
		{
			strcpy(GPSinput[1], buffer);
		}
	}
	else
		rxChar++;
}


//------------------------------MAIN------------------------------------
int main(void)
{
	char Input[8][160];
	char Output[20];
	float x, y, speed, course;
	float time, ret;
	char flag, xf, yf;
	int buf_cnt = 0;
	int i = 0;

	DDRD = 0x02;			//Schalterport
	DDRB = 0xff;			//LEDsport
	DDRC = 0xff;			//LCDdaten
	DDRA = 0xf0;			//LCDctrl & ADC

	sei();

	lcd_init(LCD_DISP_ON);  	//Initialisieren
	USART_Init(103);

	while(1)
	{
		for(i = 0; i < 3; i++)
		{
			USART_Transmit_STRING(GPSinput[i]);
		}
		sscanf(GPSinput[0], "$GPRMC,%f,%c,%f,%c,%f,%c,%f,,%d,,,%*s", &time,&flag,&x,&xf,&y,&yf,&speed,&course);
			
		sprintf(Output, "%06.0f, %3.1f",time, speed);
		lcd_gotoxy(0,0);
		lcd_puts(Output);

		_delay_ms(1000);
//------------------------------------------
	}//end of while
}//end of main
