#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#define F_CPU 8000000UL
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "serial.c"
#include "adc.c"
#include "hc12.c"

#define PULS 500UL			//PULS in MS
#define PERIODE 1000UL		//Periode in MS

/*
* Die NTC Tabelle, bestehend aus 1024 Temperaturstützpunkten.
* Einheit: 0.1 °C
* 12k PullDown Resistor
*/
short NTC_table[1024] PROGMEM =
{
  -852, -808, -764, -731, -705, -683, -664, 
  -647, -633, -619, -607, -595, -584, -574, 
  -565, -556, -548, -540, -532, -525, -518, 
  -511, -505, -498, -492, -487, -481, -476, 
  -470, -465, -460, -455, -451, -446, -441, 
  -437, -433, -428, -424, -420, -416, -412, 
  -409, -405, -401, -398, -394, -391, -387, 
  -384, -381, -377, -374, -371, -368, -365, 
  -362, -359, -356, -353, -350, -347, -345, 
  -342, -339, -336, -334, -331, -329, -326, 
  -323, -321, -318, -316, -314, -311, -309, 
  -306, -304, -302, -299, -297, -295, -293, 
  -291, -288, -286, -284, -282, -280, -278, 
  -276, -274, -271, -269, -267, -265, -263, 
  -261, -260, -258, -256, -254, -252, -250, 
  -248, -246, -244, -243, -241, -239, -237, 
  -235, -234, -232, -230, -228, -227, -225, 
  -223, -221, -220, -218, -216, -215, -213, 
  -211, -210, -208, -207, -205, -203, -202, 
  -200, -199, -197, -195, -194, -192, -191, 
  -189, -188, -186, -185, -183, -182, -180, 
  -179, -177, -176, -174, -173, -171, -170, 
  -169, -167, -166, -164, -163, -161, -160, 
  -159, -157, -156, -155, -153, -152, -150, 
  -149, -148, -146, -145, -144, -142, -141, 
  -140, -138, -137, -136, -134, -133, -132, 
  -130, -129, -128, -127, -125, -124, -123, 
  -122, -120, -119, -118, -117, -115, -114, 
  -113, -112, -110, -109, -108, -107, -105, 
  -104, -103, -102, -101, -99, -98, -97, -96, 
  -95, -93, -92, -91, -90, -89, -88, -86, -85, 
  -84, -83, -82, -81, -79, -78, -77, -76, -75, 
  -74, -73, -71, -70, -69, -68, -67, -66, -65, 
  -64, -62, -61, -60, -59, -58, -57, -56, -55, 
  -54, -52, -51, -50, -49, -48, -47, -46, -45, 
  -44, -43, -42, -41, -39, -38, -37, -36, -35, 
  -34, -33, -32, -31, -30, -29, -28, -27, -26, 
  -25, -24, -23, -22, -20, -19, -18, -17, -16, 
  -15, -14, -13, -12, -11, -10, -9, -8, -7, 
  -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 
  6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 
  18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 
  29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 
  40, 41, 42, 43, 44, 45, 46, 46, 47, 48, 49, 
  50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 
  61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 
  71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 
  82, 83, 84, 85, 86, 87, 88, 89, 89, 90, 91, 
  92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 
  102, 103, 104, 105, 106, 106, 107, 108, 109, 
  110, 111, 112, 113, 114, 115, 116, 117, 118, 
  119, 120, 121, 122, 122, 123, 124, 125, 126, 
  127, 128, 129, 130, 131, 132, 133, 134, 135, 
  136, 137, 138, 138, 139, 140, 141, 142, 143, 
  144, 145, 146, 147, 148, 149, 150, 151, 152, 
  153, 154, 155, 155, 156, 157, 158, 159, 160, 
  161, 162, 163, 164, 165, 166, 167, 168, 169, 
  170, 171, 172, 173, 173, 174, 175, 176, 177, 
  178, 179, 180, 181, 182, 183, 184, 185, 186, 
  187, 188, 189, 190, 191, 192, 193, 194, 195, 
  195, 196, 197, 198, 199, 200, 201, 202, 203, 
  204, 205, 206, 207, 208, 209, 210, 211, 212, 
  213, 214, 215, 216, 217, 218, 219, 220, 221, 
  222, 223, 224, 225, 226, 227, 228, 229, 230, 
  231, 232, 232, 233, 234, 235, 236, 237, 238, 
  239, 240, 241, 242, 243, 244, 245, 246, 247, 
  248, 249, 250, 251, 252, 253, 254, 256, 257, 
  258, 259, 260, 261, 262, 263, 264, 265, 266, 
  267, 268, 269, 270, 271, 272, 273, 274, 275, 
  276, 277, 278, 279, 280, 281, 282, 283, 284, 
  285, 286, 287, 288, 290, 291, 292, 293, 294, 
  295, 296, 297, 298, 299, 300, 301, 302, 303, 
  304, 306, 307, 308, 309, 310, 311, 312, 313, 
  314, 315, 316, 317, 319, 320, 321, 322, 323, 
  324, 325, 326, 327, 329, 330, 331, 332, 333, 
  334, 335, 336, 337, 339, 340, 341, 342, 343, 
  344, 345, 347, 348, 349, 350, 351, 352, 353, 
  355, 356, 357, 358, 359, 360, 362, 363, 364, 
  365, 366, 368, 369, 370, 371, 372, 374, 375, 
  376, 377, 378, 380, 381, 382, 383, 384, 386, 
  387, 388, 389, 391, 392, 393, 394, 396, 397, 
  398, 399, 401, 402, 403, 404, 406, 407, 408, 
  409, 411, 412, 413, 415, 416, 417, 419, 420, 
  421, 422, 424, 425, 426, 428, 429, 430, 432, 
  433, 434, 436, 437, 438, 440, 441, 443, 444, 
  445, 447, 448, 449, 451, 452, 454, 455, 456, 
  458, 459, 461, 462, 464, 465, 466, 468, 469, 
  471, 472, 474, 475, 477, 478, 480, 481, 483, 
  484, 486, 487, 489, 490, 492, 493, 495, 496, 
  498, 499, 501, 502, 504, 506, 507, 509, 510, 
  512, 514, 515, 517, 518, 520, 522, 523, 525, 
  527, 528, 530, 531, 533, 535, 537, 538, 540, 
  542, 543, 545, 547, 549, 550, 552, 554, 556, 
  557, 559, 561, 563, 564, 566, 568, 570, 572, 
  574, 575, 577, 579, 581, 583, 585, 587, 589, 
  591, 593, 594, 596, 598, 600, 602, 604, 606, 
  608, 610, 612, 614, 616, 619, 621, 623, 625, 
  627, 629, 631, 633, 635, 638, 640, 642, 644, 
  646, 649, 651, 653, 655, 658, 660, 662, 665, 
  667, 669, 672, 674, 676, 679, 681, 684, 686, 
  689, 691, 694, 696, 699, 701, 704, 706, 709, 
  712, 714, 717, 720, 722, 725, 728, 730, 733, 
  736, 739, 742, 745, 747, 750, 753, 756, 759, 
  762, 765, 768, 771, 774, 778, 781, 784, 787, 
  790, 794, 797, 800, 804, 807, 810, 814, 817, 
  821, 824, 828, 831, 835, 839, 843, 846, 850, 
  854, 858, 862, 866, 870, 874, 878, 882, 886, 
  890, 895, 899, 903, 908, 912, 917, 921, 926, 
  931, 936, 940, 945, 950, 955, 960, 966, 971, 
  976, 981, 987, 992, 998, 1004, 1010, 1016, 
  1022, 1028, 1034, 1040, 1047, 1053, 1060, 
  1066, 1073, 1080, 1087, 1095, 1102, 1110, 
  1117, 1125, 1133, 1141, 1150, 1158, 1167, 
  1176, 1185, 1195, 1204, 1214, 1224, 1235, 
  1245, 1256, 1267, 1279, 1291, 1303, 1316, 
  1329, 1342, 1356, 1371, 1386, 1401, 1418, 
  1434, 1452, 1470, 1489, 1509, 1530, 1552, 
  1575, 1599, 1625, 1652, 1681, 1712, 1746, 
  1781, 1820, 1862, 1908, 1959, 2015, 2078, 
  2150, 2233, 2330, 2447, 2592, 2781, 3045, 
  3463, 4336, 5209
};
 
short NTC_ADC2Temperature(unsigned short adc_value)
{
  //Werte direkt aus der Tabelle lesen.
  return pgm_read_word(&(NTC_table[adc_value]));
};

//------------------------------MAIN------------------------------------
int main(void)
{
	unsigned char Output[10];
	unsigned short temp = 0;
	unsigned char i = 0;
		
	DDRC = 0x00;
	DDRB = 0xFF;
	DDRD = 0x02;

	ADC_init(0x01);
	USART_Init(51);
	HC_init(96,1,1);
	
	while(1)
	{
		temp = read_ADC(0);

		//für Temperatursensor (Nur  Temperatur auf 0.1C genau 25.6C = 256)
		sprintf(Output, "%03d\r", NTC_ADC2Temperature(temp));
		
		USART_Transmit_STRING(Output);

/*		//Transmit ADC value via SPI (MSB First)
		for(i = 0; i < 10; i++)
		{
			//Signal PB6
			if(temp & 0x200)
				PORTB |= (1<<PB6);
			else
				PORTB &= ~(1<<PB6);

			temp <<= 1;
			
			//Clock PB7
			PORTB |= (1<<PB7);
			//_delay_us(10);
			PORTB &= ~(1<<PB7);
		}
		PORTB |= (1<<PB6);	//led anschalten
*/

		_delay_ms(2000);
//------------------------------------------
	}//end of while
}//end of main
