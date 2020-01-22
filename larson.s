/*
 * LARSON SCANNER
 *
 * Currently this code is the blink code copied from lab 1.
 *
 * Modify the code to implement the larson scanner for assignment 1.
 *
 * Make sure to use GPIO pins 20-23 (or 20-27) for your scanner.
 */

.equ DELAY, 0x3F0000 // duration of pulse
.equ NUMJMPS, 3 // the number of LEDS minus 1

// configure GPIO 20-27 for output
ldr r0, FSEL2
mov r1, #1 // output for 20
orr r1, r1, #(1<<3)
orr r1, r1, #(1<<6)
orr r1, r1, #(1<<9) // output for 23
// orr r1, r1, #(1<<12)
// orr r1, r1, #(1<<15)
// orr r1, r1, #(1<<18)
// orr r1, r1, #(1<<22) // output for 27
str r1, [r0] // set value in register

// set bit 20 and 21
mov r1, #(0b0001<<20)

cycle:
	// decide led shift direction
	cmp r1, #(0b0001<<20)
	mov r3, #NUMJMPS
	beq left

	right:
		// load new scanner pattern
		lsr r1, r1, #1 
		ldr r0, SET0
		str r1, [r0] 

		// delay
		mov r2, #DELAY
		wait1:
			subs r2, #1
			bne wait1

		// set GPIO 20-23 low
		ldr r0, CLR0
		str r1, [r0] 

		subs r3, #1
		bne right
	b cycle

	left:
		// load new scanner pattern
		lsl r1, r1, #1 
		ldr r0, SET0
		str r1, [r0] 

		// delay
		mov r2, #DELAY
		wait2:
			subs r2, #1
			bne wait2

		// set GPIO 20-23 low
		ldr r0, CLR0
		str r1, [r0] 

		subs r3, #1
		bne left
	b cycle


FSEL0: .word 0x20200000
FSEL1: .word 0x20200004
FSEL2: .word 0x20200008
SET0:  .word 0x2020001C
SET1:  .word 0x20200020
CLR0:  .word 0x20200028
CLR1:  .word 0x2020002C

