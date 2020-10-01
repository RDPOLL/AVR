#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>

//To use this Libary you need to copy this section to your Main.c document.
/*

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
		SPEEDRIGHT = roverVarSpeedR;
	}

	if(roverDirL)
	{
		PORTB |= (1<<DIRLPIN);
		SPEEDLEFT = (255 - roverVarSpeedL);
	}
	else
	{
		PORTB &= ~(1<<DIRLPIN);
		SPEEDLEFT = roverVarSpeedL;
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
*/

//User defines
#define DIRRPIN	PB5
#define DIRLPIN PB6

#define SPEEDRIGHT OCR0A
#define SPEEDRIGHTPIN PB3

#define SPEEDLEFT OCR0B
#define SPEEDLEFTPIN PB4
//------------------


#define FORWARD 1
#define BACKWARD 0


//initialize for rover use. For more detail read datasheet of mC.
void rover_init(void)
{
	//configure outputs
	DDRB |= (1<<DIRRPIN) | (1<<DIRLPIN) | (1<<SPEEDRIGHTPIN) | (1<<SPEEDLEFTPIN);

	//pwm settings
	TCCR0A |= (1<<COM0A1) | (1<<COM0B1);
	TCCR0A |= (1<<WGM00);

	//Set pwm speed
	TCCR0B |= (1<<CS00);

	//enable overflow interupt
	sei();
	//TIMSK0 |= (1<<TOIE0);

	//ext interupts settings
	EICRA |= (1<<ISC00) | (1<<ISC10);
	EIMSK |= (1<<INT0) | (1<<INT1);
}

//Use FORWARD and BACKWARD to controll direction.
void rover_move(unsigned char dirLeft, unsigned char speedLeft, unsigned char dirRight, unsigned char speedRight)
{
	extern unsigned char roverSetSpeedR;
	extern unsigned char roverSetSpeedL;

	extern unsigned char roverDirR;
	extern unsigned char roverDirL;
	
	roverSetSpeedL = speedLeft;
	roverSetSpeedR = speedRight;

	roverDirL = dirLeft;
	roverDirR = dirRight;
}

void rover_straight(unsigned char dirRover, unsigned char speedRover)
{
	rover_move(dirRover, speedRover, dirRover, speedRover);
}

void rover_stop(void)
{
	rover_move(FORWARD, 0, FORWARD, 0);
}

void rover_turn_left(unsigned char speedRoverL)
{
	rover_move(BACKWARD, speedRoverL, FORWARD, speedRoverL);
}

void rover_turn_right(unsigned char speedRoverR)
{
	rover_move(FORWARD, speedRoverR, BACKWARD, speedRoverR);
}
