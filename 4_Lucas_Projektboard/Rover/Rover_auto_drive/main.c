#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/sleep.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
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

#define MINDIST 30
#define SCANDELAY 60
#define MINBATTVOLT 7400 //In mV

unsigned char scanDir = 0;
unsigned char scanOnOff = 0;
unsigned short sense[17];
unsigned char speed = 0;


//Rover var
volatile unsigned short extInt0Cntr = 0;
volatile unsigned short extInt1Cntr = 0;

volatile unsigned char roverSetSpeedR = 0;
volatile unsigned char roverSetSpeedL = 0;

volatile unsigned char roverVarSpeedR = 0;
volatile unsigned char roverVarSpeedL = 0;

volatile unsigned char roverDirR = 0;
volatile unsigned char roverDirL = 0;


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
		if(roverVarSpeedR == 0)
			roverVarSpeedR = INITSTARTSPEED;
		else
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
		if(roverVarSpeedL == 0)
			roverVarSpeedL = INITSTARTSPEED;
		else
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

unsigned short obstical(void)
{
	unsigned short dist = 0;
	unsigned short obstical = 0;
	unsigned short i = 0;


	//scanning
	if(scanDir)
	{
		for(i = 0; i < 17; i++)
		{
			OCR1B = (i + 15);

			_delay_ms(SCANDELAY);

			dist = readDistance();

			sense[i] = dist;
		}
		scanDir = 0;
	}
	else
	{
		for(i = 17; i > 0; i--)
		{
			OCR1B = ((i-1) + 15);

			_delay_ms(SCANDELAY);

			dist = readDistance();

			sense[(i-1)] = dist;
		}
		scanDir = 1;
	}

//=================================================
//calculate an obstical

	for(i = 1; i <= 15; i++)
	{
		if(sense[i] <= MINDIST)
		{
			if(((sense[i-1] <= (sense[i] + 10)) && (sense[i-1] >= (sense[i] - 10))) && ((sense[i+1] <= (sense[i] + 10)) && (sense[i+1] >= (sense[i] - 10))))
			{
				obstical |= (1<<(15 - i));
			}
		}
	}

	obstical = (obstical << 1);

//*******************//How to react to obsticals//*********************

	if((obstical & (1<<3)) || (obstical & (1<<4)) || (obstical & (1<<5)))
	{
		rover_turn_left(speed);
	}
	else if((obstical & (1<<0)) || (obstical & (1<<1)) || (obstical & (1<<2)))
	{
		rover_turn_left_light(speed);
	}
	else if((obstical & (1<<9)) || (obstical & (1<<10)) || (obstical & (1<<11)))
	{
		rover_turn_right(speed);
	}
	else if((obstical & (1<<12)) || (obstical & (1<<13)) || (obstical & (1<<14)))
	{
		rover_turn_right_light(speed);
	}
	else if((obstical & (1<<6)) || (obstical & (1<<7)) || (obstical & (1<<8)))
	{
		rover_straight(BACKWARD, speed);
	}
	else if(obstical == 0)
	{
		rover_straight(FORWARD, speed);
	}
//****************************************************************
	
	return obstical;
}

unsigned short checkBattery(void)
{
	unsigned char i = 0;
	unsigned long battVolt = 0;

	for(i = 0; i < 100; i++)
	{
		battVolt += ((long)ADC_read(2) * 10000) / 1023;
	}
	battVolt /= 100;

	if((battVolt <= MINBATTVOLT) && (battVolt >= 5500))
	{
		rover_stop();
		OCR1B = 0;
		HC_goSleep();
		lcd_clrscr();
		
		while(1)
		{
			PORTC ^= (1<<PC7);
			_delay_ms(500);
		}
	}
	
	return battVolt;
}
//------------------------------MAIN------------------------------------
int main(void)
{
	//volatile unsigned short i = 0;
	char input[16];
	char output[16];
	unsigned short obsticalVar = 0;
	unsigned short batteryVolt = 0;
	

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
	rover_init();
	
	lcd_init(LCD_DISP_ON);  	
	PORTC |= (1<<PC7);

	USART_Init(103);
	
	HC_init(96, 1, 1);

	ADC_init(0x04);

	usound_init();
	//-------------------
	
	while(1)
	{
		batteryVolt = checkBattery();
		usrInput(PINB);

		//Setting the Speed
		if(USART_check_RX())
		{
			USART_Receive_STRING(input);
			//sscanf(input, "%d", &speed);
			speed = atoi(input);
		}

		if(rotary.right && (speed < 50))
		{
			speed++;
		}
		else if(rotary.left && (speed > 0))
		{
			speed--;
		}
		//-------------------


		if(scanOnOff)
		{
			//sense an react to obsticals in front of the rover
			obsticalVar = obstical();

			//outpur
			sprintf(output, BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY((obsticalVar >> 8)));
			USART_Transmit_STRING(output);
			lcd_gotoxy(0,0);
			lcd_puts(output);

			sprintf(output, BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(obsticalVar));
			USART_Transmit_STRING(output);
			USART_Transmit_STRING("\n\r");
			lcd_gotoxy(8,0);
			lcd_puts(output);
		}
		else
		{
			//read distance directly in front of the rover (only one sample)
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

			sprintf(output, "Speed: %02d       ", speed);
			lcd_gotoxy(0,0);
			lcd_puts(output);
		}

		sprintf(output, "%05dmV %03dcm", batteryVolt, sense[8]);
		//USART_Transmit_STRING(output);
		//USART_Transmit_STRING("\n\r");
		lcd_gotoxy(0,1);
		lcd_puts(output);
//------------------------------------------
	}//end of while
}//end of main
