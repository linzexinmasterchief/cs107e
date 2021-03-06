#include "../assert.h"
#include "../gpio.h"
#include "../timer.h"

void test_gpio_set_get_function(void) {
    gpio_init();

    // Test get pin function (pin2 defaults to input)
    assert( gpio_get_function(GPIO_PIN2) == GPIO_FUNC_INPUT );
    // Test set pin to output
    gpio_set_output(GPIO_PIN2);
    // Test get pin function after setting
    assert( gpio_get_function(GPIO_PIN2) == GPIO_FUNC_OUTPUT );
	
	// Test set pin to input
	gpio_set_input(GPIO_PIN2);
	assert( gpio_get_function(GPIO_PIN2) == GPIO_FUNC_INPUT );

	//Test get/set pin for a different register
	assert( gpio_get_function(GPIO_PIN25) == GPIO_FUNC_INPUT );
	gpio_set_function(GPIO_PIN25, GPIO_FUNC_ALT5);
	assert( gpio_get_function(GPIO_PIN25) == GPIO_FUNC_ALT5 );

	//Test invalid pin number
	assert( gpio_read(55) == -1 );
	gpio_set_input(100);
	gpio_set_output(100);
}

void test_gpio_read_write(void) {
    gpio_init();
    gpio_set_function(GPIO_PIN20, GPIO_FUNC_OUTPUT);

    // Test gpio_write low, then gpio_read
    gpio_write(GPIO_PIN20, 0);
    assert( gpio_read(GPIO_PIN20) ==  0 );

   // Test gpio_write high, then gpio_read
    gpio_write(GPIO_PIN20, 1);
    assert( gpio_read(GPIO_PIN20) ==  1 );
	
	// Test invalid pin number
	assert( gpio_read(55) == -1 );
	gpio_write(100, 1);
}

void test_timer(void) {
    timer_init();

    // Test timer tick count incrementing
    unsigned int start = timer_get_ticks();
    for( int i=0; i<10; i++ ) { /* Spin */ }
    unsigned int finish = timer_get_ticks();
    assert( finish > start );

    // Test timer delay
    //int usecs = 100;
    //start = timer_get_ticks();
    //timer_delay_us(usecs);
    //finish = timer_get_ticks();
    //assert( finish >= start + usecs );
}

// Uncomment each call below when you have implemented the functions
// and are ready to test them

void main(void) {
    test_gpio_set_get_function();
    test_gpio_read_write();
    test_timer();
}
