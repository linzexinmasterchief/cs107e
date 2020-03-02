#include "gpio.h"
#include "printf.h"
#include "gpioextra.h"
#include "keyboard.h"
#include "ps2.h"
#include "timer.h"
#include "interrupts.h"
#include "uart.h"
#include "ringbuffer.h"

static keyboard_modifiers_t mods;
static unsigned int CLK, DATA;
static const unsigned int MAX_DELAY = 3000;

// made vars global for interrupts
static int bitcount = 0; // number of bits read thus far
static unsigned int start_time; // time of last start code
static unsigned int parity_count = 0; // current parity on data bits
static unsigned char curr_code = 0; // scancode being currently parsed
static rb_t *codes; // queue housing all scancodes

/*
* Event handler for falling clock edge on PS2 keyboard
*/
void clock_edge(){
	if(gpio_check_and_clear_event(CLK)){
		if(bitcount == 0) { // Read start bit (low)
			if(!gpio_read(DATA)) { 
				bitcount++; 
				start_time = timer_get_ticks(); // account for delay between bits
			}
			return;
		}
	
		if(bitcount < 9){ // Read data bits
			if(timer_get_ticks() - start_time > MAX_DELAY){ // handle dropped bits
				curr_code = 0;
				bitcount = 0;
			} else{
				unsigned int next = gpio_read(DATA);	
				parity_count += next; // update current code parity
		    	curr_code |= (next << (bitcount - 1)); // read (Little Endian) data
				bitcount++;
			}
			return;
		}

		if(bitcount == 9){ // Read parity bit
			if((parity_count + gpio_read(DATA)) % 2 == 0) { // using odd parity
				curr_code = 0;
				bitcount = 0;
			} else{
				bitcount++;
			}
			return;
		}

		if(gpio_read(DATA)){ // Read stop bit (high)
		    if(!rb_enqueue(codes, curr_code)) uart_putchar('X');
		}
		
		// Get ready for next character
		curr_code = 0;
		bitcount = 0; 
	}
}

static int read_bit(void) 
{
	while(!gpio_read(CLK)) {} // wait through low signal
	while(gpio_read(CLK)) {} // wait for falling clock edge
	return gpio_read(DATA);
}

unsigned char keyboard_read_scancode(void)
{
	unsigned char data = 0;
	unsigned int start = 1;
	while(1){
		while(start) start = read_bit();
		unsigned int start_time = timer_get_ticks();
	
		unsigned int count = 0; // keeps track of parity
		for(int i = 0; i < 8; i++){
			unsigned int next = read_bit(); 
			if(timer_get_ticks() - start_time > MAX_DELAY){ // handle dropped bits
				start = next;
				continue;
			}
			count += next;
		    data |= (next << i); // read (Little Endian) data
		}

		if((count + read_bit()) % 2 == 0) continue; // checks parity
		if(read_bit()) return data; // checks stop bit
	}
}

key_action_t keyboard_read_sequence(void)
{
    key_action_t action;
	int int_code;
    while(!rb_dequeue(codes, &int_code)) {/* spin */}
	unsigned char first_code = (unsigned char)int_code;

	if(first_code == PS2_CODE_EXTENDED){ // ignore extended keys
		first_code = keyboard_read_scancode(); 
	}

	if(first_code != PS2_CODE_RELEASE){ // if event was a press
		action.what = KEY_PRESS;
		action.keycode = first_code;
		return action;
	}

	action.what = KEY_RELEASE; // if event was a release
	action.keycode = keyboard_read_scancode();
    return action;
}

key_event_t keyboard_read_event(void) 
{
    key_event_t event;
	event.action = keyboard_read_sequence();	
	event.key = ps2_keys[event.action.keycode];

	// Handle conditional modifiers
	if(event.key.ch == PS2_KEY_SHIFT){
		if(event.action.what == KEY_PRESS) mods |= KEYBOARD_MOD_SHIFT; // turn bit on
		else mods &= ~KEYBOARD_MOD_SHIFT; // turn bit off
	}
	if(event.key.ch == PS2_KEY_ALT){
		if(event.action.what == KEY_PRESS) mods |= KEYBOARD_MOD_ALT;
		else mods &= ~KEYBOARD_MOD_ALT;
	}
	if(event.key.ch == PS2_KEY_CTRL){
		if(event.action.what == KEY_PRESS) mods |= KEYBOARD_MOD_CTRL;
		else mods &= ~KEYBOARD_MOD_CTRL;
	}

	//Handle stick modifiers
	if(event.key.ch == PS2_KEY_CAPS_LOCK && event.action.what == KEY_PRESS){
		mods ^= KEYBOARD_MOD_CAPS_LOCK; // toggle bit	
	}	
	if(event.key.ch == PS2_KEY_SCROLL_LOCK && event.action.what == KEY_PRESS){
		mods ^= KEYBOARD_MOD_SCROLL_LOCK;
	}	
	if(event.key.ch == PS2_KEY_NUM_LOCK && event.action.what == KEY_PRESS){
		mods ^= KEYBOARD_MOD_NUM_LOCK;
	}	

	event.modifiers = mods;	
    return event;
}


int is_letter(char ch){
	return 'a' <= ch && ch <= 'z';
}

static const unsigned char MODS[] = {PS2_KEY_SHIFT, PS2_KEY_ALT, PS2_KEY_CTRL, PS2_KEY_CAPS_LOCK, PS2_KEY_SCROLL_LOCK, PS2_KEY_NUM_LOCK};
static const int NUM_MODS = 6;

unsigned char keyboard_read_next(void) 
{
	key_event_t event = keyboard_read_event();
	if(event.action.what == KEY_RELEASE) return keyboard_read_next(); // ignore key releases
	for(int i = 0; i < NUM_MODS; i++){ // ignore modifiers
		if(event.key.ch == MODS[i])
			return keyboard_read_next();
	}
	if(event.key.other_ch == 0) return event.key.ch; // just return if there's no other option
    if(mods & KEYBOARD_MOD_SHIFT) return event.key.other_ch; // shift takes precedence
	if(mods & KEYBOARD_MOD_CAPS_LOCK && is_letter(event.key.ch)) return event.key.other_ch; // finally caps lock
	return event.key.ch;
}

void keyboard_init(unsigned int clock_gpio, unsigned int data_gpio) 
{
    CLK = clock_gpio;
    gpio_set_input(CLK);
    gpio_set_pullup(CLK);
	gpio_enable_event_detection(CLK, GPIO_DETECT_FALLING_EDGE); 
	interrupts_attach_handler((handler_fn_t)clock_edge, INTERRUPTS_GPIO3);

    DATA = data_gpio;
    gpio_set_input(DATA);
    gpio_set_pullup(DATA);

	codes = rb_new();
}

