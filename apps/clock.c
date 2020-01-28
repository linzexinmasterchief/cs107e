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
	0b00000110,
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
	0b01110001,
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

unsigned int DELAY = 2500;
unsigned int INTERVAL = 1000000;

int BUTTON = GPIO_PIN2;

void setup_pins(){
	for(volatile unsigned int i=A; i <= G; i++){
		gpio_set_output(i);
	}
	for(volatile unsigned int i=FIRST; i <= FOURTH; i++){
		gpio_set_output(i);
	}

	gpio_set_input(BUTTON);
}

void write_digit(unsigned int digit, unsigned int value){
	gpio_write(digit, 1);

	for(volatile int seg_counter=A; seg_counter <= G; seg_counter++){
		gpio_write(seg_counter, value & 1);
		value >>= 1;
	}
}

void init_pins(){
	for(int i=FIRST; i <= FOURTH; i++){
		gpio_write(i, 1);
		gpio_write(G, 1);
	}
}

void main(void)
{
	setup_pins();
	init_pins();

	while(gpio_read(BUTTON)) { /* spin */ }
	
	int curr_num = 0;
	unsigned int curr_tick = timer_get_ticks();	
	
	while(1){
		write_digit(FOURTH, values[ curr_num         % 10]);
	    timer_delay_us(DELAY);
	    gpio_write(FIRST + 3, 0);

		write_digit(THIRD, values[(curr_num / 10)   % 10]);
		timer_delay_us(DELAY);
		gpio_write(FIRST + 2, 0);

		write_digit(SECOND, values[(curr_num / 100)  % 10]);
		timer_delay_us(DELAY);
		gpio_write(FIRST + 1, 0);

		write_digit(FIRST, values[(curr_num / 1000) % 10]);
		timer_delay_us(DELAY);
		gpio_write(FIRST + 0, 0);
	
		if(timer_get_ticks() - curr_tick > INTERVAL){
			curr_num = (curr_num + 1) % 10000;
			curr_tick = timer_get_ticks();
		}
  
	}	

}
