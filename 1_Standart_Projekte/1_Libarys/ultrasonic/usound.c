#include "usound.h"

void usound_init(void)
{
	US_DDR |= (1 << US_TRIGER)|(0 << US_ECHO);
	US_PIN |= (1<<US_ECHO);
}

uint32_t readDistance(void)
{
	uint8_t readStatus=1;
	uint32_t disTime=0;

	PORTD |= (1 << PD2);
	PORTD |= (0 << PD2);

	
	while(readStatus)
	{
		while(PIND & (1 << PD4))
		{
			disTime++;
			readStatus = 0;
		}
	}	

	return disTime * 0.038;	//return data in cm	
}


