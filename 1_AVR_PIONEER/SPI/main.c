#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include "serial.c"

unsigned char input = 0;

void digPot_set(unsigned short res)
{
	SPI_MasterTransmit(0x04 | ((res & 0x300) >> 8), 1);
	SPI_MasterTransmit((res & 0xff), 1);
}

//------------------------------Flankentriggerung-----------------------

struct
{
	unsigned char act;					//Variabeln fuer die Flankentriggerung
	unsigned char old;
	unsigned char flk;
	unsigned char pos_flk;
	unsigned char neg_flk;
}flk = {0};

void flk_t(unsigned char in)			//PORT in uebergabe wert angeben
{
	flk.act = in;
	flk.flk = flk.act ^ flk.old;
	flk.neg_flk = flk.flk & flk.old;
	flk.pos_flk = flk.flk & flk.act;
	flk.old = flk.act;

	return;
}
//------------------------------END-------------------------------------

//------------------------------Rotary----------------------------------

struct
{
	unsigned char right;
	unsigned char left;
	unsigned char press;
}rotary = {0};

void rotary_f(void)
{	
	rotary.right = 0;
	rotary.left = 0;
	rotary.press = 0;
	
	if(flk.neg_flk & (1 << PD3)) rotary.press = 1;
	if((flk.neg_flk & (1 << PD7)) && (input & (1 << PD6))) rotary.right = 1;
	if((flk.neg_flk & (1 << PD6)) && (input & (1 << PD7))) rotary.left = 1;
	
	return;
}

//------------------------------END-------------------------------------

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
	DDRB = 0Xff;			//LEDsport
	DDRC = 0xff;			//LCDdaten
	DDRA = 0xff;			//LCDctrl

	unsigned char output[16];
	unsigned short hori = 0;
	unsigned short taster = 0;
	unsigned short widerstand = 0;
	//volatile unsigned long i = 0;

	USART_Init(416);
	USART_Transmit_STRING("\n\rUSART initialized");
	
	SPI_MasterInit(0);
	USART_Transmit_STRING("\n\rSPI initialized");

	stdout = &uart_str;
	stderr = &lcd_str;

	lcd_init(LCD_DISP_ON);  //Initialisieren
	
	printf("\n\rStartup complete");
	
	while(1)
	{
		input = PIND;
		flk_t(input);
		rotary_f();
		
		if(rotary.right)
		{
			 widerstand ++;
			 lcd_clrscr();
		}
		if(rotary.left)
		{
			 widerstand --;
			 lcd_clrscr();
		}
		if(rotary.press)
		{
			taster ^= 1;
			lcd_clrscr();
		}
		
//----------OUTPUT---------------------
		
		printf("\n\r %d, %04d", taster, widerstand);

		digPot_set(widerstand);
		
		lcd_gotoxy(0,0);
		fprintf_P(stderr, PSTR( "%0d"), taster);

		lcd_gotoxy(1,0);
		fprintf_P(stderr, PSTR(", "));

		lcd_gotoxy(3,0);
		fprintf_P(stderr, PSTR( "%04d"), widerstand);
		
//------------------------------------------
	}//end of while
}//end of main
