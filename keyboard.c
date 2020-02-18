#include "gpio.h"
#include "printf.h"
#include "gpioextra.h"
#include "keyboard.h"
#include "ps2.h"

static keyboard_modifiers_t mods;
static unsigned int CLK, DATA;

void keyboard_init(unsigned int clock_gpio, unsigned int data_gpio) 
{
    CLK = clock_gpio;
    gpio_set_input(CLK);
    gpio_set_pullup(CLK);
 
    DATA = data_gpio;
    gpio_set_input(DATA);
    gpio_set_pullup(DATA);
}


static int read_bit(void) 
{
	while(!gpio_read(CLK)) {} // wait through low signal
	while(gpio_read(CLK)) {} // wait for falling clock edge
	return gpio_read(DATA);
}

unsigned char keyboard_read_scancode(void) 
{
    while(!read_bit()) {} // wait for start bit
	unsigned char data = 0;
	for(int i = 0; i < 8; i++)
		data |= read_bit() << i;
	read_bit();
	read_bit();
    return data;
}

// unsigned int read_scancode_helper(unsigned char *data){
//     while(!read_bit()) {} // wait for start bit
// 
// 	unsigned int count = 0; // keeps track of parity
// 	for(int i = 0; i < sizeof(*data); i++){
// 		unsigned int next = read_bit(); 
// 		if(next) count++; // increment parity
// 	    data[0] |= next << i; // read (Little Endian) data
// 	}
// 
// 	if(!((count + read_bit()) % 2)) return 0; // check parity
// 	return read_bit(); // check stop bit
// }
// 
// unsigned char keyboard_read_scancode(void) 
// {
// 	unsigned char data[1];
// 	while(read_scancode_helper(data)){}
//     return *data;
// }

key_action_t keyboard_read_sequence(void)
{
    key_action_t action;
    
	unsigned char break_code = keyboard_read_scancode();
	if(break_code != PS2_CODE_RELEASE){
		action.what = KEY_PRESS;
		action.keycode = break_code;
	}
	action.what = KEY_RELEASE;

	unsigned char extend_code = keyboard_read_scancode();
	if(extend_code != PS2_CODE_EXTENDED){
		action.keycode = extend_code;
	}

	action.keycode = keyboard_read_scancode();
    return action;
}

key_event_t keyboard_read_event(void) 
{
    key_event_t event;
	event.action = keyboard_read_sequence();	
	event.key = ps2_keys[event.action.keycode];

	// Handle conditional modifiers
	if(event.action.keycode == PS2_KEY_SHIFT){
		if(event.action.what == KEY_PRESS) mods |= KEYBOARD_MOD_SHIFT; // turn bit on
		else mods &= ~KEYBOARD_MOD_SHIFT; // turn bit off
	}
	if(event.action.keycode == PS2_KEY_ALT){
		if(event.action.what == KEY_PRESS) mods |= KEYBOARD_MOD_ALT;
		else mods &= ~KEYBOARD_MOD_ALT;
	}
	if(event.action.keycode == PS2_KEY_CTRL){
		if(event.action.what == KEY_PRESS) mods |= KEYBOARD_MOD_CTRL;
		else mods &= ~KEYBOARD_MOD_CTRL;
	}

	//Handle stick modifiers
	if(event.action.keycode == PS2_KEY_CAPS_LOCK && event.action.what == KEY_PRESS){
		mods ^= KEYBOARD_MOD_CAPS_LOCK; // toggle bit	
	}	
	if(event.action.keycode == PS2_KEY_SCROLL_LOCK && event.action.what == KEY_PRESS){
		mods ^= KEYBOARD_MOD_SCROLL_LOCK;
	}	
	if(event.action.keycode == PS2_KEY_NUM_LOCK && event.action.what == KEY_PRESS){
		mods ^= KEYBOARD_MOD_NUM_LOCK;
	}	

	event.modifiers = mods;	
    return event;
}


int is_letter(char ch){
	return 'a' <= ch && ch <= 'z';
}

unsigned char keyboard_read_next(void) 
{
	key_event_t event = keyboard_read_event();
	if(event.key.other_ch == 0) return event.key.ch;
    if(mods & KEYBOARD_MOD_SHIFT) return event.key.other_ch;
	if(mods & KEYBOARD_MOD_CAPS_LOCK && is_letter(event.key.ch)) return event.key.other_ch;
	return '!';
}
