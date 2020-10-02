#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include "serial.c"
#include "rotary.c"

int lcd_putchar(char c, FILE *stream)
{
	lcd_putc(c);
	return 0;
}

FILE lcd_str = FDEV_SETUP_STREAM(lcd_putchar, NULL, _FDEV_SETUP_WRITE);

//------------------------------MAIN------------------------------------
int main(void)
{	
	DDRD = 0x02;			//Schalterport
	DDRB = 0xff;			//LEDsport
	DDRC = 0xff;			//LCDdaten
	DDRA = 0xf0;			//LCDctrl & ADC

	//char output[16];
	unsigned char input = 0;
	unsigned short hori = 8;
	unsigned short taster = 0;
	//volatile unsigned long i = 0;

	//initialiting USART and SPI
	USART_Init(103);
	SPI_MasterInit(1);

	stdout = &uart_str;			//print uart_str in StandartOutput FILE
	stderr = &lcd_str;			//print lcd_str in StandartError FILE

	printf("\n\rStartup complete");
	//SPI_MasterTransmit_string("Startup complete", 0);

	lcd_init(LCD_DISP_ON);  	//Initialisieren
	
	while(1)
	{
		usrInput(PIND);
		input = PIND;
		if(rotary.right)
		{
			 hori++;
			 lcd_clrscr();
		}
		if(rotary.left)
		{
			 hori--;
			 lcd_clrscr();
		}
		if(rotary.press)
		{
			taster ^= 1;
			lcd_clrscr();
		}
		
//----------OUTPUT---------------------

		PORTB = input;

		printf("\n\r%d, %d", taster, hori);

		lcd_gotoxy(0,0);
		//lcd_puts("STD AVR PROJEKT");
		fprintf_P(stderr,PSTR("STD AVR PROJECT1"));
		
		lcd_gotoxy(hori,1);
		lcd_putc('*');

		//sprintf(output, "%0d", taster);
		lcd_gotoxy(0,1);
		//lcd_puts(output);
		fprintf_P(stderr,PSTR( "%0d"), taster);
		
		//sprintf(output, "%02d", hori);
		lcd_gotoxy(14,1);
		//lcd_puts(output);
		fprintf_P(stderr,PSTR( "%02d"), hori);
		
//------------------------------------------
	}//end of while
}//end of main
