/*
 	This libary is for the AVR Pioneer board.
*/

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
