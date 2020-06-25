// Titel:       Reaktionstester
// Datei:       reaktionstester.c
// Funktion:
// Urheber:		Erasmo Colacicco
// Datum:       11.11.2011
/*********************************************************************************************************************************************************************************************************/

/******************************************************** LAUFZEITBIBLIOTHEKEN ********************************************************************/


#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdlib.h>
#include <avr/pgmspace.h>
#include <C:\Users\Gast\Desktop\Lehrlinge\Erasmo Colacicco\uC Kurs 2\Versuch\Robin\lcd.h>

// Für alte avr-gcc Versionen
#ifndef SIGNAL
#include <avr/signal.h>
#endif // SIGNAL

// Geblinkt wird PortB.1 (push-pull)
// Eine LED in Reihe mit einem Vorwiderstand zwischen
// PortB.1 und GND anschliessen.
#define PAD_LED  1
#define PORT_LED PORTD
#define DDR_LED  DDRD

// Der MCU-Takt. Wird gebraucht, um Timer1 mit den richtigen
// Werten zu initialisieren. Voreinstellung ist 1MHz.
// (Werkseinstellung für AVRs mit internem Oszillator).
// Das Define wird nur gemacht, wenn F_CPU noch nicht definiert wurde.
// F_CPU kann man so auch per Kommandozeile definieren, z.B. für 8MHz:
// avr-gcc ... -DF_CPU=8000000
//
// ! Der Wert von F_CPU hat rein informativen Character für
// ! die korrekte Codeerzeugung im Programm!
// ! Um die Taktrate zu ändern müssen die Fuses des Controllers
// ! und/oder Quarz/Resonator/RC-Glied/Oszillator
// ! angepasst werden!
#ifndef F_CPU
#define F_CPU    1000000
#endif

// So viele IRQs werden jede Sekunde ausgelöst.
// Für optimale Genauigkeit muss
// IRQS_PER_SECOND ein Teiler von F_CPU sein
// und IRQS_PER_SECOND ein Vielfaches von 100.
// Ausserdem muss gelten F_CPU / IRQS_PER_SECOND <= 65536
#define IRQS_PER_SECOND   2000 /* 500 µs */

// Anzahl IRQs pro 10 Millisekunden
#define IRQS_PER_10MS     (IRQS_PER_SECOND / 100)

// Gültigkeitsprüfung.
// Bei ungeeigneten Werten gibt es einen Compilerfehler
#if (F_CPU/IRQS_PER_SECOND > 65536) || (IRQS_PER_10MS < 1) || (IRQS_PER_10MS > 255)
#   error Diese Werte fuer F_CPU und IRQS_PER_SECOND
#   error sind ausserhalb des gueltigen Bereichs!
#endif

// Compiler-Warnung falls die Genauigkeit nicht optimal ist.
// Wenn das nervt für deine Werte, einfach löschen :-)
#if (F_CPU % IRQS_PER_SECOND != 0) || (IRQS_PER_SECOND % 100 != 0)
#   warning Das Programm arbeitet nicht mit optimaler Genauigkeit.
#endif

/********************************************************************** DEFINES ***********************************************************/

#define BEGRUESSUNG     1
#define RUN             2
#define STOP            3


/***************************************************************************** PROTOTYPEN **************************************************/


void wait_10ms (const uint8_t);
void timer1_init();

unsigned char Taste_L (void);
unsigned char Taste_R (void);
unsigned char Taste_M (void);


/************************************************************************* INITIALISIEREN ***************************************************/

unsigned char state = 1;
char entprell = 0;      // Für Entprellung

/***************************************************************************** ISR ***********************************************************/

// Zähler-Variable. Wird in der ISR erniedrigt und in wait_10ms benutzt.
static volatile uint8_t timer_10ms;

// //////////////////////////////////////////////////////////////////////
// Implementierungen der Funktionen
// //////////////////////////////////////////////////////////////////////

#if !defined (TCNT1H)
#error Dieser Controller hat keinen 16-Bit Timer1!
#endif // TCNT1H

// //////////////////////////////////////////////////////////////////////
// Timer1 so initialisieren, daß er IRQS_PER_SECOND
// IRQs pro Sekunde erzeugt.
void timer1_init()
{
    // Timer1: keine PWM
    TCCR1A = 0;

    // Timer1 ist Zähler: Clear Timer on Compare Match (CTC, Mode #4)
    // Timer1 läuft mit vollem MCU-Takt: Prescale = 1
#if defined (CTC1) && !defined (WGM12)
   TCCR1B = (1 << CTC1)  | (1 << CS10);
#elif !defined (CTC1) && defined (WGM12)
   TCCR1B = (1 << WGM12) | (1 << CS10);
#else
#error Keine Ahnung, wie Timer1 fuer diesen AVR zu initialisieren ist!
#endif

    // OutputCompare für gewünschte Timer1 Frequenz
    // TCNT1 zählt immer 0...OCR1A, 0...OCR1A, ...
    // Beim überlauf OCR1A -> OCR1A+1 wird TCNT1=0 gesetzt und im nächsten
    // MCU-Takt eine IRQ erzeugt.
    OCR1A = (unsigned short) ((unsigned long) F_CPU / IRQS_PER_SECOND-1);

    // OutputCompareA-Interrupt für Timer1 aktivieren
#if defined (TIMSK1)
    TIMSK1 |= (1 << OCIE1A);
#elif defined (TIMSK)
    TIMSK  |= (1 << OCIE1A);
#else
#error Keine Ahnung, wie IRQs fuer diesen AVR zu initialisieren sind!
#endif
}

// //////////////////////////////////////////////////////////////////////
// Wartet etwa t*10 ms.
// timer_10ms wird alle 10ms in der Timer1-ISR erniedrigt.
// Weil es bis zum nächsten IRQ nicht länger als 10ms dauert,
// wartet diese Funktion zwischen (t-1)*10 ms und t*10 ms.
void wait_10ms (const uint8_t t)
{
    timer_10ms = t;
    while (timer_10ms);
}

// //////////////////////////////////////////////////////////////////////
// Die Interrupt Service Routine (ISR).
// In interrupt_num_10ms werden die IRQs gezählt.
// Sind IRQS_PER_10MS Interrups geschehen,
// dann sind 10 ms vergangen.
// timer_10ms wird alle 10 ms um 1 vermindert und bleibt bei 0 stehen.
SIGNAL (SIG_OUTPUT_COMPARE1A) //  für GGC bis Version 3.x
//   ab Version GCC 4.x   ISR(SIG_OUTPUT_COMPARE1A)
{
    static uint8_t interrupt_num_10ms;

    // interrupt_num_10ms erhöhen und mit Maximalwert vergleichen
    if (++interrupt_num_10ms == IRQS_PER_10MS)
    {
        // 10 Millisekunden sind vorbei
        // interrupt_num_10ms zurücksetzen
        interrupt_num_10ms = 0;

        // Alle 10ms wird timer_10ms erniedrigt, falls es nicht schon 0 ist.
        // Wird verwendet in wait_10ms
        if (timer_10ms != 0)                /****** 10 ms ******/
        {
            timer_10ms--;
        }

          if (entprell > 0)
            {
                entprell --;
            }

    }
}
/************************************************************************************************************************************************************/



/******************************************************************** MAIN**********************************************************************************/

int main()
{

    DDRB = 0b11111000;    // Taster als Eingänge und LEDs + LCD als Ausgänge definieren
    DDRC = 0xFF;    // PortC als Ausgang definieren für LCD
    DDRD = 0xFF;    // Port D als Ausgänge definieren für LEDs

    PORTB &= ~(1<<PB6); // PB6 auf 0 setzen
    PORTB &= ~(1<<PB7); // PB7 auf 0 setzen

    PORTD = 0x00;   // auf Null setzen




    DDR_LED  |= (1 << PAD_LED);// LED-Port auf OUT


    timer1_init();  // Timer1 initialisieren


    sei();  // Interrupts aktivieren

    lcd_init(LCD_DISP_ON);  // initialisieren von LCD

    lcd_clrscr();

    lcd_gotoxy(0, 0);

    lcd_puts ("WILLKOMMEN");


    while (1)
    {

        switch (state)
        {
            case BEGRUESSUNG:    if(Taste_M() && (entprell == 0))
                                {
                                    entprell = 50;

                                    lcd_clrscr();
                                    lcd_gotoxy(0,0);
                                    lcd_puts ("RUN");

                                    state = RUN;
                                }

                                break;

            case   RUN:         if(Taste_M() && (entprell == 0))
                                {
                                    entprell = 50;

                                    lcd_clrscr();
                                    lcd_gotoxy(0, 0);
                                    lcd_puts("STOP");

                                    state = STOP;
                                }

                                break;

            case STOP:          if(Taste_M() && (entprell == 0))
                                {
                                    entprell = 50;

                                    lcd_clrscr();
                                    lcd_gotoxy(0, 0);
                                    lcd_puts("WILLKOMMEN");

                                    state = BEGRUESSUNG;
                                }

                                break;

        }

    }
}

/********************************************************************************* FUNKTIONEN *********************************************************/


unsigned char Taste_L (void)
{
    if (!(PINB & (1<<PINB2)))
    {
            return 1;
    }

    else return 0;
}

unsigned char Taste_M (void)
{
    if (!(PINB & (1<<PINB1)))
    {
            return 1;
    }

    else return 0;
}

unsigned char Taste_R (void)
{
    if (!(PINB & (1<<PINB0)))
    {
            return 1;
    }

    else return 0;
}
