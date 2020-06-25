#include <stdlib.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "lcd.h"   //" = Lib in akt Projektverzeichnis
#include <avr/interrupt.h>
#include <avr/eeprom.h>


#ifndef F_CPU
#define F_CPU    8000000
#endif

#define IRQS_PER_SECOND   1000 /* 500 µs */
#define IRQS_PER_10MS     (IRQS_PER_SECOND / 25)

#define START 1
#define LESEN_SCHREIBEN 2
#define SCHREIBEN 3
#define STATE_4 4


uint16_t x, highscore_zeit;
uint8_t test, test2, beep, beep_flag, entprell, beep_laenge, eingabe_pos;
uint8_t ms1, ms10, ms100, sec, min, state;
uint16_t highscore, verz;
uint16_t eehighscore EEMEM, toggle, toggle_alt;

uint8_t eehighscore_name[8] EEMEM;

uint8_t highscore_name[8];

char eingabe_buchstabe, highscore_enter_flag, highscore_enter_verzoegerung;

static volatile uint8_t timer_10ms;

void wait_10ms (const uint8_t);
void timer1_init();
char taste_oben(void);
char taste_mitte(void);
char taste_unten(void);

#if !defined (TCNT1H)
#error Dieser Controller hat keinen 16-Bit Timer1!
#endif // TCNT1H

void timer1_init()
{
    // Timer1: keine PWM
    TCCR1A = 0;

#if defined (CTC1) && !defined (WGM12)
   TCCR1B = (1 << CTC1)  | (1 << CS10);
#elif !defined (CTC1) && defined (WGM12)
   TCCR1B = (1 << WGM12) | (1 << CS10);
#else
#error Keine Ahnung, wie Timer1 fuer diesen AVR zu initialisieren ist!
#endif

    OCR1A = (unsigned short) ((unsigned long) F_CPU / IRQS_PER_SECOND-1);

#if defined (TIMSK1)
    TIMSK1 |= (1 << OCIE1A);
#elif defined (TIMSK)
    TIMSK  |= (1 << OCIE1A);
#else
#error Keine Ahnung, wie IRQs fuer diesen AVR zu initialisieren sind!
#endif
}


ISR (SIG_OUTPUT_COMPARE1A) //Timer so eingestellt, dass ISR jede ms aufgerufen wird
{
    static uint8_t interrupt_num_10ms;

  
    if (++interrupt_num_10ms == IRQS_PER_10MS)
    {
		interrupt_num_10ms = 0;
                
		if(beep>=1)
		{
			beep--;
			beep_flag = 1;
		}
		else beep_flag = 0;
				
		ms1++; 			// 1 ms
		if(ms1==10)		//10 ms
		{
			ms10++;
			ms1 = 0;
		
			
		}
		if(ms10==10);	//100 ms
		{
			ms100++;
			ms10 = 0;
				if(entprell >=1)
			{
				entprell--;
			}
		//	lcd_gotoxy(0,1);
		//		lcd_count(highscore_enter_verzoegerung);
			
		
		}
		if(ms100==10)	//1 sec
		{
			sec++;
			toggle++;
			ms100 = 0;
			
			if(highscore_enter_flag==1)
			{
				highscore_enter_verzoegerung++;
				
			}
		


/*	if(toggle !=toggle_alt)
	{
		
		lcd_gotoxy(0,1);
		toggle_alt=toggle;
		lcd_count(toggle);
	}
	*/	
			
		}
		if(sec==60)		//1 min
		{
			min=0;
			min++;
		}
    }
}

int main(void)
{
	x = 0;
	ms1=0;
	ms10=0;
	ms100=0;
	sec=0;
	min=0;
	beep=0;
	beep_flag = 0;
	beep_laenge = 10;
	toggle = 0;
	toggle_alt = 0;
	entprell=0;
	state = START;
	highscore = 737;
	test=0;
	highscore_zeit = 0;
	eingabe_pos = 0;
	verz=0;
	
	highscore_enter_flag=0;
	highscore_enter_verzoegerung = 0;
	
	eingabe_buchstabe = 'A';
	for(x=0; x<8; x++)
	{
		highscore_name[x] = ' ';
	}
	
	DDRD = 0xFF;			//Port D Ausgang 
	PORTD = 0x00;
	
	DDRB = 0X00;			//Port B Eingang
	PORTB = 0XFF;

	timer1_init();  		// Timer1 initialisieren
	sei();  				// Interrupts aktivieren
	
	lcd_init(LCD_DISP_ON);  // initialisieren
    lcd_clrscr();


	lcd_gotoxy(0,0);
	lcd_puts("hallo");
							
	while(1)
	{
		if(beep != 0)
		{
			PORTD = 0xff;
		}else
		{
			PORTD = 0x00;
		}	
	
		switch(state)
		{
			case START:	if((taste_oben() || taste_mitte() || taste_unten()) && (entprell == 0))
							{
								entprell=2;
								state = LESEN_SCHREIBEN;
								eingabe_pos = 0;
								lcd_clrscr();
								lcd_gotoxy(0,0);
								lcd_puts("schreiben");
								lcd_gotoxy(0,1);
								lcd_puts("lesen");
								lcd_gotoxy(0,0);
								
							}
							break;
						
			case LESEN_SCHREIBEN:	if(taste_unten() && (entprell == 0))
							{
								entprell=2;
								state = START;
								for(x=0; x<8; x++)
								{
									highscore_name[x] = ' ';
								}
								for(x=0; x<8; x++)
								{
									highscore_name[x] = eeprom_read_byte(&eehighscore_name[x]);
								}
								
								lcd_clrscr();
								for(x=0; x<8; x++)
								{
									lcd_gotoxy(x,0);
									lcd_putc(highscore_name[x]);
								}
								lcd_gotoxy(0,1);
								lcd_count(highscore);
							}
						
							if(taste_oben() && (entprell == 0))
							{
								entprell=2;
								state = SCHREIBEN;
								lcd_clrscr();
								lcd_gotoxy(1,0);
								lcd_count(highscore);
								lcd_gotoxy(0,0);
								lcd_puts("        ");
								lcd_gotoxy(0,0);
								lcd_putc(eingabe_buchstabe);
							}
							break;
												
			case SCHREIBEN:	if(taste_oben() && (entprell == 0))
							{
								entprell=2;
								eingabe_buchstabe++;
								//lcd_clrscr();
								lcd_gotoxy(eingabe_pos,0);
								lcd_putc(eingabe_buchstabe);
							
							}
											
							if(taste_unten() && (entprell == 0))
							{
								entprell=2;
								eingabe_buchstabe--;
								//lcd_clrscr();
							
								lcd_gotoxy(eingabe_pos,0);
								lcd_putc(eingabe_buchstabe);
						
							}
												
							if(taste_mitte())
							{
							
								if(highscore_enter_flag==0)
								{
										
										highscore_enter_flag=1;//erstes mal
										//entprell=2;
										
										highscore_name[eingabe_pos] = eingabe_buchstabe;
										eingabe_pos++;
										eingabe_buchstabe = 'A';
											
										
										//lcd_clrscr();
										//lcd_gotoxy(0,0);
										//lcd_puts("highscore");
										//lcd_gotoxy(0,1);
										//lcd_puts("saved");
								}
								
								if(highscore_enter_verzoegerung>1)
								{
									while(eingabe_pos<8)
									{
										highscore_name[eingabe_pos] = ' ';
										eingabe_pos++;
									}
									for(x=0; x<8; x++)
									{
										eeprom_write_byte(&eehighscore_name[x], highscore_name[x]);
									}
																		
									lcd_clrscr();
							
									lcd_gotoxy(0,0);
									lcd_puts("highscore");
									lcd_gotoxy(0,1);
									lcd_puts("saved");
									
									for(verz=0; verz<65000;verz++);
									
									highscore_enter_flag=0;
									highscore_enter_verzoegerung=0;
									
									state = START;
									break;
								}
							
							}else //wenn Taste nicht gedrueckt
							{
								highscore_enter_flag=0;
								highscore_enter_verzoegerung=0;
							}
							break;
			
		}

	}//end of while
	return 0;
}//end of main



char taste_oben(void)
{
	if(!(PINB & (1<<PINB0)))
	{
		return 1;
	}else return 0;
}	

char taste_mitte(void)
{
	if(!(PINB & (1<<PINB1)))
	{
		return 1;
	}else return 0;
	
}
	
char taste_unten(void)
{
	if(!(PINB & (1<<PINB2)))
	{
		return 1;
	}else return 0;
}
