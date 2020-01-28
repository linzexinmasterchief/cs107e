#include "../gpio.h"
#include "../timer.h"

/**
* This array establishes a correspondence between the first 16 
* hexadecimal digits and their corresponding binary values on
* a 7 segment display. segments[0] = 0, segments[1] = 1, all 
* the way up to segment[15] = F
*/

volatile unsigned int values[16] = {
	0b00111111, 
	0b00110000,
	0b01011011,
	0b01001111,
	0b01100110,
	0b01101101,
	0b01111101,
	0b00000111,
	0b01111111,
	0b01100111,
	0b01110111,
	0b01111111,
	0b00111001,
	0b00111111,
	0b01111001,
	0b01110001	
};

enum segments{
	A = GPIO_PIN20,
	B = GPIO_PIN21,
	C = GPIO_PIN22,
	D = GPIO_PIN23,
	E = GPIO_PIN24,
	F = GPIO_PIN25,
	G = GPIO_PIN26
};

enum digits{
	FIRST = GPIO_PIN10,
	SECOND = GPIO_PIN11,
	THIRD = GPIO_PIN12,
	FOURTH = GPIO_PIN13
};

void setup_pins(){
	for(volatile unsigned int i=A; i <= G; i++){
		gpio_set_output(i);
	}
	for(volatile unsigned int i=FIRST; i <= FOURTH; i++){
		gpio_set_output(i);
	}
}

void write_digit(unsigned int digit, unsigned int value){
	gpio_write(FIRST + digit, 1);

	volatile unsigned int seg_counter = A;	
	while(value){
		if(value & 1) {
			gpio_write(seg_counter, 1);
		} else{
			gpio_write(seg_counter, 0);
		}

		seg_counter++;
		value >>= 1;
	}
}

void main(void)
{
	setup_pins();
	// gpio_write(FIRST, 1);
	// gpio_write(B, 1);
	// gpio_write(C, 1);
	write_digit(1, values[15]);
}
