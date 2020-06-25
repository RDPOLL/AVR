/*
 *		This Libary is used to Initialise and read the ADC
 * 		of the atmega1284p. For Register discription use the Datasheet
 * 		of the uP.
 * 
 * 		For 5V fullscale use this formula to calculate the coresponding voltage:
 * 		volt = ((long)adc_value * 10000) / 2046;
 * 		
 */

void ADC_init(uint8_t channel)
{
	/*At Higher Frequency, conversion is fast but it is not very accurate.
	So, we trade accuracy for speed */
	DIDR0  = channel;								//Disconnect Digital input from used Analog inputs
	ADCSRA |= ((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0));	//ADC Prescaler of 128 i.e 16Mhz/128 = 125Khz the ADC clock
	ADMUX  |= (0<<REFS0)|(1<<REFS1);				//We are using Vcc for reference i.e Vref = Vcc.
	ADCSRA |= (1<<ADEN)|(1<<ADSC);					//Turn on ADC and Do an initial conversion because this one is the slowest and to ensure that everything is up in running-Free running mode
}
 
uint16_t read_ADC(uint8_t channel)
{
	channel &= 0b00000111;
	ADMUX &= 0xF0;						//Clear the older channel that was read-i.e MUX0-3 cleared without effecting REFS0-1 ,ADLAR and the reserved bit.
	ADMUX |= channel;					//Defines the new ADC channel to be read
	ADCSRA |= (1<<ADSC);				//Starts a new conversion

	while(ADCSRA & (1<<ADSC));			//Wait until the conversion is done
	return ADCW;						//Returns the ADC converted value of the chosen channel
}
