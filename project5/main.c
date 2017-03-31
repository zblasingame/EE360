#include <hidef.h>
#include "derivative.h"
#include "SetClk.h"

// Declare variables
int count, count_max;
int time;
int state; // 0 = reset, 1 = start, 2 = stop
int enable; // 0 = disable, 1 = enable;

// char array where index corresponds to decimal value
char segPat[10] = {0x3F, 0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
char digit[4] 	= {0xFE,0xFD,0xFB,0xF7};
int digit_index;
char time_arr[4] = {0x3F, 0x3F, 0x3F, 0x3F}; // format MM:SS


void format_time(int time) {
	int minutes;
	int seconds;
	
	minutes = time / 60;
	seconds = time % 60;
	
	time_arr[0] = segPat[minutes / 10];
	time_arr[1] = segPat[minutes % 10];
	time_arr[2] = segPat[seconds / 10];
	time_arr[3] = segPat[seconds % 10];
}

void update_state() {
	switch (state) {
		case 0:
			enable = 0;
			time = 0;
			// convert time to formatted time
			format_time(time);
			break;
		case 1:
			enable = 1;
			break;
		case 2:
			enable = 0;
			break;
		default:
			break;
	}
	
	if (++state > 2) {
		state = 0;
	}
}

// main method declared here
void main(void) {
	digit_index = 0;
	count_max 	= 976;					// performed math to get magic number, sets frequency to 1 Hz
	count 		= 0;
	time 		= 0;
	state		= 0;
	enable	 	= 0;
	
	// initialize E clock to 24 MHz
	SetClk8();
	RTICTL		= 0x40; 				// RTI interrupt interval set to 2**13 OSCCLK cycles
	DDRB		= 0xFF; 				// configure Port B for ouput
	DDRP		= 0xFF; 				// conigure Port P for output
	CRGINT		|= CRGINT_RTIE_MASK; 	// enable interupt
	
	// Enable PTH Interrupt
	PIFH		= 0x01;
	DDRH		= 0xFE;					// PTH as input
	PIEH		= 0x01;					// Enable PTH interrupt
	PERH		= 0x01;					// Enable pull-up or pull_down for pin 0
	PPSH		= 0xFE;					// Make PTH Interrupt falling edge-trigger
	
	asm("CLI");							// enable interupt globally
	
	// loop forever
	while(1);
}

// RTI interrupt service routine
void interrupt 7 RTI_ISR() {
	CRGFLG 	= 0x80;   			// clear RTIF bit
	
	PORTB	= time_arr[digit_index];
	PTP		= digit[digit_index];
	
	// add colon between MM and SS
	if (digit_index == 1 || digit_index == 2) {
		PORTB += 0x80;
	}
	
	if (++digit_index >= 4) {
		digit_index = 0;
	}
	
	if (++count >= count_max && enable == 1) {
		time++;
		// convert time to formatted time
		format_time(time);
		count = 0;
	}
	
	if (time >= 3599 && enable == 1) {
		update_state(); // automatically stop
	}
	
}

// Key interrupt handler
void interrupt 25 KEY_HANDLER() {
	PIFH = 0x01;
	
	update_state();
}
