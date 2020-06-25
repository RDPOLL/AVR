// Titel:
// Datei:
// Funktion:
// Urheber:		Erasmo Colacicco
// Datum:
/*********************************************************************************************************************************************************************************************************/

#include <stdlib.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "lcd.h"
//#include <C:\Users\Gast\Desktop\Lehrlinge\Erasmo Colacicco\uC Kurs 2\Versuch\Robin\lcd.h>


int main (void)
{

    lcd_init(LCD_DISP_ON);  // initialisieren

    lcd_clrscr();

        /* put string to display (line 1) with linefeed */
        lcd_puts("LCD Test Line 1\n");

        /* cursor is now on second line, write second line */
        lcd_puts("Line 2");



    while (1)
    {

    }
}
