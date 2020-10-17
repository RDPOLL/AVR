#include "usound.h"
#include <util/delay.h>

void usound_init(void)
{
	US_DDR |= (1 << US_TRIGER)|(0 << US_ECHO);
	US_PIN |= (1 << US_TRIGER);
}

uint32_t readDistance(void)
{
	uint8_t readStatus=1;
	uint32_t disTime=0;

	US_PORT = (1 << US_TRIGER);
	_delay_us(10);
	US_PORT = (0 << US_TRIGER);

	
	while(readStatus)
	{
		while((US_PIN & (1 << US_ECHO)) && ((disTime / 55) < 500))
		{
			disTime++;
			readStatus = 0;
		}
		if((disTime / 55) == 500)
		{
			disTime = 0;
		}
	}	

	return disTime /= 55;	//return data in cm	(16Mhz)
}


