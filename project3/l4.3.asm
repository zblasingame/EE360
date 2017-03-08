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

ROMStart    EQU  $4000  ; absolute address to place my code/constant data

; variable/data section

            ORG $1500
 ; Insert here your data definition.
taboff		ds.b 	2
index		ds.b	1
time_count	ds.b	2

; code section
            ORG   	ROMStart


Entry:
_Startup:
            LDS   	#ROMStart       ; initialize the stack pointer
            CLI                     ; enable interrupts
mainLoop:
            movb	#$FF,DDRB
			movb	#$0F,DDRP
			ldd		#0
			std		taboff
forever:
			ldd		#0
			std		time_count
loop_time:
			clr		index
			; add Disptab and taboff for starting address
			ldd		#DispTab
			addd	taboff
			tfr		d,x
loopi:
			movb 	1,x+,PTB		; output segment pattern
			movb	1,x+,PTP		; output display select
			ldy		#1
			jsr		delayby1ms		; wait for 1 ms
			ldaa	index
			inca					; a++
			staa	index
			cmpa    #4				; has reached end of word?
			blt		loopi
			
			; add one to time counter
			ldx		time_count
			inx						; x++
			stx		time_count		; time_count++
			; delay magic number accounts for time to complete
			; inner loop thus a delay of 500ms
			cpx		#200			; if (x < 1000) continue
			blt		loop_time
			
			; update offset
			ldd		#8				; d = 8
			addd	taboff			; d += taboff
			cpd		#80				; check if max
			bge 	reset			; set taboff = 0
			std		taboff			; set taboff += 8
			bra 	forever
reset:		ldd		#0
			std		taboff
			bra 	forever

DispTab:
			; 		num,loc
			;		1234
			dc.b	$66,$07
			dc.b	$4F,$0B
			dc.b	$5B,$0D
			dc.b	$06,$0E
			
			;		2345
			dc.b	$6D,$07
			dc.b	$66,$0B
			dc.b	$4F,$0D
			dc.b	$5B,$0E
			
			;		3456
			dc.b	$7D,$07
			dc.b	$6D,$0B
			dc.b	$66,$0D
			dc.b	$4F,$0E
			
			;		4567
			dc.b	$07,$07
			dc.b	$7D,$0B
			dc.b	$6D,$0D
			dc.b	$66,$0E
			
			;		5678
			dc.b	$7F,$07
			dc.b	$07,$0B
			dc.b	$7D,$0D
			dc.b	$6D,$0E
			
			;		6789
			dc.b	$6F,$07
			dc.b	$7F,$0B
			dc.b	$07,$0D
			dc.b	$7D,$0E
			
			;		7890
			dc.b	$3F,$07
			dc.b	$6F,$0B
			dc.b	$7F,$0D
			dc.b	$07,$0E
			
			;		8901
			dc.b	$06,$07
			dc.b	$3F,$0B
			dc.b	$6F,$0D
			dc.b	$7F,$0E
			
			;		9012
			dc.b	$5B,$07
			dc.b	$06,$0B
			dc.b	$3F,$0D
			dc.b	$6F,$0E
			
			;		0123
			dc.b	$4F,$07
			dc.b	$5B,$0B
			dc.b	$06,$0D
			dc.b	$3F,$0E
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
;**************************************************************
;*                 Interrupt Vectors                          *
;**************************************************************
            ORG   $FFFE
            DC.W  Entry           ; Reset Vector
			end
