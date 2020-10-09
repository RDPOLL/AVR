#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/sleep.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/interrupt.h>
#include "adc.c"
#include "lcd.h"
#include "rotary.c"
#include "rover.c"
#include "serial.c"
#include "hc12.c"
#include "usound.c"

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

#define MINDIST 35


unsigned char scanOnOff = 0;
unsigned short sense[17];
unsigned char scanDir = 0;


//Rover var
unsigned short extInt0Cntr = 0;
unsigned short extInt1Cntr = 0;

unsigned char roverSetSpeedR = 0;
unsigned char roverSetSpeedL = 0;

unsigned char roverVarSpeedR = 0;
unsigned char roverVarSpeedL = 0;

unsigned char roverDirR = 0;
unsigned char roverDirL = 0;


ISR(TIMER1_OVF_vect)
{
	unsigned short measSpeedR = 0;
	unsigned short measSpeedL = 0;
	
	measSpeedR = extInt0Cntr;
	extInt0Cntr = 0;
	measSpeedL = extInt1Cntr;
	extInt1Cntr = 0;

//-----PID Control---------
	//Rightside
	if(roverSetSpeedR == 0)
	{
		roverVarSpeedR = 0;
	}
	else if((measSpeedR < roverSetSpeedR) && (roverVarSpeedR < 255))
	{
		roverVarSpeedR++;
	}
	else if((measSpeedR > roverSetSpeedR) && (roverVarSpeedR > 0))
	{
		roverVarSpeedR--;
	}

	//Leftside
	if(roverSetSpeedL == 0)
	{
		roverVarSpeedL = 0;
	}
	else if((measSpeedL < roverSetSpeedL) && (roverVarSpeedL < 255))
	{
		roverVarSpeedL++;
	}
	else if((measSpeedL > roverSetSpeedL) && (roverVarSpeedL > 0))
	{
		roverVarSpeedL--;
	}
//--------------------------------

//----PWM Control--------
	if(roverDirR)
	{
		PORTB |= (1<<DIRRPIN);
		SPEEDRIGHT = (255 - roverVarSpeedR);
	}
	else
	{
		PORTB &= ~(1<<DIRRPIN);
		SPEEDRIGHT = (255 - roverVarSpeedR);
	}

	if(roverDirL)
	{
		PORTB |= (1<<DIRLPIN);
		SPEEDLEFT = (255 - roverVarSpeedL);
	}
	else
	{
		PORTB &= ~(1<<DIRLPIN);
		SPEEDLEFT = (255 - roverVarSpeedL);
	}
}

ISR(INT0_vect)
{
	extInt0Cntr++;
}

ISR(INT1_vect)
{
	extInt1Cntr++;
}

ISR(INT2_vect)
{
	scanOnOff ^= 1;
}

unsigned short scanObstical(void)
{
	unsigned short dist = 0;
	unsigned short obstical = 0;
	unsigned short i = 0;

//=====================================
//scanning
	if(scanDir)
	{
		for(i = 0; i < 17; i++)
		{
			OCR1B = (i + 14);

			_delay_ms(30);

			dist = readDistance();

			sense[i] = dist;
		}
		scanDir = 0;
	}
	else
	{
		for(i = 17; i > 0; i--)
		{
			OCR1B = (i + 14);

			_delay_ms(30);

			dist = readDistance();

			sense[i] = dist;
		}
		scanDir = 1;
	}

//=================================================
//Readout

	for(i = 1; i < 15; i++)
	{
		if(sense[i] <= MINDIST)
		{
			if(((sense[i-1] <= (sense[i] + 10)) && (sense[i-1] >= (sense[i] - 10))) && ((sense[i+1] <= (sense[i] + 10)) && (sense[i+1] >= (sense[i] - 10))))
			{
				obstical |= (1<<(15 - i));
			}
		}
	}
	
	return obstical;
}

//------------------------------MAIN------------------------------------
int main(void)
{
	unsigned short i = 0;
	unsigned short obsTmp = 0;
	unsigned char speed = 0;
	char output[16];

	DDRA = 0x00;
	//DDRB = 0xff;
	DDRC = 0xff;			//LCD
	DDRD |= (1<<PD4);

	PORTB = 0x07;

	//PWM output B init
	TCCR1A |= (1<<COM1B1);
	//set servo to 50%
	OCR1B = 23;

	//ext interupts settings
	EICRA |= (1<<ISC21);
	EIMSK |= (1<<INT2);
	
//Lib Initialisieren
	lcd_init(LCD_DISP_ON);  	
	PORTC |= (1<<PC7);

	USART_Init(103);
	
	HC_setPower(1);
	
	rover_init();

	usound_init();
//-------------------
	
	while(1)
	{		
		usrInput(PINB);

		//Setting the Speed
		if(rotary.right && (speed < 50))
		{
			speed++;
		}
		else if(rotary.left && (speed > 0))
		{
			speed--;
		}


		if(scanOnOff)
		{
			
			obsTmp = scanObstical();

			if(obsTmp & (1<<8))
			{
				rover_stop();
				rover_turn_right(speed);
			}
			else if((obsTmp & (1<<2)) || (obsTmp & (1<<5)))
			{
				rover_turn_left(speed);
			}
			else if((obsTmp & (1<<11)) || (obsTmp & (1<<14)))
			{
				rover_turn_right(speed);
			}
			else
				rover_straight(FORWARD, speed);

			sprintf(output, BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY((obsTmp >> 8)));
			//USART_Transmit_STRING(output);
			lcd_gotoxy(0,0);
			lcd_puts(output);

			sprintf(output, BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(obsTmp));
			//USART_Transmit_STRING(output);
			USART_Transmit_STRING("\n\r");
			lcd_gotoxy(8,0);
			lcd_puts(output);
		}
		else
		{
			OCR1B = 23;
			sense[8] = readDistance();

			if(sense[8] < MINDIST)
			{
				rover_stop();
			}
			else
			{
				rover_straight(FORWARD, speed);
			}

			sprintf(output, "Speed: %03d", speed);
			lcd_gotoxy(0,1);
			lcd_puts(output);

			sprintf(output, "Distance: %03d   ", sense[8]);
			USART_Transmit_STRING(output);
			USART_Transmit_STRING("\n\r");
			lcd_gotoxy(0,0);
			lcd_puts(output);
		}

//------------------------------------------
	}//end of while
}//end of main
