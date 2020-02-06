
#include <stdint.h>
#include "uart.h"
#include "string.h"
#include "printf.h"
#include "printf_internal.h"

static const char *cond[16] = {"eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc", 
                               "hi", "ls", "ge", "lt", "gt", "le", "", ""};
static const char *opcodes[16] = {"and", "eor", "sub", "rsb", "add", "adc", "sbc", "rsc",
                                  "tst", "teq", "cmp", "cmn", "orr", "mov", "bic", "mvn"};
static const char *shift[4] = {", lsl", ", lsr", ", asr", ", ror"};
static const cahr *reg[16] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "sl", "fp", "ip", "sp", "lr", "pc"};	
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
struct iminsn {
	uint32_t imm8:8;
	uint32_t rot:4;
    uint32_t reg_dst:4;
    uint32_t reg_op1:4; 
    uint32_t s:1;
    uint32_t opcode:4;
    uint32_t imm:1;
    uint32_t kind:2;
    uint32_t cond:4;
}

static void decode(unsigned int *addr)
{
    struct insn in = *(struct insn *)addr;

	//operation
	char op[6] = opcodes[in.opcode];
	strlcat(op, cond[in.cond];
	if(s) strlcat(op, 's');

	if(kind == 0b00) { // data processing instructions
		if(imm) { //immediate mode instruction 
			struct iminsn imin = *(struct iminsn *)addr;
			uint32_t reg_op2 = imin.imm8 >> (imin.rot << 2);
			printf()
		} else { // need to load from memory
			//decode second operand
			str[10] op2 = reg[reg_op2];
			if(in.shift || in.shift_op || in.one)) { //
				strlcat(op2 + strlen(op2), shift[shift_op]);
			}

			if(in.one){ // register
				uint32_t shift_amt = 
			} else{ // register shifted register
				switch (shift_op){
					printf(*addr);	
				}
			}
		}
	} else if (kind == 0b01) { // branch or store/load

	} else { // no clue
		printf(*addr);
	}

	printf("%p%c %s %x %x %x", addr, ':', opcodes[in.opcode], in.reg_dst, in.reg_op1, in.reg_op2);
    printf("opcode is %s, s is %d, reg_dst is r%d\n", opcodes[in.opcode], in.s, in.reg_dst);
}


void main(void) 
{
    uart_init();

    unsigned int start = 0x8000;
	for(unsigned int i = start; i < start + NUM_DECODE; i++){
		decode(i);
	}

    uart_putchar(EOT);
}
