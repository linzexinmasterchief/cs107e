#include "printf.h"
#include <stdint.h>
#include "uart.h"


static const char *cond[16] = {"eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc", 
                               "hi", "ls", "ge", "lt", "gt", "le", "", ""};
static const char *opcodes[16] = {"and", "eor", "sub", "rsb", "add", "adc", "sbc", "rsc",
                                  "tst", "teq", "cmp", "cmn", "orr", "mov", "bic", "mvn"};

const int NUM_DECODE = 400;
/* 
 * This bitfield is declared using exact same layout as bits are organized in
 * the encoded instruction. Accessing struct.field will extract just the bits
 * apportioned to that field. If you look at the assembly the compiler generates
 * to access a bitfield, you will see it simply masks/shifts for you. Neat!
 */

struct insn  {
    uint32_t reg_op2:4;
    uint32_t one:1;
    uint32_t shift_op: 2;
    uint32_t shift: 5;
    uint32_t reg_dst:4;
    uint32_t reg_op1:4; 
    uint32_t s:1;
    uint32_t opcode:4;
    uint32_t imm:1;
    uint32_t kind:2;
    uint32_t cond:4;
};

static void decode(unsigned int *addr)
{
    struct insn in = *(struct insn *)addr;
	printf("%p%c %s %x %x %x", addr, ':', opcodes[in.opcode], in.reg_dst, in.reg_op1, in.reg_op2);
    printf("opcode is %s, s is %d, reg_dst is r%d\n", opcodes[in.opcode], in.s, in.reg_dst);
}


void main(void) 
{
    uart_init();

    unsigned int start = 0x8000;
	for(unsigned int i = start; i < start + NUM_DECODE; i += 4){
		decode(&i);
	}

    uart_putchar(EOT);
}
