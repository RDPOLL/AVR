/*	Dies ist eine Libary zum komunizieren durch den Seriellenschnittstellen
 *	des ATMEGA1284p.
 */

#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>

//USART RX max laenge
#define SLAENG 80

	//Datasheet s.197 for example baud
	//@16mHz: baud = 103 for: 9600
	//@16mHz: baud = 3 for: 256K
	//@16mHz: baud = 0 for: 1M
void USART_Init( unsigned int baud )
{
	/* Set baud rate */
	UBRR0 = (unsigned int)baud;

	/* 	Rx output, Tx input (USART0) */
	DDRD |= 0x02;
	
	/* Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	
	/* Set frame format: 8data, 2stop bit */
	UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}

unsigned char USART_check_RX(void)
{
	if(UCSR0A & (1<<RXC0))
		return 1;
	else
		return 0;
}

void USART_Transmit( char data )
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) );
	
	/* Put data into buffer, sends the data */
	UDR0 = data;
}

void USART_Transmit_STRING(char *data)
{
    while(*data != 0) USART_Transmit(*data++);
}

unsigned char USART_Receive(void)
{	
	/* Wait for data to be received */
	while( !(UCSR0A & (1<<RXC0)) );
	
	/* Get and return received data from buffer */
	return UDR0;
}

unsigned char USART_Receive_STRING(unsigned char *st)
{
	unsigned char c = 0;
	unsigned char digit = 0;

	while((digit < SLAENG) && ((c = USART_Receive()) != '\r'))
	{
		st[digit++] = c;		//add char to string
	}
	
	st[digit] = '\0';			//add NULL termination
	return (int)st;
}

void USART_Flush(void)
{
	unsigned char dummy;
	while(UCSR0A & (1<<RXC0)) dummy = UDR0;
}

//============= SPI =======================

void SPI_MasterInit(unsigned char lsbFist)
{
	/* Set MOSI, SCK and SS output, all others input */
	DDRB |= (1<<PB5)|(1<<PB7)|(1<<PB4);
	
	/* Enable SPI, Master, set clock rate fck/16 */
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
	
	/* if direction is set LSB first, else MSB first */
	if(lsbFist) SPCR |= (1<<DORD);
	else SPCR &= ~(1<<DORD);
}

void SPI_MasterTransmit(unsigned char cData, unsigned char doubleSpeed)
{
	/* Set Double Speed mode */
	if(doubleSpeed) SPSR |= (1<<SPI2X);
	else SPSR &= ~(1<<SPI2X);
	
	/* Start transmission */
	SPDR = cData;
	
	/* Wait for transmission complete */
	//for(i = 160; i > 0; i--);
	while(!(SPSR & (1<<SPIF)));
}

void SPI_MasterTransmit_string(unsigned char *data, unsigned char doubleSpeed)
{
    while(*data != 0) SPI_MasterTransmit(*data++, doubleSpeed);
}

void SPI_SlaveInit(void)
{
	/* Set MISO output, all others input */
	PORTB = (1<<PB6);
	
	/* Enable SPI */
	SPCR = (1<<SPE);
}
char SPI_SlaveReceive(void)
{
	/* Wait for reception complete */
	while(!(SPSR & (1<<SPIF)));
	
	/* Return Data Register */
	return SPDR;
}
