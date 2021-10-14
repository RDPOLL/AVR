/*
	Used to control a servo. Timer 1 is used.
*/

#define PULSOFFSET -15


void servo_init(void)
{
	//PWM settings
	TCCR1A |= (1<<COM1B1) | (1<<WGM11);
	TCCR1B |= (1<<WGM12) | (1<<WGM13);

	//PWM speed
	TCCR1B |= (1<<CS11);

	//set counter top
	ICR1 = 10000;
}

//Set Puls between 750us and 2250us 
void servo_setPuls(unsigned short puls)
{
	if((puls >= 750) && (puls <= 2250))
		OCR1B = ((puls + PULSOFFSET) * 2);
	else
		OCR1B = 0;
}

//Set degree between -45 and 45
void servo_setDegree(char degree)
{
	servo_setPuls((((long)degree - ((long)-45)) * ((long)1000 - (long)2000) / ((long)45 - ((long)-45)) + (long)2000));
}


