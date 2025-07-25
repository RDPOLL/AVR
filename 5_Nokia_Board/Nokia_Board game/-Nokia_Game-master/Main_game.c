/*
 * Ersteller:	Lucas Micha Pollak
 * Datum:		6.9.2019
 * Ort:			Duebendorf
 * 
 * Beschreibung:
 * Diese Programm ist ein Spiel für das Nokiadisplay-Board. 
 * Es ist das erste grosse Programm für ein Mikrokontroller.
 * Als Vorbild für dieses Spiel hatte ich Space Invaders und Blocks.
 * Es geht darum alle Bloecke oben mit dem Ball zu treffen. 
 * Nachdem diese getroffen wurden verschwinden sie. Wenn alle Bloecke
 * getroffen wurden wird einem den Score angezeigt. Als Spieler kann 
 * man einen Balken am unterern Rand des Display steueren.
 * jenachdem wo der Ball auf dem Balken auftrifft springt er in einem 
 * anderen Winkel wieder ab. Falls der Ball unten aus dem Spiel faellt 
 * verliert man ein Leben. Nach drei Leben ist das Spiel vorbei.
*/


#include <avr/io.h>
#include "glcd/glcd.h"
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include "glcd/fonts/Liberation_Sans15x21_Numbers.h"
#include "glcd/fonts/font5x7.h"
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/delay.h>

#define F_CPU 16000000UL  // 1 MHz

#define RELOAD_TIME 30													//Schalter entprellungs zeit

#define TASTER_R (!(PIND &(1<<PD5)))
#define TASTER_M (!(PIND &(1<<PD2)))
#define TASTER_L (!(PIND &(1<<PD6)))

enum Status {STARTSCREEN, RUN, GAMEOVER_SCREEN, INIT, WIN, NAMEN};		//Stati fuer switch case definieren


unsigned char Blocks [1] [5] = {{1, 1, 1, 1, 1}};						//[LEVEL][STATUS DER BLOECKE]

unsigned char Blocks_korr [5] [4] = {{ 4, 3, 12, 4},{ 20, 3, 12, 4},{ 36, 3, 12, 4},{ 52, 3, 12, 4},{ 68, 3, 12, 4}};		//{x_achse, y_achse, laenge, hoehe}


unsigned char milisek = 0;												//Variabeln fuer Timer Deklarieren
unsigned char sek = 0;
unsigned char move_kreis = 0;
unsigned long gametime = 0;
unsigned char schalter_ent = 0;
volatile unsigned char Status = STARTSCREEN;							//Status fuer das switch case


volatile signed char Kreis_vertikal = 0;								//deklarieren und setzen der Vertikal bewegung des Kreises (0 = DOWN, 1 = UP)
volatile signed char Kreis_horizontal = 2;								//deklarieren und setzen der Horizontalen bewegung des Kreises (0 = RECHTS, 1 = LINKS, 2 = GERADE, 3 = RECHTS, 4 = LINKS)

//=============================================
//ISR wird hier ausgefuehrt und definiert

ISR (TIMER1_COMPA_vect)
{
	milisek ++;
	
	if(move_kreis != 0)move_kreis --;
	
	if(schalter_ent != 0)schalter_ent --;								//Schalter entprellung countdown
	
	if(Status == RUN) gametime ++;										//Gametime wird nur hochgezaelt wenn das Spiel gespielt wird

	if(milisek == 100)													//alle 100 milisekunden eine sekunde hoch stellen und milisekunden auf 0 setzen
		{
			sek ++;
			milisek = 0;
            
            if(sek == 60)												//alle 100 milisekunden eine sekunde hoch stellen und milisekunden auf 0 setzen
                {
                    sek = 0;
                }
		}
}

//=============================================
//Funktion zum Score berechnen

unsigned short score (unsigned long time, unsigned short auf, unsigned char heart)	
	{
		unsigned long score = 0;
		
		score = (10000 / (time + 100)) * (1000 / (auf + 10)) * heart;		//Algoritmus zum Score berechnen
		
		return score;
	}

//=============================================
//Diese Funktion ist dafuer zustaendig die Grenzen der Bloecke zu bestimmen.

void blocks (signed char ball_x, signed char ball_y)
	{
		for(unsigned char x = 0; x < 5; x ++)														//Fuenf durchlaeufe da fuenf bloecke gezeichnet werden
			{	
				if(Blocks [0] [x] == 1)																//Nur Bloecke verwenden welche noch nicht verschwunden sind
					{
						if(ball_x >= Blocks_korr [x] [0] && ball_x <= (Blocks_korr [x] [0] + Blocks_korr [x] [2]))					//Ist der Ball zwischen der Linken und der Rechten Kante der Xten Blockes
							{
								if(Kreis_vertikal == 1)																				//Bewegt sich der Ball nach oben
									{
										if(ball_y <= (Blocks_korr [x] [1] + Blocks_korr [x] [3]) && ball_y >= Blocks_korr [x] [1]) 	//Ist der Ball auf der Y-Achse zwischen der Unteren und der Oberen Kante des Xten Blockes
											{
												Kreis_vertikal = 0;
												Blocks [0] [x] = 0;
											}
									}
								else
									{
										if(ball_y >= Blocks_korr [x] [1] && ball_y <= (Blocks_korr [x] [1] + Blocks_korr [x] [3]))	//Falls der Ball sich nicht nach oben bewegt
											{
												Kreis_vertikal = 1;
												Blocks [0] [x] = 0;
											}
									}
							}
						else
							{
								if(ball_y >= Blocks_korr [x] [1] && ball_y <= (Blocks_korr [x] [1] + Blocks_korr [x] [3]))					//Ist der Ball zwischen dem oberen und der unteren Kante
									{
										if(Kreis_horizontal == 0)																			//bewegt sich der Ball nach rechts
											{
												if(ball_x >= Blocks_korr [x] [0] && ball_x <= (Blocks_korr [x] [0] + Blocks_korr [x] [2]))	//ist der Ball zwischen der linken und der rechten Kante
													{
														Kreis_horizontal = 1;
														Blocks [0] [x] = 0;
													}
											}
										else
											{
												if(ball_x <= (Blocks_korr [x] [0] + Blocks_korr [x] [2]) && ball_x >= Blocks_korr [x] [0])	//Falls der Ball sich nicht nach rechts bewegt
													{
														Kreis_horizontal = 0;
														Blocks [0] [x] = 0;
													}
											}
									}
							}
					}
			}
	}
	
//=============================================

int main(void)
{	

//=============================================	
//initialisierung aller Ein und Ausgaenge
//PBx fuer x Pin des Mikrokontrollers
	
	DDRB |= (1<<PB2);						//Display belechtung
	PORTB |= (1<<PB2);						
	
	DDRD &= ~(1<<PD5);						//Rechter Taster
	PORTD |= (1<<PD5);
	
	DDRD &= ~(1<<PD2);						//Mittlerer Taster
	PORTD |= (1<<PD2);
	
	DDRD &= ~(1<<PD6);						//Linker Taster
	PORTD |= (1<<PD6);
	
	DDRC |= (1<<PC3);						//Gruene SMD LED
	PORTC &= ~(1<<PC3);
	
//=============================================	

	//Timer 1 Configuration
	OCR1A = 1270;	//OCR1A = 0x3D08;==1sec
	
    TCCR1B |= (1 << WGM12);
    // Mode 4, CTC on OCR1A

    TIMSK1 |= (1 << OCIE1A);
    //Set interrupt on compare match

    TCCR1B |= (1 << CS11) | (1 << CS10);
    // set prescaler to 64 and start the timer

//=============================================	
	
	signed char Balken = 35;							//deklarieren und setzen der erst position des Balkens
	
	signed char Kreis_y = 10, Kreis_x = 42;				//deklarieren und setzen der ersten Kordinaten des Kreises

	signed char Balken_width = 15;						//deklarieren und setzen der breite des Balkens
	
	signed char Kreis_speed = 10;						//Kreis geschwindikeit (Hoehere Zahl bedeutet langsamer)

	signed char buffer[10];								//Array fuer anzeige auf dem Display
    
    signed char Gameover_count = 0;						//Variabel fuer Lebenszaehler (3 Leben)
	
    signed short aufprall = 0;							//aufprall counter
    
    signed char heart1 = 1;								//Variabeln fuer Herzensanzeige
    signed char heart2 = 1;
    signed char heart3 = 1;
    
    signed char curser_x = 0;
    signed char curser_y = 0;
    
//=============================================
   
    sei();										//enable global interupt
	
	glcd_init();								//vorbereiten und initialisieren der Displays
	
	glcd_clear();
	glcd_write();
	
	glcd_tiny_set_font(Font5x7,5,7,32,127);		//Definieren des Fonds des Testes
	glcd_clear_buffer();
        
//=============================================	
//Start der Endlosschleife

	while(1)										
	{ 
		
//=============================================	
//Start des Switch Case

		switch(Status)
			{
		
//=============================================	
//Setzen aller Variabeln auch nach Neustart			
				
				case INIT:
				
				Gameover_count = 0;					//Bewegungs Variabeln
				gametime = 0;
				Balken = 35;
				Kreis_x = 42;
				Kreis_y = 10;
				Kreis_horizontal = 2;
				Kreis_vertikal = 0;
				
				aufprall = 0;						//Aufprall auf null setzten
				
				for(unsigned char x = 0; x < 5; x ++)		//Blocks wieder Schwarz zeichen (damit sie erscheinen)
					{
						Blocks[0][x] = 1;
					}
				
				heart1 = 1;							//lebensanzeige auf voll stellen
				heart2 = 1;
				heart3 = 1;
			
				Status = RUN;
				
				break;

				case STARTSCREEN:
				
				glcd_clear_buffer();
				glcd_tiny_draw_string( 14, 2, "Press Blue" );
				glcd_tiny_draw_string( 17, 3, "for START" );
				glcd_write();
				
				if(TASTER_M && (schalter_ent == 0))							//Mittlerer Taster zum starten druecken
					{
						 schalter_ent = RELOAD_TIME;
						 glcd_clear_buffer();
						 glcd_tiny_draw_string( 11, 2, "LOADING..." );		//Loadingscreen ohne guten grund
						 glcd_write();
						 _delay_ms(1000);
						 
						 Status = INIT;
					 }
					 
				if(TASTER_L && TASTER_R)									//Easteregg impiziert
					{														//wenn Rechter und Linker Taster gedrueckt werden erscheint das Easteregg
						glcd_clear_buffer();
						glcd_tiny_draw_string( 16, 0, "EASTEREGG" );
						glcd_tiny_draw_string( 4, 5, "made by LUCAS" );
						glcd_write();
					}
				
				gametime = 0; 												//Gametime wird fuer score auf 0 gesetzt
				
				break;

//=============================================	
//Hauptteil des Spieles wird ausgefuehrt
	
				case RUN:
					
//=============================================
//Abfrage der Taster und bestimmen in welche Richtung sich der Balken bewegen muss			
		
		if(TASTER_R && (schalter_ent == 0))										//Rechter Taster wird gedrueckt
			{
				schalter_ent = 3;
				Balken = Balken + 1;											//der Balken wird um 1 Pixel nach Rechts verschoben
				if(Balken > 84 - Balken_width) Balken = 84 - Balken_width;		//Hier wird kontroliert das der Balken nicht aus dem Display wandert
			}
		
		if(TASTER_L && (schalter_ent == 0))										//Linker Taster wird gedrueckt
			{
				schalter_ent = 3;
				Balken = Balken - 1;											//der Balken wird um 1 Pixel nach Links verschoben
				if(Balken < 1) Balken = 1;										//Hier wird kontroliert das der Balken nicht aus dem Display wandert
			}
			
//=============================================
//Display Raender werden zu barieren gemacht. Ball springt zurueck 
		
		//Hier wird bestimmt das Wenn der Kreis sich auf dem Balken befindet, dass er weder hoch springt
		if((Kreis_vertikal == 0) && (Kreis_y >= 41) && (Kreis_x >= Balken) && (Kreis_x <=(Balken+Balken_width)))
			{
				aufprall ++;
				Kreis_vertikal = 1;
			}
		
		if((Kreis_vertikal == 1) && (Kreis_y <= 2)) Kreis_vertikal = 0;					//Obere Grenze
		
		if((Kreis_horizontal == 0) && (Kreis_x >= 82)) Kreis_horizontal = 1;			//Rechte Grenze
		
		if((Kreis_horizontal == 1) && (Kreis_x <= 2)) Kreis_horizontal = 0;				//Linke Grenze
		
		if((Kreis_horizontal == 3) && (Kreis_x >= 82)) Kreis_horizontal = 4;			//Rechte Grenze fuer flacher winkel
		
		if((Kreis_horizontal == 4) && (Kreis_x <= 2)) Kreis_horizontal = 3;				//Linke Grenze fuer flacher winkel

//=============================================
//Bloecke oben werden definiert (Hitbox)

		blocks (Kreis_x, Kreis_y);												//Funktion blocks aufrufen
	
		if((Blocks [0] [0]  == 0) && (Blocks [0] [1]  == 0) && (Blocks [0] [2]  == 0) && (Blocks [0] [3] == 0) && (Blocks [0] [4] == 0))Status = WIN;			//Falls alle Bloecke getroffen wurden (Farbe auf 0) wird der Status auf WIN gesetzt

//=============================================	
//Dieser Abschnitt definiert wohin der Kreis springt wenn er auf den Balken trifft
		
		if(Kreis_y >= 41)
			{
				if((Kreis_x >= Balken + 1) && (Kreis_x <=(Balken + 5))) Kreis_horizontal = 1; 					//Nach lintks springen (45 Grad)
				
				if((Kreis_x >= Balken+10) && (Kreis_x <= (Balken + Balken_width - 1))) Kreis_horizontal = 0;	//Nach rechts springen (45 Grad)
				
				if((Kreis_x >= Balken+5) && (Kreis_x <= (Balken + 10))) Kreis_horizontal = 2;					//Wenn der Kreis in die mitte des Balkens geraeht, fliegt er wieder gerade nach oben
				
				if(Kreis_x == Balken) Kreis_horizontal = 4;														//Wenn an kante des Balkens getroffen wird spickt der Kreis sehr flach in beide richtungen
				
				if(Kreis_x == Balken + Balken_width) Kreis_horizontal = 3;
			}

		if(Kreis_y >= 48)														//wenn Kreis unten heraus faehlt wieder in die Mitte Teleportieren
			{
				 Gameover_count ++;												//Leben werden hochgezaehlt
				 Kreis_x = 42;
				 Kreis_y = 10;
				 Kreis_horizontal = 2;
			}
			
		if(Gameover_count == 3) Status = GAMEOVER_SCREEN;						//Nach 3 Leben wird zum status gameover gewechsel

//=============================================		
//Herzenanzeige fuer lebensanzahl
//Herzen am rechten Rand werden je nach Gameover_count gesetzt

		switch (Gameover_count)
			{
				case 1:															
				heart1 = 0;
				break;
				
				case 2:
				heart1 = 0;
				heart2 = 0;
				break;
				
				case 3:
				heart1 = 0;
				heart2 = 0;
				heart3 = 0;
				break;
			}
		
//=============================================
// Hier werden die Bewegungen des Kreises von Variabeln in wirkliche Bewegungen umgesetzt.
		
		if(move_kreis == 0)
			{
				move_kreis = Kreis_speed;
				
				if(Kreis_vertikal == 0) Kreis_y = Kreis_y + 2;					//Vertikale bewegung(0 = DOWN, 1 = UP)
				if(Kreis_vertikal == 1) Kreis_y = Kreis_y - 2;
				
				if(Kreis_horizontal == 0) Kreis_x ++;							//Horizontalen bewegung(0 = RECHTS, 1 = LINKS, 2 = GERADE)
				if(Kreis_horizontal == 1) Kreis_x --;
				if(Kreis_horizontal == 3) Kreis_x = Kreis_x + 4;				//Starke Horizontale bewegung(3 = RECHTS, 4 = LINKS)
				if(Kreis_horizontal == 4) Kreis_x = Kreis_x - 4;
			}

//=============================================
//Das Diaplay wir hier zuert "geloescht" und danach wieder vollstaendig aufgezeichet.

		glcd_clear_buffer();
		
        glcd_fill_rect(Balken, 43, 15, 3, 1);									//Balken zeichnen
		
		for(unsigned char x = 0; x < 5; x ++)
			{
				glcd_draw_rect(Blocks_korr [x] [0], Blocks_korr [x] [1], Blocks_korr [x] [2], Blocks_korr [x] [3], Blocks [0] [x] );		//erste Reihe der Bloecke von Links nach Rechts
			}
		
		glcd_fill_circle(83, 18, 2, heart1);
		glcd_fill_circle(83, 24, 2, heart2);
		glcd_fill_circle(83, 30, 2, heart3);
		
		glcd_fill_circle(Kreis_x, Kreis_y, 2, 1);								//Kreis zeichnen

		glcd_write();

		break;
		
//=============================================			
//Gameover wird auf dem Display angezeigt

			case GAMEOVER_SCREEN:
			
			glcd_clear_buffer();
			glcd_tiny_draw_string( 18, 0, "GAMEOVER" );
			glcd_tiny_draw_string( 14, 2, "Press Blue" );
			glcd_tiny_draw_string( 11, 3, "for restart" );
			glcd_write();
			
			if(TASTER_M && (schalter_ent == 0))							//Wenn mittlerer Taster gedrueckt wird startet das Spiel neu
				{
					schalter_ent = RELOAD_TIME;
					Status = INIT;
				}
			
			break;
			
//=============================================

			case WIN:
			
			glcd_clear_buffer();
			glcd_tiny_draw_string( 32, 0, "WIN" );
			glcd_tiny_draw_string( 15, 1, "Score:" );
			glcd_tiny_draw_string( 14, 2, "Press Blue" );
			glcd_tiny_draw_string( 11, 3, "for restart" );

			
			sprintf(buffer, " %d", score(gametime, aufprall, 3 - Gameover_count));		//gametime wird auf dem display wiedergegeben
			glcd_tiny_draw_string(47, 1, buffer);
			
			glcd_write();

			if(TASTER_M && (schalter_ent == 0))							//Wenn mittlerer Taster gedrueckt wird startet das Spiel neu
				{
					schalter_ent = RELOAD_TIME;
					Status = INIT;
				}
				
			break;
		}
	}
}
