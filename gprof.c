#include "armtimer.h"
#include "malloc.h"
#include "console.h"
#include "gprof.h"
#include "backtrace.h"
#include "interrupts.h"
#include "printf.h"
#include "uart.h"

extern int __text_end__;
static const int __text_start__ = 0x8000;
/* Interval for profiler timer interrupts */
#define GPROF_TIMER_INTERVAL 0x10
static int *counters;
static uintptr_t num_insn = -1;
static bool on = false;

void gprof_on(void) 
{
	if(num_insn == -1){
		num_insn = (__text_end__ - __text_start__) / 4; // 4 bytes per instruction
		counters = (int *)malloc(num_insn); 
	}
	uart_putchar('x');
	printf("%x - %x = 4 * %x", __text_end__, __text_start__, num_insn);
	for(int i = 0; i < num_insn; i++) {
		counters[i] = 0; // zero out counters
	}
	on = true;
	armtimer_enable_interrupts();
}

void gprof_off(void) 
{
	armtimer_disable_interrupts();
	on = false;
}

bool gprof_is_active(void) 
{
    return on;
}

/*
* Returns the start address of a function 
* Expects that the -mpoke-function-name flag was turned on
* Defaults to 0x8000 or nearest named function if function not named
*/
uintptr_t get_start(uintptr_t addr){
	// loop while looking for name start character. if you get to the end then use 0
	while(((char *)addr)[3] != 0xff && addr > (__text_start__ - 4)) 
		addr -= 4;
	return addr + 4; // first instruction is four bytes over
}

void gprof_dump(void) 
{
	// Added support for both console and shell printf
    printf("     PC     |  COUNT    \n");
    printf("************************\n");
    console_printf("     PC     |  COUNT    \n");
    console_printf("************************\n");
	
	
	int top_ten[10] = {0}; // will store indices of 10 highest instructions in counter
	for(int i = 0, j = 0; i < num_insn; i++, j = 0){
		while(counters[i] > counters[top_ten[j]] && j < 10) j++;

		// swap highest element into place
		int to_swap = i;
		while(j > 0){
			int temp = top_ten[j - 1];
			top_ten[j - 1] = to_swap;
			to_swap = temp;

			j--;
		}
	}

	// prints a statement for each of the highest values
	for(int i = 0; i < 9; i++){
		uintptr_t abs_addr = top_ten[i] * 4 + __text_start__;
		uintptr_t start_addr = get_start(abs_addr);
		uintptr_t offset = abs_addr - start_addr;

    	printf("%p %s+%d: %d\n", (void *)abs_addr, name_of(start_addr), offset, counters[top_ten[i]]); 
    	console_printf("%p %s+%d: %d\n", (void *)abs_addr, name_of(start_addr), offset, counters[top_ten[i]]); 
	}
}

static bool gprof_handler(unsigned int pc) 
{
    if(armtimer_check_and_clear_interrupt()){
		counters[(pc - __text_start__) / 4]++;	
		return true;
	}
    return false;
}

void gprof_init(void) 
{ 
	armtimer_init(GPROF_TIMER_INTERVAL);
	armtimer_enable();
	interrupts_attach_handler(gprof_handler, INTERRUPTS_BASIC_ARM_TIMER_IRQ);
}

