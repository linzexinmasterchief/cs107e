#include "gpio.h"

static volatile unsigned int *FSEL0 = (unsigned int *)0x20200000;
static volatile unsigned int * CLR0 = (unsigned int *)0x20200028;
static volatile unsigned int * SET0 = (unsigned int *)0x2020001C;
static volatile unsigned int * LEV0 = (unsigned int *)0x20200034;

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
	// Error checking
	if(pin < GPIO_PIN_FIRST || pin > GPIO_PIN_LAST) return; // valid pin
  	if((function & 000) != 0) return; // valid function
	
	int num_register = pin / 10;
    unsigned volatile int *currReg = FSEL0 + num_register; // correct function register depends on MSB
    int fcn = function << (3 * (pin % 10)); // 3 bits per function register
	*currReg = fcn;
}

unsigned int gpio_get_function(unsigned int pin) {    
	// Error checking
	if(pin < GPIO_PIN_FIRST || pin > GPIO_PIN_LAST) return GPIO_INVALID_REQUEST; // handles invalid pin
	
	// isolate the correct register
    unsigned volatile int *curr_reg = (FSEL0 + pin / 10);
	
	// isolate correct pin
	unsigned int filter = 111 << (3 * (pin % 10)); // 3 bits per function register
 	filter &= *curr_reg;
	filter >>= (3 * (pin % 10));

	return binary_to_decimal(filter);
}

void gpio_set_input(unsigned int pin) {
	gpio_set_function(pin, GPIO_FUNC_INPUT); 
}

void gpio_set_output(unsigned int pin) {
	gpio_set_function(pin, GPIO_FUNC_OUTPUT);
}

void gpio_write(unsigned int pin, unsigned int value) {
	// Error checking
	if(pin < GPIO_PIN_FIRST || pin > GPIO_PIN_LAST) return; // handles invalid pin
	if((pin & 0) != 0) return; // invalid value	

	volatile unsigned int *update_pointer = value ? SET0 : CLR0; // use SET0 if value=1 and CLR0 otherwise  
	if(pin < GPIO_PIN32) {
		*update_pointer = 1 << pin;
	} else {
		*(update_pointer + 1) = 1 << (pin - 32); // shifts to next register if needed
	}
}

unsigned int gpio_read(unsigned int pin) {
	// Error checking
	if(pin < GPIO_PIN_FIRST || pin > GPIO_PIN_LAST) return GPIO_INVALID_REQUEST; // handles invalid pin

	unsigned int filter;
	if(pin < GPIO_PIN32) {
		filter = 1 << pin;
		filter &= *LEV0;
	} else {
		filter = 1 << (pin - 32);
		filter &= *(LEV0 + 1); // shifts to next register if needed
	}				                                                           		
	
	filter >>= pin;
	return binary_to_decimal(filter);
}   				                                                           		
