;*****************************************************************
;* This stationery serves as the framework for a                 *
;* user application (single file, absolute assembly application) *
;* For a more comprehensive program that                         *
;* demonstrates the more advanced functionality of this          *
;* processor, please see the demonstration applications          *
;* located in the examples subdirectory of the                   *
;* Freescale CodeWarrior for the HC12 Program directory          *
;*****************************************************************

; Authors: Z. Blasingame, D. Glass, J. DeMeyers

; export symbols
            XDEF Entry, _Startup            ; export 'Entry' symbol
            ABSENTRY Entry        ; for absolute assembly: mark this as application entry point



; Include derivative-specific definitions 
		include 'hcs12.inc'
		
ROMStart    EQU  	$4000  			; absolute address to place my code/constant data

; variable/data section

            ORG 	$1500
; Insert here your data definition.
;Counter     DS.W 1
;FiboRes     DS.W 1


; code section
            ORG   	ROMStart


Entry:
_Startup:
            LDS   	#ROMStart       ; initialize the stack pointer
            CLI                     ; enable interrupts
mainLoop:
			bset 	DDRT,BIT5 		; configure PT5 pin for output
forever 	ldx 	#250 			; repeat 500 Hz waveform 250 times
tone1 		bset 	PTT,BIT5 		; pull PT5 pin to high
			ldy 	#1
			jsr 	delayby1ms
			bclr 	PTT,BIT5
			ldy 	#1
			jsr 	delayby1ms
			dbne 	x,tone1
			ldx 	#100 			; repeat 200 Hz waveform for 100 times
tone2 		bset 	PTT,BIT5
			ldy 	#2
			jsr 	delayby1ms
			bclr 	PTT,BIT5
			ldy 	#2
			jsr 	delayby1ms
			dbne 	x,tone2
			ldx 	#500 			; repeat 1000 Hz waveform for 500 times
tone3 		bset 	PTT,BIT5
			ldy 	#10
			jsr 	delayby50us
			bclr 	PTT,BIT5
			ldy 	#10
			jsr 	delayby50us
			dbne 	x,tone3
			bra 	forever

; ***************************************************************************
; The following function creates a time delay which is equal to the multiple
; of 1 ms. The value passed in Y specifies the number of milliseconds to be
; delayed.
; ***************************************************************************
delayby1ms	movb	#$90,TSCR1	; enable TCNT & fast flags clear
			movb	#$06,TSCR2 	; configure prescale factor to 64
			movb	#$01,TIOS	; enable OC0
			ldd 	TCNT
again0		addd	#375		; start an output compare operation
			std	TC0		; with 50 ms time delay
wait_lp0	brclr	TFLG1,$01,wait_lp0
			ldd	TC0
			dbne	y,again0
			rts
			
; *******************************************************************************************
; The following subroutine uses modulus counter to create a delay that is a multiple of
; 50 us. The multiple is passed in Y.
; *******************************************************************************************
delayby50us
			bset	TSCR1,TFFCA	; enable timer fast flag clear
			movb	#04,MCCTL	; enable modulus down counter with 1:1 as prescaler
			movw	#1200,MCCNT	; load the value to be down counted
			brclr	MCFLG,MCZF,*
			bclr	MCCTL,$04	; disable modulus down counter
			dbne	y,delayby50us
			rts
	
;**************************************************************
;*                 Interrupt Vectors                          *
;**************************************************************
            ORG   $FFFE
            DC.W  Entry           ; Reset Vector
			
			end

