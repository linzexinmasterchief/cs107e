#include "backtrace.h"
#include "printf.h"

const char *name_of(uintptr_t fn_start_addr)
{
	char *name_ptr = (char *)(fn_start_addr - 4);
	if(name_ptr[3] == 0xff) {
		int length = name_ptr[0]; // TODO: what if name is bigger than 255
		return name_ptr - length;
	}	
    return "???";
}

int backtrace (frame_t f[], int max_frames)
{
	// get the backtrace frame pointer
	uintptr_t *cur_fp;
	__asm__("mov %0, fp" : "=r" (cur_fp));
	// get the bactrace caller pc
	uintptr_t *cur_pc;

	int count = 0;
	for(; cur_fp[-3] > 0; cur_fp = (uintptr_t *)cur_fp[-3]){ // loops over frame pointers
		if(count >= max_frames) break;		
		
		// function starts 3 instructions before pc says	
		cur_pc = (uintptr_t *)cur_fp[-3];
		uintptr_t start = *cur_pc - 12; 
		
		// populate the current frame
		f[count].name = name_of(start);
		f[count].resume_addr = cur_fp[-1]; // dereference lr
		f[count].resume_offset = f[count].resume_addr - start;	

		count++;
	}
    return count;
}

void print_frames (frame_t f[], int n)
{
    for (int i = 0; i < n; i++)
        printf("#%d 0x%x at %s+%d\n", i, f[i].resume_addr, f[i].name, f[i].resume_offset);
}

void print_backtrace (void)
{
    int max = 50;
    frame_t arr[max];

    int n = backtrace(arr, max);
    print_frames(arr+1, n-1);   // print frames starting at this function's caller
}
