/*********************************************************************************/
/*  Ultrasonic range finder HC-SR04 lib for AVR									 */
/*  Autor: Kornuta Taras														 */
/*  Date: 21.04.2016															 */
/*	URL: http://digiua.com														 */
/*********************************************************************************/


#ifndef USOUND_H
#define USOUND_H
#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>


// Ports initialization
#define US_DDR		DDRB
#define US_PORT		PORTB
#define US_PIN		PINB
#define US_TRIGER	PB0
#define US_ECHO		PB1


 void usound_init(void);				//Paste in Main function

 uint32_t readDistance(void);			//Return the distance in centimeters
										//Recommended use this function with 20Hz frequency 



#endif //USOUND_H
