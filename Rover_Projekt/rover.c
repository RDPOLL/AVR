#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>

//User defines
#define DIRRPIN	PB0
#define DIRLPIN PB1

#define FORWARD 1
#define BACKWARD 0
#define SPEEDRIGHT OCR0A
#define SPEEDLEFT OCR0B


//initialize for rover use. For more detail read datasheet of mC.
void initRover(void)
{
	DDRB |= (1<<DIRRPIN) | (1<<DIRLPIN) | (1<<PB3) | (1<<PB4);

	TCCR0A |= (1<<COM0A1) | (1<<COM0B1);
	TCCR0A |= (1<<WGM00);

	TCCR0B |= (1<<CS00);
}

//Use FORWARD and BACKWARD to controll direchtion. Speed on 0 to 100 scale
void moveRover(unsigned char dirLeft, unsigned char speedLeft, unsigned char dirRight, unsigned char speedRight)
{
	if(dirLeft){
		PORTB |= (1<<DIRLPIN);
		SPEEDLEFT = (255 - speedLeft);
	}
	else{
		PORTB &= ~(1<<DIRLPIN);
		SPEEDLEFT = speedLeft;
	}

	if(dirRight){
		PORTB |= (1<<DIRRPIN);
		SPEEDRIGHT = (255 - speedRight);
	}
	else{
		PORTB &= ~(1<<DIRLPIN);
		SPEEDRIGHT = speedRight;
	}
}
