#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include "rotary.c"
#include "adc.c"

#define linearOffset 7		//Offset in mA
#define gainOffset 916		//Faktor 916 sollte ok sein

unsigned short pwm = 0;

void messInit()
{
	ADC_init(0x03);				//AD0 und AD1 einschalten
//----PWM Init------------------------------------
	TCCR1A = 0xA3;				//PWM initialisierung
	TCCR1B = 0x09;				//Datenblatt: s.131
//------------------------------------------------
}

unsigned short setLast_readVolt(unsigned short sollStrom)
{
		unsigned short istStrom = 0;
		unsigned short volt = 0;
		unsigned char i = 0;
		
//Strommessung: Durchschnitt aus 10 Messungen wird genommen
//--------------------------------------------------------------
		istStrom = 0;
		for(i = 10; i > 0; i--)
		{
			istStrom += ((((long)read_ADC(1)) * 1000) / gainOffset);
		}
		istStrom /= 10;

		istStrom += linearOffset;							//Offset des ADC kompensieren
		
//Spannung wird gemessen		
//--------------------------------------------------------------

		volt = (((long)read_ADC(0)) * 10000) / 2046;		
		volt *= 6;
		
//Kontroll Code: Steuern des Mosfets
//--------------------------------------------------------------
		if((istStrom < sollStrom) && (pwm < 1023)) pwm++;
		if((istStrom > sollStrom) && (pwm > 0)) pwm--;
		if(sollStrom == 0) pwm = 0;
//--------------------------------------------------------------

		OCR1AL = pwm;
		OCR1AH = (pwm >> 8);

		OCR1BL = pwm;
		OCR1BH = (pwm >> 8);

		return volt;
}

//------------------------------MAIN------------------------------------
int main(void)
{	
	DDRD = 0x3F;			//Schalterport
	DDRB = 0xff;			//LEDsport
	DDRC = 0xff;			//LCDdaten
	DDRA = 0xf0;			//LCDctrl & ADC

	char output[16];
	unsigned short Strom = 0;
	unsigned short Spannung = 0;

	lcd_init(LCD_DISP_ON);  	//Initialisieren
	messInit();

	while(1)
	{
		usrInput(PIND);
		
		if(rotary.right && (Strom < 120))
		{
			 Strom++;
		}
		
		if(rotary.left && (Strom > 0))
		{
			 Strom--;
		}

		Spannung = setLast_readVolt(Strom);

//----------OUTPUT--------------------------

		sprintf(output, "%05d %04d", Spannung, Strom);
		lcd_gotoxy(0,0);
		lcd_puts(output);
		
//------------------------------------------
	}//end of while
}//end of main
