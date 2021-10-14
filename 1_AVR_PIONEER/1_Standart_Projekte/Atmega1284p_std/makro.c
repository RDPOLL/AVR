//Hier entsteht eine Sammlung verschiedener, für die hardwarenahe Programmierung nützlicher, C-Makros.

/* Bit setzen */
#define set_bit(var, bit) ((var) |= (1 << (bit)))

/* Bit löschen */
#define clear_bit(var, bit) ((var) &= (unsigned)~(1 << (bit)))

/* Bit togglen */
#define toggle_bit(var,bit) ((var) ^= (1 << (bit)))

/* Bit abfragen */
#define bit_is_set(var, bit) ((var) & (1 << (bit)))
#define bit_is_clear(var, bit) !bit_is_set(var, bit)

/* Konstante in Binärschreibweise angeben (max. 8 Bit) */
/* Beispiel: BIN(1001011)                              */
/*               ^------ ACHTUNG, stets OHNE fuehrende */
/*                       0 angeben, da sonst Oktalzahl */
#define BIN(x) \
        (((x)%10)\
         |((((x)/10)%10)<<1)\
         |((((x)/100)%10)<<2)\
         |((((x)/1000)%10)<<3)\
         |((((x)/10000)%10)<<4)\
         |((((x)/100000)%10)<<5)\
         |((((x)/1000000)%10)<<6)\
         |((((x)/10000000)%10)<<7)\
         )

/* Alternative mit Kommatrennung (genau 8 Bit angeben) */
/* Beispiel: BIN8(0,1,0,0,1,0,1,1)                     */
#define BIN8(b7,b6,b5,b4,b3,b2,b1,b0) \
        ((b0)\
         |((b1)<<1)\
         |((b2)<<2)\
         |((b3)<<3)\
         |((b4)<<4)\
         |((b5)<<5)\
         |((b6)<<6)\
         |((b7)<<7)\
        )

/*Reihenfolge der Bits vertauschen (7..0 -> 0..7)
/*z.&nbsp;B. 10110010 -> 01001101
/*inline assembler für AVR, da die C-Variante mehr als 4 Mal größer und 6 Mal langsamer ist. */
#define swapbits(x) asm ( \
	"lsl %1\n\t ror %0 \n\t " \
	"lsl %1\n\t ror %0 \n\t " \
	"lsl %1\n\t ror %0 \n\t " \
	"lsl %1\n\t ror %0 \n\t " \
	"lsl %1\n\t ror %0 \n\t " \
	"lsl %1\n\t ror %0 \n\t " \
	"lsl %1\n\t ror %0 \n\t " \
	"lsl %1\n\t ror %0 \n\t " \
	: "=&r" (x): "r" (x))
/* am Ende hat das Macro kein Semikolon, das das bei jedem 
   Aufruf "swapbits(x);" angegeben wird. */
   
/* 2 Variablen ohne Zwischenspeicher vertauschen */
#define SWAP(x, y)  do { (x) ^= (y); (y) ^= (x); (x) ^= (y);} while(FALSE)
