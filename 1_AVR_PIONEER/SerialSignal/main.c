#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include "serial.c"
#include "rotary.c"

#define SPEED 0
#define TIMERDELAY 14

#define HB_PIN 0
#define VA_PIN 1
#define DATAB_PIN 2
#define DATAA_PIN 3
#define HT_PIN 1

#define HB_PIN_N 4
#define VA_PIN_N 5
#define DATAB_PIN_N 6
#define DATAA_PIN_N 7
#define HT_PIN_N 2

#define M2 0
#define M1 1



enum{MENU, SENDING, ADDRESS, MEMORY_A, PHASE_A, MEMORY_B, PHASE_B};


volatile unsigned char serialEnable = 0;

volatile unsigned short timerDelay = 0;

volatile unsigned long clkCntr = 0;
volatile unsigned short dataA = 0;
volatile unsigned short dataB = 0;
volatile unsigned char HTMemorySelect = M1;



ISR(TIMER0_COMPA_vect)
{		
	if(serialEnable && (clkCntr <= 31))
	{	
		//VA Signal
		((clkCntr >= 0) && (clkCntr <= 2)) ? (PORTB |= (1<<VA_PIN)) : (PORTB &= ~(1<<VA_PIN));
		((clkCntr >= 0) && (clkCntr <= 2)) ? (PORTB &= ~(1<<VA_PIN_N)) : (PORTB |= (1<<VA_PIN_N));

		//DATA B Signal
		((clkCntr >= 14) && (clkCntr <= 15)) ? (PORTB &= ~(1<<DATAB_PIN)) : (((dataB>>(13-clkCntr)) & 0x01) ? (PORTB |= (1<<DATAB_PIN)) : (PORTB &= ~(1<<DATAB_PIN)));
		((clkCntr >= 14) && (clkCntr <= 15)) ? (PORTB |= (1<<DATAB_PIN_N)) : (((dataB>>(13-clkCntr)) & 0x01) ? (PORTB &= ~(1<<DATAB_PIN_N)) : (PORTB |= (1<<DATAB_PIN_N)));

		//DATA A Signal
		(clkCntr >= 15) ? (PORTB &= ~(1<<DATAA_PIN)) : (((dataA>>(13-clkCntr+1)) & 0x01) ? (PORTB |= (1<<DATAA_PIN)) : (PORTB &= ~(1<<DATAA_PIN)));
		(clkCntr >= 15) ? (PORTB |= (1<<DATAA_PIN_N)) : (((dataA>>(13-clkCntr+1)) & 0x01) ? (PORTB &= ~(1<<DATAA_PIN_N)) : (PORTB |= (1<<DATAA_PIN_N)));

		_delay_us(TIMERDELAY);

		//Clock signal
		((clkCntr % 2) && (clkCntr <= 25)) ? (PORTB &= ~(1<<HB_PIN)) : (PORTB |= (1<<HB_PIN));
		((clkCntr % 2) && (clkCntr <= 25)) ? (PORTB |= (1<<HB_PIN_N)) : (PORTB &= ~(1<<HB_PIN_N));
		
		//HT Signal
		((clkCntr == 28) || ((clkCntr == 30) && (HTMemorySelect == M1))) ? (PORTC |= (1<<HT_PIN)) : (PORTC &= ~(1<<HT_PIN));
		((clkCntr == 28) || ((clkCntr == 30) && (HTMemorySelect == M1))) ? (PORTC &= ~(1<<HT_PIN_N)) : (PORTC |= (1<<HT_PIN_N));
		
		clkCntr++;
	}
	else
	{
		//Reset for next transmission
		PORTB = 0xf0;
		clkCntr = 0;
		serialEnable = 0;
	}
}


void serialSend(unsigned char row, unsigned char memorySelect, unsigned char memoryA, unsigned char phaseA, unsigned char memoryB, unsigned char phaseB)
{
	unsigned char i = 0;
	unsigned short dataTempA = 0;
	unsigned short dataTempB = 0;
	unsigned char parityTest = 0;
	
	//Parse information into dataTemp A
	dataTempA |= ((phaseA&0x1f)<<1);
	dataTempA &= ~(1<<6);
	dataTempA |= ((memoryA&0x01)<<7);
	dataTempA |= ((row&0x3f)<<8);
	
	//Parse information into dataTemp B
	dataTempB |= ((phaseB&0x1f)<<1);
	dataTempB &= ~(1<<6);
	dataTempB |= ((memoryB&0x01)<<7);
	dataTempB |= ((row&0x3f)<<8);
	
	//Calculate the Paritiybit A
	parityTest = 0;
	for(i = 0; i <= 13; i++)
	{
		if((dataTempA>>(i+1)) & 0x01) parityTest++;
	}
	if(parityTest % 2) dataTempA |= (1<<0);
		
	dataA = dataTempA;
	
	//Calculate the Paritiybit B
	parityTest = 0;
	for(i = 0; i <= 13; i++)
	{
		if((dataTempB>>(i+1)) & 0x01) parityTest++;
	}
	if(parityTest % 2) dataTempB |= (1<<0);
		
	dataB = dataTempB;
	
	HTMemorySelect = memorySelect;
		
	//Enable serial output
	serialEnable = 1;
}

//------------------------------MAIN------------------------------------
int main(void)
{	
	DDRD = 0x02;			//Schalterport
	DDRB = 0xff;			//LEDsport
	DDRC = 0xff;			//LCDdaten
	DDRA = 0xf0;			//LCDctrl & ADC
	
	char output[16];
	
	unsigned char inAddress = 0;
	unsigned char inHTMemory = M2;
	unsigned char inMemoryA = 0;
	unsigned char inPhaseA = 0;
	unsigned char inMemoryB = 0;
	unsigned char inPhaseB = 0;
	
	unsigned char screenSelection = MENU;
	unsigned char subMenuSelect = ADDRESS;
	
	
//------Timer Interupt setup----------------

	TCCR0A |= (1<<WGM01);		//Setting Timer mode
	TCCR0B |= ((1<<CS00) | (1<<CS02));		//Setting clk prescaler
	
	TIMSK0 |= (1<<OCIE0A);		//Enabeling Timer A match interupt
	
	OCR0A = SPEED;				//Timer top value
	
	TCNT0 = 0;					//Setting 8bit counter to 0
	
//-------------------------------------------
	
	sei();						//enable global interupts

	lcd_init(LCD_DISP_ON);  	//Initialisieren
	
	while(1)
	{
		usrInput(PIND);
				
		switch(screenSelection)
		{
			case MENU:
				switch(subMenuSelect)
				{			
					case SENDING:
						lcd_gotoxy(0,0);
						lcd_puts("Transmit    ");
						
						if(rotary.press) screenSelection = SENDING;
						else if(rotary.left && (subMenuSelect > 1)) subMenuSelect--;
						else if(rotary.right && (subMenuSelect < 6)) subMenuSelect++;
					break;
							
					case ADDRESS:
						lcd_gotoxy(0,0);
						lcd_puts("Address   ");
						
						lcd_gotoxy(0,1);
						sprintf(output, "%03d  ", inAddress);
						lcd_puts(output);
						
						if(rotary.press) screenSelection = ADDRESS;
						else if(rotary.left && (subMenuSelect > 1)) subMenuSelect--;
						else if(rotary.right && (subMenuSelect < 6)) subMenuSelect++;
					break;
					
					case MEMORY_A:
						lcd_gotoxy(0,0);
						lcd_puts("Memory A   ");
						
						lcd_gotoxy(0,1);
						if(inMemoryA)
							sprintf(output, "M1   ");
						else
							sprintf(output, "M2   ");
						lcd_puts(output);
						
						if(rotary.press) screenSelection = MEMORY_A;
						else if(rotary.left && (subMenuSelect > 1)) subMenuSelect--;
						else if(rotary.right && (subMenuSelect < 6)) subMenuSelect++;
					break;
					
					case PHASE_A:
						lcd_gotoxy(0,0);
						lcd_puts("Phase A   ");
						
						lcd_gotoxy(0,1);
						sprintf(output, "%02d     ", inPhaseA);
						lcd_puts(output);
						
						if(rotary.press) screenSelection = PHASE_A;
						else if(rotary.left && (subMenuSelect > 1)) subMenuSelect--;
						else if(rotary.right && (subMenuSelect < 6)) subMenuSelect++;
					break;
					
					case MEMORY_B:
						lcd_gotoxy(0,0);
						lcd_puts("Memory B   ");
						
						lcd_gotoxy(0,1);
						if(inMemoryB)
							sprintf(output, "M1   ");
						else
							sprintf(output, "M2   ");
						lcd_puts(output);
						
						if(rotary.press) screenSelection = MEMORY_B;
						else if(rotary.left && (subMenuSelect > 1)) subMenuSelect--;
						else if(rotary.right && (subMenuSelect < 6)) subMenuSelect++;
					break;
					
					case PHASE_B:
						lcd_gotoxy(0,0);
						lcd_puts("Phase B   ");
						
						lcd_gotoxy(0,1);
						sprintf(output, "%02d     ", inPhaseB);
						lcd_puts(output);
						
						if(rotary.press) screenSelection = PHASE_B;
						else if(rotary.left && (subMenuSelect > 1)) subMenuSelect--;
						else if(rotary.right && (subMenuSelect < 6)) subMenuSelect++;
					break;
				}
			break;
			case SENDING:
				if(rotary.press) screenSelection = MENU;
				else if(rotary.left && (inHTMemory > 0)) inHTMemory--;
				else if(rotary.right && (inHTMemory < 1)) inHTMemory++;
				
				lcd_gotoxy(0,0);
				lcd_puts("Transmitting   ");
				
				lcd_gotoxy(0,1);
				if(inHTMemory)
					sprintf(output, "M1   ");
				else
					sprintf(output, "M2   ");
				lcd_puts(output);
				
				serialSend(inAddress, inHTMemory, inMemoryA, inPhaseA, inMemoryB, inPhaseB);
				_delay_ms(10);
			break;
			
			case ADDRESS:
				if(rotary.press) screenSelection = MENU;
				else if(rotary.left && (inAddress > 0)) inAddress--;
				else if(rotary.right && (inAddress < 0x3f)) inAddress++;
				
				lcd_gotoxy(0,0);
				lcd_puts("Address:   ");
				
				lcd_gotoxy(0,1);
				sprintf(output, "%03d", inAddress);
				lcd_puts(output);
			break;
			
			case MEMORY_A:
				if(rotary.press) screenSelection = MENU;
				else if(rotary.left && (inMemoryA > 0)) inMemoryA--;
				else if(rotary.right && (inMemoryA < 1)) inMemoryA++;
				
				lcd_gotoxy(0,0);
				lcd_puts("Memory A:   ");
				
				lcd_gotoxy(0,1);
				if(inMemoryA)
					sprintf(output, "M1   ");
				else
					sprintf(output, "M2   ");
				lcd_puts(output);
			break;
			
			case PHASE_A:
				if(rotary.press) screenSelection = MENU;
				else if(rotary.left && (inPhaseA > 0)) inPhaseA--;
				else if(rotary.right && (inPhaseA < 0x1f)) inPhaseA++;
				
				lcd_gotoxy(0,0);
				lcd_puts("Phase A:   ");
				
				lcd_gotoxy(0,1);
				sprintf(output, "%02d", inPhaseA);
				lcd_puts(output);
			break;
			
			case MEMORY_B:
				if(rotary.press) screenSelection = MENU;
				else if(rotary.left && (inMemoryB > 0)) inMemoryB--;
				else if(rotary.right && (inMemoryB < 1)) inMemoryB++;
				
				lcd_gotoxy(0,0);
				lcd_puts("Memory B:   ");
				lcd_gotoxy(0,1);
				
				if(inMemoryB)
					sprintf(output, "M1   ");
				else
					sprintf(output, "M2   ");
				lcd_puts(output);
			break;
			
			case PHASE_B:
				if(rotary.press) screenSelection = MENU;
				else if(rotary.left && (inPhaseB > 0)) inPhaseB--;
				else if(rotary.right && (inPhaseB < 0x1f)) inPhaseB++;
				
				lcd_gotoxy(0,0);
				lcd_puts("Phase B:   ");
				
				lcd_gotoxy(0,1);
				sprintf(output, "%02d", inPhaseB);
				lcd_puts(output);
			break;
		}
		
		if(rotary.press | rotary.left | rotary.right)
		{
			lcd_clrscr();
		}
//------------------------------------------
	}//end of while
}//end of main
