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

/**
* Establishes correspondence between segments and GPIO pins
*/
enum segments{
	A = GPIO_PIN20,
	B = GPIO_PIN21,
	C = GPIO_PIN22,
	D = GPIO_PIN23,
	E = GPIO_PIN24,
	F = GPIO_PIN25,
	G = GPIO_PIN26
};

/**
* Establishes correspondence between dgits and GPIO pins
*/
enum digits{
	FIRST = GPIO_PIN10,
	SECOND = GPIO_PIN11,
	THIRD = GPIO_PIN12,
	FOURTH = GPIO_PIN13
};

unsigned int DELAY = 2500; // refresh rate
unsigned int INTERVAL = 1000000; // length of "second"

int BUTTON = GPIO_PIN2; // button to start timer

/**
* Sets all segment pins as output and the start button as input
*/
void setup_pins(){
	for(volatile unsigned int i=A; i <= G; i++){
		gpio_set_output(i);
	}
	for(volatile unsigned int i=FIRST; i <= FOURTH; i++){
		gpio_set_output(i);
	}

	gpio_set_input(BUTTON);
}

/**
* Writes the starting pattern to the display
*/
void init_pins(){
	for(int i=FIRST; i <= FOURTH; i++){
		gpio_write(i, 1);
		gpio_write(G, 1);
	}
}

/**
* Writes a hexadecimal value (0-F) to a specific digit
*/
void write_digit(unsigned int digit, unsigned int value){
	gpio_write(digit, 1);

	for(volatile int seg_counter=A; seg_counter <= G; seg_counter++){
		gpio_write(seg_counter, value & 1);
		value >>= 1;
	}
}

/**
* Runs a display with a refresh rate of DELAY incrementing at a rate of INTERVAL
*/
void run(){
	int curr_sec = 0; // current second value being displayed
	int curr_min = 0; // current minute value being displayed
	volatile unsigned int curr_tick = timer_get_ticks(); // time at last update

	while(1){
		//Update each indivdual digit
		write_digit(FIRST, values[(curr_min / 10) % 10]);
	    timer_delay_us(DELAY);
	    gpio_write(FIRST, 0);

		write_digit(SECOND, values[curr_min  % 10]);
		timer_delay_us(DELAY);
		gpio_write(SECOND, 0);

		write_digit(THIRD, values[(curr_sec / 10)  % 10]);
		timer_delay_us(DELAY);
		gpio_write(THIRD, 0);

		write_digit(FOURTH, values[curr_sec % 10]);
		timer_delay_us(DELAY);
		gpio_write(FOURTH, 0);
	
		// Update displayed time if interval has elapsed
		if((timer_get_ticks() - curr_tick) >= INTERVAL){
			curr_sec += 1;
			// extra logic to display seconds/minutes
			if(curr_sec >= 60){
				curr_sec = 0;
				curr_min += 1;
			}

			curr_tick = timer_get_ticks();
		}
	}	
}

void main(void)
{
	// Puts the display in standby
	setup_pins();
	init_pins();

	// Waits until the button is pressed
	while(gpio_read(BUTTON)) { /* spin */ }
	
	run();

}
