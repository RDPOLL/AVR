
#include <avr/eeprom.h>


#define START 1
#define LESEN_SCHREIBEN 2
#define SCHREIBEN 3
#define STATE_4 4


uint16_t x, highscore_zeit;
uint8_t eingabe_pos;
uint16_t highscore, verz;
uint8_t highscore_name[8];
char eingabe_buchstabe, highscore_enter_flag, highscore_enter_verzoegerung;

uint16_t eehighscore EEMEM;
uint8_t eehighscore_name[8] EEMEM;



char eingabe_buchstabe, highscore_enter_flag, highscore_enter_verzoegerung;

ISR (SIG_OUTPUT_COMPARE1A) //Timer so eingestellt, dass ISR jede ms aufgerufen wird
{
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
		}
		if(sec==60)		//1 min
		{
			min=0;
			min++;
		}
 }// End of ISR


int main(void)
{
	x = 0;
	
	highscore_zeit = 0;
	eingabe_pos = 0;
	verz=0;
	
	highscore_enter_flag=0;
	highscore_enter_verzoegerung = 0;
	
	eingabe_buchstabe = 'A';
	
	for(x=0; x<8; x++)	// Fuellt das Array mit Leerzeichen (8 Zeichen, sonst Wert anpassen)
	{
		highscore_name[x] = ' ';
	}
	
	

							
	while(1)
	{
		
	
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
						
			case LESEN_SCHREIBEN:	//"Lesen"  wurde ausgewählt
									if(taste_unten() && (entprell == 0))
									{
										entprell=2;
										state = START;
										
										for(x=0; x<8; x++)	//bestehende Zeichen in Array löschen
										{
											highscore_name[x] = ' ';
										}
										for(x=0; x<8; x++)	// Highscore aus EEPROM übernehmen
										{
											highscore_name[x] = eeprom_read_byte(&eehighscore_name[x]);
										}
								
										lcd_clrscr();
										for(x=0; x<8; x++)	// Highscore auf Display ausgeben
										{
											lcd_gotoxy(x,0);
											lcd_putc(highscore_name[x]);
										}
										lcd_gotoxy(0,1);
										lcd_count(highscore);
									}//End of Taste unten 
						
						
									//"Schreiben" wurde ausgewählt
									if(taste_oben() && (entprell == 0))
									{
										entprell=2;
										state = SCHREIBEN;
										lcd_clrscr();
										lcd_gotoxy(1,0);
										lcd_count(highscore);
										lcd_gotoxy(0,0);
										lcd_puts("        ");// Zeile für Namen löschen
										lcd_gotoxy(0,0);
										lcd_putc(eingabe_buchstabe);
									}//End of Taste oben
									break;
												
			case SCHREIBEN:	// Buchstabe erhöhen
							if(taste_oben() && (entprell == 0))
							{
								entprell=2;
								eingabe_buchstabe++;
								
								lcd_gotoxy(eingabe_pos,0);
								lcd_putc(eingabe_buchstabe);
							
							}
							
							
							// Buchstabe abzählen				
							if(taste_unten() && (entprell == 0))
							{
								entprell=2;
								eingabe_buchstabe--;
											
								lcd_gotoxy(eingabe_pos,0);
								lcd_putc(eingabe_buchstabe);
						
							}
												
							// Buchstaben akzeptieren wenn kurz gedrück
							// Eingabe Abschliessen durch langes drücken (> 2sec)
							if(taste_mitte())	//Achtung, nicht entprellen!!!!!!!!!
							{
								
								//Buchstabe übernehmen
								//Wird nur einmal durchlaufen, auch wenn Taste gedrückt bleibt
								if(highscore_enter_flag==0) 
								{
										highscore_enter_flag=1;//Flag setzen das Taste gedrückt 
										entprell=2; 
										
										highscore_name[eingabe_pos] = eingabe_buchstabe; //Buchstabe setzen
										eingabe_pos++;	//nächster Buchstabe
										eingabe_buchstabe = 'A'; //Anzeige beginnt wieder bei "A"
								}
								
								
								//Speichert Name in EEPROM
								//Speichern der Zeit hier noch zu implementieren
								// Wenn mittlere Taste > 2 sec gedrückt 
								if(highscore_enter_verzoegerung>1)
								{
									while(eingabe_pos<8)//Füllt restliche Positionen bis 8 mit Leerzeichen
									{
										highscore_name[eingabe_pos] = ' ';
										eingabe_pos++;
									}
									for(x=0; x<8; x++)	//Kopiert Name in EEPROM Array
									{
										eeprom_write_byte(&eehighscore_name[x], highscore_name[x]);
									}
																		
									lcd_clrscr();
								
									//Ausgabe dass Highscore gespeichert wurde
									lcd_gotoxy(0,0);
									lcd_puts("highscore");
									lcd_gotoxy(0,1);
									lcd_puts("saved");
									
									for(verz=0; verz<65000;verz++);
									
									highscore_enter_flag=0;
									highscore_enter_verzoegerung=0;
									
									state = START; //Retour in gewünschten State springen
									break;
								}
							
							}else //Durchlauf wenn Taste nicht gedrueckt
							{
								highscore_enter_flag=0;
								highscore_enter_verzoegerung=0;
							}
							break;
						
		
		}

			
	}//end of while
	return 0;
}//end of main

