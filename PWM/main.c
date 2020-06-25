#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include "serial.c"
#include "rotary.c"
#include "adc.c"

//------------------------------MAIN------------------------------------
int main(void)
{	
	DDRD = 0x3F;			//Schalterport
	DDRB = 0Xff;			//LEDsport
	DDRC = 0xff;			//LCDdaten
	DDRA = 0xf0;			//LCDctrl

	char output[16];
	unsigned short adc = 0;
	unsigned short volt = 0;
	unsigned short komaVolt = 0;

	lcd_init(LCD_DISP_ON);

	ADC_init(0x0F);

	TCCR1A = 0xA1;			//PWM initialisierung 8bit
	TCCR1B = 0x01;			//Datenblatt: s.131

	//TCCR1A = 0xA3;			//PWM initialisierung 10bit
	//TCCR1B = 0x09;			//Datenblatt: s.131

	OCR1A = 0x00;			//PWM output A
	OCR1B = 0x80;			//PWM output B = 50%
	
	while(1)
	{
		usrInput(PIND);

		if(rotary.left) OCR1A--;
		if(rotary.right) OCR1A++;
		if(PIND & (1<<PD0))
		{
			OCR1A++;
			_delay_ms(100);
		}

//--------------------------------------------------
//PWM A über RC glied auf ADC0 verdrahten

		adc = read_ADC(0);
		
		volt = ((long)adc * 10000) / 2046;

		PORTB = OCR1A;

		komaVolt = volt % 1000;
		volt /= 1000;

		sprintf(output, "%d,%03d", volt, komaVolt);
		lcd_gotoxy(0, 0);
		lcd_puts(output);

	}//end of while
}//end of main
