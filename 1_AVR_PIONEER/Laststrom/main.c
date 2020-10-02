#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include "rotary.c"
#include "adc.c"

#define linearOffset 0		//Offset in mA
#define gainOffset 916		//Faktor 916 sollte ok sein

#define maxStrom 1000		//Setzten des Maximalen Stromes
#define maxPower 30000		//Setzten der Maximalen Leistung


//------------------------------MAIN------------------------------------
int main(void)
{	
	DDRD = 0x3F;			//Schalterport
	DDRB = 0xff;			//LEDsport
	DDRC = 0xff;			//LCDdaten
	DDRA = 0xf0;			//LCDctrl & ADC

	char output[16];
	unsigned short sollStrom = 0;
	unsigned short istStrom = 0;
	unsigned short pwm = 0;
	unsigned short volt = 0;
	unsigned short messPower = 0;
	unsigned short setPower = 0;
	volatile unsigned long i = 0;
	unsigned char modus = 0;

	enum
	{
		Strom_mA,
		Strom_100mA,
		Leistung_mW,
		Leistung_W
	};

	lcd_init(LCD_DISP_ON);  	//Initialisieren
	ADC_init(0x03);				//AD0 und AD1 einschalten

//----PWM Init------------------------------------

	TCCR1A = 0xA3;				//PWM initialisierung
	TCCR1B = 0x09;				//Datenblatt: s.131

//------------------------------------------------
	
	while(1)
	{
		usrInput(PIND);
		
		if(PIND & (1 << PD3))
		{
			if(rotary.right)
			{
				modus++;
				rotary.right = 0;
			}
			if(rotary.left)
			{
				modus--;
				rotary.left = 0;
			}
		}

//Strommessung: Durchschnitt aus 10 Messungen wird genommen
//--------------------------------------------------------------
		istStrom = 0;
		for(i = 10; i > 0; i--)
		{
			istStrom += ((((long)read_ADC(1)) * 1000) / gainOffset);
		}
		istStrom /= 10;

		istStrom += linearOffset;							//Offset des ADC kompensieren
//--------------------------------------------------------------
		
		volt = (((long)read_ADC(0)) * 10000) / 2046;		//Spannung wird gemessen
		volt *= 6;

		messPower = (((long)volt / 100) * istStrom) / 10;		//Verlustleistung wird berechnet

		switch(modus)
		{
			case Strom_mA:
				if(rotary.right && (sollStrom < maxStrom))
				{
					 sollStrom++;
				}
				
				if(rotary.left && (sollStrom > 0))
				{
					 sollStrom--;
				}
			break;
			
			case Strom_100mA:
				if(rotary.right && (sollStrom <= (maxStrom - 100)))
				{
					 sollStrom += 100;
				}
				
				if(rotary.left && (sollStrom >= (0 + 100)))
				{
					 sollStrom -= 100;
				}
			break;
			
			case Leistung_mW:
				if(rotary.right && (setPower < maxPower))
				{
					 setPower += 10;
				}
				
				if(rotary.left && (setPower > 0))
				{
					 setPower -= 10;
				}

				sollStrom = ((long)setPower * 1000) / volt;
			break;

			case Leistung_W:
				if(rotary.right && (setPower <= (maxPower - 1000)))
				{
					 setPower += 1000;
				}
				
				if(rotary.left && (setPower >= (0 + 1000)))
				{
					 setPower -= 1000;
				}

				sollStrom = ((long)setPower * 1000) / volt;
			break;

			default:
				modus = 0;
				sollStrom = 0;
				setPower = 0;
			break;
		}


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

//----------OUTPUT--------------------------

		sprintf(output, "%04d %04d %04d %01d", sollStrom, setPower, pwm, modus);
		lcd_gotoxy(0,0);
		lcd_puts(output);
		
		sprintf(output, "%04d %05d %05d", istStrom, volt, messPower);
		lcd_gotoxy(0,1);
		lcd_puts(output);
		
//------------------------------------------
	}//end of while
}//end of main
