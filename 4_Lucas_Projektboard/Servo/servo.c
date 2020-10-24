/*
	Used to controll a servo. Timer 1 is used.
*/

long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

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

//Set Puls between 1000us and 2000us 
void servo_setPuls(unsigned short puls)
{
	if((puls >= 1000) && (puls <= 2000))
		OCR1B = (puls * 2);
	else
		OCR1B = 0;
}

//Set degree between -45 and 45
void servo_setDegree(char degree)
{
	servo_setPuls(map(degree, -45, 45, 2000, 1000));
}


