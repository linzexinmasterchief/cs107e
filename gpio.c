#include "gpio.h"

static unsigned int *FSEL0 = (unsigned int *)0x20200000;
static unsigned int *FSEL5 = (unsigned int *)0x20200014;
static unsigned int * SET0 = (unsigned int *)0x2020001C;



void gpio_init(void) {
}

/*
* Convert a binary-encoded value to a decimal value
*/
int binary_to_decimal(int n) {
	int dec_value = 0;
	
	int temp = n;
	int base = 1; // current place value
	while(temp > 0){
		int last_digit = temp % 10; 
		temp = temp / 10;
		dec_value += last_digit * base;
		base = base * 2;
	}

	return dec_value;
}

void gpio_set_function(unsigned int pin, unsigned int function) {
	if(pin < GPIO_PIN_FIRST || pin > GPIO_PIN_LAST) return;
  
	int num_register = pin / 10;
    unsigned int *currReg = (FSEL0 + num_register); // correct function register depends on MSB
    int fcn = function << 3 * (pin % 10); // 3 bits per function register
	*currReg = fcn;
}

unsigned int gpio_get_function(unsigned int pin) {    
	if(pin < GPIO_PIN_FIRST || pin > GPIO_PIN_LAST) return GPIO_INVALID_REQUEST;
	
	// isolate the correct register
	int num_register = pin / 10;
    unsigned int *curr_reg = (FSEL0 + num_register);
	
	// isolate correct pin
	unsigned int filter = 111 << (3 * (pin % 10));
    filter = filter & *curr_reg;
	filter = filter >> (3 * (pin % 10));

	return binary_to_decimal(filter);
}

void gpio_set_input(unsigned int pin) {
	gpio_set_function(pin, GPIO_FUNC_INPUT); 
}

void gpio_set_output(unsigned int pin) {
	gpio_set_function(pin, GPIO_FUNC_OUTPUT);
}

void gpio_write(unsigned int pin, unsigned int value) {
    // TODO: Your code goes here.
}

unsigned int gpio_read(unsigned int pin) {
    return 0;  // TODO: Your code goes here.
}
