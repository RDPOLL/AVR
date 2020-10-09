#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>

//User defines
#define DIRRPIN	PB5
#define DIRLPIN PB6

#define SPEEDRIGHT OCR0A
#define SPEEDRIGHTPIN PB3

#define SPEEDLEFT OCR0B
#define SPEEDLEFTPIN PB4
//------------------


#define FORWARD 0
#define BACKWARD 1


//initialize for rover use. For more detail read datasheet of uC.
void rover_init(void)
{
	//configure outputs
	DDRB |= (1<<DIRRPIN) | (1<<DIRLPIN) | (1<<SPEEDRIGHTPIN) | (1<<SPEEDLEFTPIN);

	//pwm settings
	TCCR0A |= (1<<COM0A1) | (1<<COM0B1) | (1<<WGM00);

	//Set pwm speed
	TCCR0B |= (1<<CS00);
}

//Use FORWARD and BACKWARD to controll direction.
void rover_move(unsigned char dirLeft, unsigned char speedLeft, unsigned char dirRight, unsigned char speedRight)
{
	//----PWM Control--------
	if(dirRight)
	{
		PORTB &= ~(1<<DIRRPIN);
		SPEEDRIGHT = (255 - speedRight);
	}
	else
	{
		PORTB |= (1<<DIRRPIN);
		SPEEDRIGHT = (255 - speedRight);
	}

	if(dirLeft)
	{
		PORTB &= ~(1<<DIRLPIN);
		SPEEDLEFT = (255 - speedLeft);
	}
	else
	{
		PORTB |= (1<<DIRLPIN);
		SPEEDLEFT = (255 - speedLeft);
	}
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
