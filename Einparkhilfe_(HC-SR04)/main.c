#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "usound.c"
#include "lcd.h"
#include "rotary.c"
#define F_CPU 16000000
#include <util/delay.h>

#define Buzzer PA4

#define BuzEin 300

enum{normBetrieb, einstellung};

//------------------------------MAIN------------------------------------
int main(void)
{
	
	DDRA = 0x10;			//Buzzer
	DDRD = 0x00;			//Schalterport
	DDRB = 0XFD;			//LEDsport
	DDRC = 0xFF;			//LCDdaten

	char output[16];
	unsigned long dist = 0;
	unsigned char settings = normBetrieb;
	unsigned char BuzDauerhaft = 40;
	volatile unsigned long Cntr = 0;
	volatile unsigned long i = 0;


	lcd_init(LCD_DISP_ON);  //LCD initialisieren

	usound_init();			//HC-SR04 init

	while(1)
	{
		usrInput(PIND);		//detect rotary input


		//Change mode if Rotary is pressed
		if(rotary.press)
		{
			settings ^= 1;
			lcd_clrscr();
		}
		
		switch(settings)
		{
			case einstellung:
//------------------SET BUZZER ON DISTANCE---
				if(rotary.left && (BuzDauerhaft > 0)) BuzDauerhaft--;
				if(rotary.right && (BuzDauerhaft < 99)) BuzDauerhaft++;
				
				lcd_gotoxy(0,0);
				lcd_puts("Buzzer dist:");
				
				sprintf(output, "%02dCM", (short)BuzDauerhaft);
				lcd_gotoxy(0,1);
				lcd_puts(output);
//-------------------------------------------
			break;
			
			case normBetrieb:
//-----------MEASURE DISTANCE----------------
				dist = 0;
				for(i = 0; i < 10; i++)			//Durchschnitt aus 10 messungen
				{
					dist += readDistance();		//Dist Measuring function
				}
				dist /= 10;
//----------------BUZZER---------------------
				//Buzzer on/off switch
				if(PIND & (1<<PD0))
				{
					
					if((dist <= BuzEin) && (dist >= (BuzDauerhaft + 1)))	//Buzzer buzzing
					{
						PORTA |= (1 << Buzzer);
						_delay_ms(50);
						PORTA &= ~(1 << Buzzer);
						_delay_ms(dist);
					}
					else if((dist <= BuzDauerhaft) && (dist > 0))
					{
						PORTA |= (1 << Buzzer);			//Buzzer on
					}
					else
					{
						PORTA &= ~(1 << Buzzer);		//Buzzer off
					}
				}
				else
				{
					PORTA &= ~(1 << Buzzer);		//Buzzer aus
				}
//----------------OUTPUT---------------------
				lcd_gotoxy(0,0);
				lcd_puts("Distance:");

				sprintf(output, "%04dCM", (short)dist);
				lcd_gotoxy(0,1);
				lcd_puts(output);
//-------------------------------------------
			break;
		}//end of Switch
	}//end of while
}//end of main
