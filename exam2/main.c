#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include "SetClk.h"

int state; //1 = START; 0 = STOP
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

// duration (periods), time_on (periods), time_off (periods)
int tone_map[4][3] = 
{
	{1953, 3, 2},  	// 1480 Hz	F#6
	{7813, 3, 3}, 	// 1245 Hz	D#6
	{11719, 3, 4}, 	// 1108 Hz	C#6
	{15625, 7, 7}		// 555 Hz	C#5
};

int tone_index;

// timer vars
int time; // measures time in us
int time_max;
int time_on;
int time_off;
int num_periods;
int loop_count;
int loop_num;
int enable_siren;

// debouncing code
int is_debouncing;
int debounce_count;


void main(void) {
	// init variables
	pat_index = 0;
	digit_index = 0;
	state = 1;
	count_max = 7813; // ever so slightly off should be 7812.5
	time = 0;
	time_max = 0;
	num_periods = 0;
	tone_index = 0;
	loop_count = 0;
	loop_num = 2;
	enable_siren = 0;
	
	
	SetClk8();
	RTICTL = 0x10;					//RTI interrupt interval set to 2^10, period is 128us
	DDRB = 0xFF;					//configure port b for output
	DDRP = 0xFF;					//configure port p for output
	DDRT = 0xFF;					// configure port t for output   
	CRGINT |= CRGINT_RTIE_MASK;
	
	PIFH = PIFH_PIFH0_MASK;			// clear interrupt flag register for H1		- INTERRUPT FLAG REGISTER
	PIEH = PIFH_PIFH0_MASK;			// enable PTH interrupt						- INTERRUPT ENABLE REGISTER
	PERH = PERH_PERH0_MASK;			// Enable pull-up or pull-down for pin 1	- PULL DEVICE ENABLE REGISTER	
	PPSH = 0x00;					// falling edge sets associated flag in PIFH								- PORT H POLARITY SELECT REGSITER
	DDRH = 0xFE;					// select H1 as input						- PORT H DATA DIRECTION REGISTER
	
	asm("CLI");						// enable interrupt globally
	
	while(1);
}

//RTI interrupt handlerhandler
void interrupt 7 RTI_ISR() {
	CRGFLG 	= 0x80;   			// clear RTIF bit

		
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
	
	// buzzer
	if (state == 0 && enable_siren == 1) {
		// create tone
		time_max = tone_map[tone_index][0];
		// set frequency
		time_on = tone_map[tone_index][1];
		time_off = tone_map[tone_index][2];
		
		// output pin operations
		if (++num_periods >= time_off + time_on) {
			PTT = 0xFF;
			num_periods = 0;
		} else if (num_periods == time_on) {
			PTT = 0x00;
		}
		
		// reset time
		if (++time >= time_max) {
			time = 0;
			num_periods = 0;
			
			if (++tone_index >= 4) {
				tone_index = 0;
				
				// loop tones n times
				if (++loop_count >= loop_num) {
					loop_count = 0;
					enable_siren = 0;
				}
			}
		}
	}
	
	// debounce code
	if (is_debouncing == 1) {
		// delay 128 ms
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

// Key interrupt handler
void interrupt 25 KEY_HANDLER() {
	is_debouncing = 1;
}
