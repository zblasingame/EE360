#include <hidef.h>      
#include "derivative.h"      
#include "SetClk.h"

int state; 
int count;
int count_max;

char wordPat[9][4] = 
{
	{0x3F, 0x3F, 0x3F, 0x3F},
	{0x3F, 0x77, 0x3F, 0x77},
	{0x77, 0x3F, 0x77, 0x3F},
	{0x71, 0x79, 0x66, 0x6D},
	{0x77, 0x7C, 0x39, 0x5E},
	{0x5E, 0x39, 0x7C, 0x77}, 
	{0x71, 0x79, 0x79, 0x5E},
	{0x5E, 0x79, 0x77, 0x5E},
	{0x7C, 0x79, 0x79, 0x71}
};

int pat_index;
char digit[4] = {0xFE, 0xFD, 0xFB, 0xF7};
int digit_index;


int tone_map[4][3] = 
{
	{1953, 3, 2},  	
	{7813, 3, 3}, 	
	{11719, 3, 4}, 	
	{15625, 7, 7}		
};

int tone_index;


int time; 
int time_max;
int time_on;
int time_off;
int num_periods;
int loop_count;
int loop_num;
int enable_siren;


int is_debouncing;
int debounce_count;


void main(void) {
	
	pat_index = 0;
	digit_index = 0;
	state = 1;
	count_max = 7813; 
	time = 0;
	time_max = 0;
	num_periods = 0;
	tone_index = 0;
	loop_count = 0;
	loop_num = 2;
	enable_siren = 0;
	
	
	SetClk8();
	RTICTL = 0x10;					
	DDRB = 0xFF;					
	DDRP = 0xFF;					
	DDRT = 0xFF;					
	CRGINT |= CRGINT_RTIE_MASK;
	
	PIFH = PIFH_PIFH0_MASK;			
	PIEH = PIFH_PIFH0_MASK;			
	PERH = PERH_PERH0_MASK;			
	PPSH = 0x00;					
	DDRH = 0xFE;					
	
	asm("CLI");						
	
	while(1);
}


void interrupt 7 RTI_ISR() {
	CRGFLG 	= 0x80;   			

		
	PORTB = wordPat[pat_index][digit_index];
	PTP = digit[digit_index];

	
	if (++digit_index >= 4) {
		digit_index = 0;
	}
	
	if (state == 1) {
		if (++count >= count_max) {
			count=0;
			
			if (++pat_index >= 9) {
				pat_index = 0;
			}
		}
	}
	
	
	if (state == 0 && enable_siren == 1) {
		
		time_max = tone_map[tone_index][0];
		
		time_on = tone_map[tone_index][1];
		time_off = tone_map[tone_index][2];
		
		
		if (++num_periods >= time_off + time_on) {
			PTT = 0xFF;
			num_periods = 0;
		} else if (num_periods == time_on) {
			PTT = 0x00;
		}
		
		
		if (++time >= time_max) {
			time = 0;
			num_periods = 0;
			
			if (++tone_index >= 4) {
				tone_index = 0;
				
				
				if (++loop_count >= loop_num) {
					loop_count = 0;
					enable_siren = 0;
				}
			}
		}
	}
	
	
	if (is_debouncing == 1) {
		
		if (++debounce_count >= 1000) {
			PIFH = PIFH_PIFH0_MASK;
			is_debouncing = 0;
			
			time = 0;
			PTT = 0xFF;
			tone_index = 0;
			loop_count = 0;

			state = !state;
			enable_siren = !state;
		}
	}
}


void interrupt 25 KEY_HANDLER() {
	is_debouncing = 1;
}
