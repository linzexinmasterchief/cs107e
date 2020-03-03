#include "armtimer.h"
#include "malloc.h"
#include "console.h"
#include "gprof.h"
#include "backtrace.h"
#include "interrupts.h"

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
		num_insn = (__text_start__ - __text_end__) / 4; // 4 bytes per instruction
		counters = (int *)malloc(num_insn); 
	}
	for(int i = 0; i < num_insn; i++) counters[i] = 0; // zero out counters

	armtimer_enable_interrupts();
	on = true;
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

uintptr_t get_offset(uintptr_t addr){
	// loop while looking for name start character. if you get to the end then use 0
	while(((char *)addr)[3] != 0xff && addr > 0) 
		addr -= 4;
	return addr + 4; // first instruction is four bytes over
}

void gprof_dump(void) 
{
    console_printf("     PC     |  COUNT    \n");
    console_printf("************************\n");
	
	int threshold_count = 0; // count of 10th largest instruction

	for(int i = 0, num_marked = 0; i < num_insn; i++){
		if(num_marked < 10 && counters[i] < threshold_count){
			threshold_count = counters[i];
			num_marked++;
		}
		if(num_marked >= 10 && counters[i] > threshold_count){
			threshold_count = counters[i];
			num_marked++;
		}
	}

	for(int i = 0; i < num_insn; i++){
		if(counters[i] >= threshold_count){
			uintptr_t abs_addr = i * 4 + __text_start__;
			uintptr_t offset = get_offset(abs_addr);
			uintptr_t start_addr = abs_addr - offset;

    		console_printf("%p %s+%d: %d\n", (void *)abs_addr, name_of(start_addr), offset, counters[i]); 
		}
	}
}

bool gprof_handler(unsigned int pc) 
{
    if(armtimer_check_and_clear_overflow()){
		counters[pc / 4]++;	
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

