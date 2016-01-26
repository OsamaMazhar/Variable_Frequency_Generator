/*
 * Title: Variable Frequency Generator
 * Author: Osama Mazhar
 * Date: 21st Jan 2012
 * Email: osamazhar@yahoo.com
 */
 
  
#include <reg51.h>

#define KEYPAD P1												
#define ldata P0 											//sfr ldata = 0x0B0; for P3

sbit rs = P3^5;
sbit rw = P3^6;
sbit en = P3^7;
sbit busy = P0^7;
sbit Clock = P3^0;
sbit Status = P3^1;


void lcdready();
void lcdcmd(unsigned char);
void lcddata(unsigned char);
void MSDelay(unsigned int);
unsigned char KeypadRead();
unsigned int GetInput();
void TimerH(unsigned char, unsigned char);
void TimerL(unsigned char, unsigned char);

unsigned char code keypad[4][4] = {'7','8','9','/','4','5','6','X','1','2','3','-','.','0','=','+'};



void main()
	{
	 float Frequency, Percent, LowTime, HighTime, TotalDelayMilli, TotalDelayMicro;
	 unsigned int ConvHH, ConvLH, ValueH, ConvHL, ConvLL, ValueL;
	 unsigned char x, HighH, LowH, HighL, LowL;
	 unsigned char Freq[17] = "Freq:         Hz", DutyC[17] = "Duty Cycle:    %";
	 unsigned char colloc;

	 Status = 0;
	 KEYPAD = 0xF0;											// make higher bits of keypad port that is column bits input and low row bits outputs
	 Clock = 0;
	 lcdcmd(0x38);											//2 lines and 5X7 matrix
	 lcdcmd(0x0C);											//Display ON, Cursor Blinking = 0x0E But this command is Display ON, Cursor OFF
	 lcdcmd(0x01);											//Clear Display Screen
	 lcdcmd(0x80);											//Force Cursor to the beginning of the First Line
	 lcdcmd(0x06);
	 
main:lcdcmd(0x01);
	 for(x = 0; x<16; x++)
	 	lcddata(Freq[x]);
	 lcdcmd(0xC0);
	 for(x = 0; x<16; x++)
	 	lcddata(DutyC[x]);
	 lcdcmd(0x80);
	 for(x=0; x<6; x++)
	 	lcdcmd(0x16);
	 lcdcmd(0x0E);
	 Frequency = GetInput();
	 if(Frequency > 20000)
	 	goto main;
	 lcdcmd(0x0C);
	 lcdcmd(0xC0);
	 for(x = 0; x<12; x++)
	 	lcdcmd(0x16);
	 lcdcmd(0x0E);
	 Percent = GetInput();
	 lcdcmd(0x0C);
	 TotalDelayMilli = (1 / Frequency) * 1000;
	 TotalDelayMicro = (TotalDelayMilli * 1000) - 45;		//With correction of 45
	 HighTime = (TotalDelayMicro / 100) * Percent;
	 LowTime = TotalDelayMicro - HighTime;

	 ValueH = 65536 - HighTime;									 
	 ConvLH = ValueH & 0x00FF;								 // Zeroing the Higher 8 bits to have Lower 8 bits
	 LowH = ConvLH;											 // Storing in a 8 bit character
	 ConvHH = ValueH & 0xFF00;								 // Masking the Lower 8 bits to have Higher 8 bits
	 ConvHH = ConvHH >> 8;									 // Shifting right 8 bits to hit to the Lower Part to save it to a 8-bit character
	 HighH = ConvHH;										 // Storing in a 8 bit character
	 
	 ValueL = 65536 - LowTime;									
	 ConvLL = ValueL & 0x00FF;								 // Zeroing the Higher 8 bits to have Lower 8 bits
	 LowL = ConvLL;											 // Storing in a 8 bit character
	 ConvHL = ValueL & 0xFF00;								 // Masking the Lower 8 bits to have Higher 8 bits
	 ConvHL = ConvHL >> 8;									 // Shifting right 8 bits to hit to the Lower Part to save it to a 8-bit character
	 HighL = ConvHL;									
	 
	 Status = 1;
	 while(1)												//2 MC
		{
		 Clock = 1;											//1 MC
		 TimerH(HighH, LowH);
		 Clock = 0;
		 TimerL(HighL, LowL);
		 colloc = KEYPAD;									//2 MC		 
	 	 colloc &= 0xF0;									//2 MC		 
		 if(colloc == 0xE0)									//4 MC
		 	{
	 	 	 MSDelay(5);
		 	 colloc = KEYPAD;								//read the columns
		 	 colloc &= 0xF0;								//mask unused bits
			 if(colloc == 0xE0)
			 	{
				 Status = 0;
				 goto main;	 
				}
			}												
		}
	}														//main braces

unsigned int GetInput()
	{
	 unsigned char L, cnt, Key[5];
	 unsigned int Entry = 0;
	 cnt = 0;
	 do
		{
Re:		 Key[cnt] = KeypadRead();
		 L = Key[cnt];
		 if(L == '+' | L == '-')
		 	goto Re;
		 if(L == '=')
		 	continue;
		 lcddata(Key[cnt]);
		 Key[cnt] = Key[cnt] - 48;
		 cnt++;

		}
	 while(L != '=' & cnt != 5);
	 switch(cnt)
	 	{
		 case(1):
		 	{
			 Entry = Key[0];
			 break;
			}
		 case(2):
		 	{
			 Entry = (Key[0] * 10) + Key[1];
			 break;
			}
		 case(3):
		 	{
			 Entry = (Key[0] * 100) + (Key[1] * 10) + Key[2];
			 break;
			}
		 case(4):
		 	{
			 Entry = (Key[0] * 1000) + (Key[1] * 100) + (Key[2] * 10) + Key[3];
			 break;
			}
		 case(5):
		 	{
			 Entry = (Key[0] * 10000) + (Key[1] * 1000) + (Key[2] * 100) + (Key[3] * 10) + Key[4];
			 break;
			}
		}
	 return Entry;
	}

void MSDelay(unsigned int count) 
	{// mSec Delay 11.0592 Mhz 
     unsigned int i;		       		
     while(count)
		{
         i = 122; 
		 while(i>0)
		 	i--;
         count--;
    	}
	}

void lcdready()
	{														
		busy = 1;	  										//make the busy pin an input
		rs = 0;
		rw = 1;
		while(busy == 1)									//wait here for busy flag
			{
				en = 0;										//strobe the enable pin
				MSDelay(1);
				en = 1;
			}
		return;					
	 }
	 	  
void lcdcmd(unsigned char value)
	{
		lcdready();
		ldata = value;
		rs = 0;
		rw = 0;
		en = 1;
		MSDelay(1);
		en = 0;
		return;
	}


void lcddata(unsigned char value)
	{
		lcdready();
		ldata = value;
		rs = 1;
		rw = 0;
		en = 1;
		MSDelay(1);
		en = 0;
		return;
	}

unsigned char KeypadRead()
	{
		unsigned char colloc, rowloc; 		
		do
		   {												
		   KEYPAD = 0xF0;									//ground all rows at once
		   colloc = KEYPAD;									//read the port for columns
		   colloc &= 0xF0;									//mask row bits
		   }
		while(colloc != 0xF0);								//check until all keys are released
		
		do
		   {
		  	 do
		   		{
				MSDelay(8);  								//call delay
				colloc = KEYPAD;							//see if any key is pressed
				colloc &= 0xF0;								//mask unsused bits
				}
		 	 while(colloc == 0xF0);							//keep checking for keypress

			MSDelay(8);										//call delay for debounce
			colloc = KEYPAD;								//read columns
			colloc &= 0xF0;									//masku unused bits
			}
			while(colloc == 0xF0);							//wait for keypress

			while(1)
				{
					KEYPAD &= 0xF0;							//masking row bits
					KEYPAD |= 0x0E;							//now ground row 0 0E = 00001110b ORing won't affect column data
					colloc = KEYPAD;						//read columns
					colloc &= 0xF0;							//mask row bits
					if(colloc != 0xF0)						//column detected
						{
							rowloc = 0;						//save row location
							break;
						}
					KEYPAD &= 0xF0;
					KEYPAD |= 0x0D;							//ground row 1 0D = 00001101b  ORing won't affect column data
					colloc = KEYPAD;						//read columns
					colloc &= 0xF0;							//mask row bits
					if(colloc != 0xF0)						//column detected
						{
							rowloc = 1;						//save row location
							break;
						}
					KEYPAD &= 0XF0;
					KEYPAD |= 0x0B;							//ground row 2 0B = 00001011b
					colloc = KEYPAD;						//read columns
					colloc &= 0xF0;							//mask row bits
					if(colloc != 0xF0)						//column detected
						{
							rowloc = 2;						//save row location
							break;
						}
					KEYPAD &= 0XF0;
					KEYPAD |= 0x07;						//ground row 3 07 = 00000111b
					colloc = KEYPAD;					//read columns
					colloc &= 0xF0;						//mask row bits
					if(colloc != 0xF0)					//column detected
					rowloc = 3;							//save row location
					break;
				}
		//check columns and send result to LCD
		if(colloc == 0xE0)							//0E = 00001110
			return keypad[rowloc][0];
		else if(colloc == 0xD0)						//0D = 00001101
			return keypad[rowloc][1];
		else if(colloc == 0xB0)						//0B = 00001011
			return keypad[rowloc][2];
		else
			return keypad[rowloc][3];
		}

void TimerH(unsigned char High, unsigned char Low)
	{
	 TMOD = 0x01;
	 TL0 = Low;
	 TH0 = High;
	 TR0 = 1;
	 while(TF0 == 0);
	 TR0 = 0;
	 TF0 = 0;	 
	}
void TimerL(unsigned char High, unsigned char Low)
	{
	 TMOD = 0x01;
	 TL0 = Low;
	 TH0 = High;
	 TR0 = 1;
	 while(TF0 == 0);
	 TR0 = 0;
	 TF0 = 0;	 
	} 	 