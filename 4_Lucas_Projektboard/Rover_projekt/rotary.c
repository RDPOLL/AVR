/*
 	This libary is for the AVR Pioneer board.
*/

#define PIN_RIGHT PB0
#define PIN_LEFT PB1
#define PIN_PRESS PB2

//------------------------------Flankentriggerung-----------------------

struct
{
	unsigned char act;					//Variabeln fuer die Flankentriggerung
	unsigned char old;
	unsigned char flk;
	unsigned char pos_flk;
	unsigned char neg_flk;
}flk = {0};

void flk_t(unsigned char in)			//PORT in uebergabe wert angeben
{
	flk.act = in;
	flk.flk = flk.act ^ flk.old;
	flk.neg_flk = flk.flk & flk.old;
	flk.pos_flk = flk.flk & flk.act;
	flk.old = flk.act;

	return;
}
//------------------------------END-------------------------------------

//------------------------------Rotary----------------------------------

struct
{
	unsigned char right;
	unsigned char left;
	unsigned char press;
}rotary = {0};

void rotary_f(unsigned char in)
{	
	rotary.right = 0;
	rotary.left = 0;
	rotary.press = 0;
	
	if(flk.neg_flk & (1 << PIN_PRESS)) rotary.press = 1;
	if((flk.neg_flk & (1 << PIN_RIGHT)) && (in & (1 << PIN_LEFT))) rotary.right = 1;
	if((flk.neg_flk & (1 << PIN_LEFT)) && (in & (1 << PIN_RIGHT))) rotary.left = 1;
	
	return;
}

void usrInput(unsigned char in)
{
	flk_t(in);			//call flankentriggerung
	rotary_f(in);		//call rotary
	//_delay_ms(5);		//for stability
}
