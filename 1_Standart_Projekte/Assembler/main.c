#include <avr/io.h>

int main()
{
	DDRB = 0xFF;
	DDRD = 0x00;

	while(1)
	{
		PORTB = PIND;
	}
}

