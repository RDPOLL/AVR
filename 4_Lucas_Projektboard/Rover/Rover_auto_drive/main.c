#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include "adc.c"
#include "lcd.h"
#include "rotary.c"
#include "rover.c"
#include "serial.c"
#include "hc12.c"
#include "usound.c"
#include "servo.c"

//==============WDTIMER===============
uint8_t mcusr_mirror __attribute__ ((section (".noinit")));

    void get_mcusr(void) \
      __attribute__((naked)) \
      __attribute__((section(".init3")));
    void get_mcusr(void)
    {
      mcusr_mirror = MCUSR;
      MCUSR = 0;
      wdt_disable();
    }
//=====================================

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
#define SCANDELAY 40
#define MINBATTVOLT 7400 //In mV
#define LIGHTTRESHOLD 500
#define AUTODRIVESTARTSPEED 6

unsigned char scanDir = 0;
unsigned char scanOnOff = 0;
unsigned short sense[17];
unsigned char speed = 0;
char output[16];


//Rover var
volatile unsigned short extInt0Cntr = 0;
volatile unsigned short extInt1Cntr = 0;

volatile unsigned char roverSetSpeedR = 0;
volatile unsigned char roverSetSpeedL = 0;

volatile unsigned char roverVarSpeedR = 0;
volatile unsigned char roverVarSpeedL = 0;

volatile unsigned char roverDirR = 0;
volatile unsigned char roverDirL = 0;


ISR(TIMER2_OVF_vect)
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
	//disable INT2
	EIMSK &= ~(1<<INT2);
	
	scanOnOff ^= 1;

	if(scanOnOff && (speed == 0))
	{
	//if speed wasnt set to another value
		speed = AUTODRIVESTARTSPEED;
	}
	else if(!scanOnOff)
	{
		speed = 0;
	}

	_delay_ms(100);

	//enable INT2
	EIMSK |= (1<<INT2);
}

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

unsigned short obstical(void)
{
	unsigned short dist = 0;
	unsigned short obstical = 0;
	unsigned short i = 0;
	unsigned char maxDist = 0;


	//scanning
	if(scanDir)
	{
		for(i = 0; i < 17; i++)
		{
			servo_setPuls(map(i, 0, 17, 2000, 1000));

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
			servo_setPuls(map((i-1), 0, 17, 2000, 1000));

			_delay_ms(SCANDELAY);

			dist = readDistance();

			sense[(i-1)] = dist;
		}
		scanDir = 1;
	}
//=================================================
//filtering for not seeing near parallel wall

for(i = 1; i < 16; i++)
{
	if(((sense[i-1] + 50) < sense[i]) && ((sense[i+1] + 50) < sense[i]))
	{
		sense[i] = ((sense[i-1] + sense[i+1]) / 2);
	}
}

//=================================================
//calculate an obstical

	for(i = 1; i < 16; i++)
	{
		if(sense[i] <= MINDIST)
		{
			if(((sense[i-1] <= (sense[i] + 10)) && (sense[i-1] >= (sense[i] - 10))) && ((sense[i+1] <= (sense[i] + 10)) && (sense[i+1] >= (sense[i] - 10))))
			{
				obstical |= (1<<(15 - i));
			}
		}
	}

	//scaning the outher most sense[] for an obsitcal
	if((sense[0] <= MINDIST) && ((sense[1] <= (sense[0] + 10)) && (sense[1] >= (sense[0] - 10))))
		obstical |= (1<<1);
		
	if((sense[16] <= MINDIST)  && ((sense[15] <= (sense[16] + 10)) && (sense[15] >= (sense[16] - 10))))
		obstical |= (1<<15);

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
		//scanning
		for(i = 0; i <= 16; i++)
		{
			if(sense[i] >= sense[maxDist])
			{
				maxDist = i;
			}
		}

		if((maxDist >= 6) && (maxDist <= 10))
		{
			rover_straight(FORWARD, speed);
		}
		else if(maxDist <= 5)
		{
			rover_move(FORWARD, (speed/2), FORWARD, speed);
		}
		else if(maxDist >= 11)
		{
			rover_move(FORWARD, speed, FORWARD, (speed/2));
		}
	}
//****************************************************************
//output uart

for(i = 0; i < 17; i++)
{
	sprintf(output, "%d:%03d ", i, sense[i]);
	USART_Transmit_STRING(output);
}
USART_Transmit_STRING("\r");
	
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
		PORTD &= ~(1<<PD6);
		HC_goSleep();
		lcd_clrscr();
		

		for(i = 0; i < 10; i++)
		{
			wdt_reset();
			PORTC |= (1<<PC7);
			_delay_ms(100);
			PORTC &= ~(1<<PC7);
			_delay_ms(900);
		}
		
		wdt_disable();

		lcd_gotoxy(1,0);
		lcd_puts("Charge Battery");
		lcd_gotoxy(2,1);
		lcd_puts("Immediately");

		//lcd_command(LCD_DISP_OFF);

		MCUCR |= (1<<BODS);
		PRR1 = 0x01;
		PRR0 = 0xFF;
		set_sleep_mode(SLEEP_MODE_PWR_DOWN);
		sleep_mode();
	}
	
	return battVolt;
}

unsigned short headLights(void)
{
	unsigned short lightSens = ADC_read(3);

	if((lightSens <= LIGHTTRESHOLD) && (!(PORTD & (1<<PD6))))
	{
		PORTD |= (1<<PD6);
	}
	else if(lightSens >= (LIGHTTRESHOLD + 25))
	{
		PORTD &= ~(1<<PD6);
	}

	return lightSens;
}

//------------------------------MAIN------------------------------------
int main(void)
{
	volatile unsigned char i = 0;
	char input[16];
	unsigned short obsticalVar = 0;
	unsigned short batteryVolt = 0;
	

	DDRA = 0x00;
	//DDRB = 0xff;
	DDRC = 0xff;			//LCD
	DDRD |= (1<<PD4) | (1<<PD6);

	//Pull down
	PORTB = 0x07;

	//ext interupts settings
	EICRA |= (1<<ISC21);
	EIMSK |= (1<<INT2);
	
	//Lib Initialisieren
	rover_init();
	
	lcd_init(LCD_DISP_ON);  	
	PORTC |= (1<<PC7);

	USART_Init(103);
	
	HC_init(96, 1, 8);

	ADC_init(0x0C);

	servo_init();

	usound_init();

	wdt_enable(WDTO_2S);
	//-------------------
	
	while(1)
	{
		wdt_reset();
		
		batteryVolt = checkBattery();
		usrInput(PINB);

		headLights();
		
		//Setting the Speed
		if(USART_check_RX())
		{
			USART_Receive_STRING(input);
			//sscanf(input, "%d", &speed);
			speed = atoi(input);
		}

		if(rotary.right && (speed < 30))
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
			//sense and react to obsticals in front of the rover
			obsticalVar = obstical();
			
			sprintf(output, BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY((obsticalVar >> 8)));
			//USART_Transmit_STRING(output);
			lcd_gotoxy(0,0);
			lcd_puts(output);

			sprintf(output, BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(obsticalVar));
			//USART_Transmit_STRING(output);
			//USART_Transmit_STRING("\n\r");
			lcd_gotoxy(8,0);
			lcd_puts(output);
		}
		else
		{
			//read distance directly in front of the rover (only one sample)
			servo_setDegree(0);
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
