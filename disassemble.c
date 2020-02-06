
#include <stdint.h>
#include "uart.h"
#include "string.h"
#include "printf.h"
#include "printf_internal.h"

static const char* cond[16] = {"eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc", 
                               "hi", "ls", "ge", "lt", "gt", "le", "", ""};
static const char* opcodes[16] = {"and", "eor", "sub", "rsb", "add", "adc", "sbc", "rsc",
                                  "tst", "teq", "cmp", "cmn", "orr", "mov", "bic", "mvn"};
static const char* shift[4] = {"lsl ", "lsr ", "asr ", "ror "};
static const char* reg[16] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "sl", "fp", "ip", "sp", "lr", "pc"};	
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
};
struct branin {
	uint32_t offset: 24;
	uint32_t l:1;
	uint32_t kind:3;
	uint32_t cond:4;
};

static void decode(unsigned int *addr)
{
    struct insn in = *(struct insn *)addr;

	printf("%x: ", *addr);

	if(in.kind == 0b00) { // data processing instructions
		// get operation
		char op[6] = "";
		strlcat(op, opcodes[in.opcode], 3);
		strlcat(op + strlen(op), cond[in.cond], 5);
		if(in.s) strlcat(op, "s", 6);

		if(in.imm) { //immediate mode instruction 
	 		struct iminsn imin = *(struct iminsn *)addr;
	 		unsigned int reg_op2 = imin.imm8 >> (imin.rot << 2);
	 		printf("%s r%d, r%d, #%d\n", op, imin.reg_dst, imin.reg_op1, reg_op2);
	 	} else { // need to load from memory
	 		//decode second operand
	 		char op2[10] = "";
			strlcat(op2, reg[in.reg_op2], 2);
	 		if(in.shift || in.shift_op || in.one) { // translate shifted registeri
				snprintf(op2 + 2, sizeof(op2) - 2, " %s %d", shift[in.shift_op], in.shift);
	 		}

			if(8 <= in.opcode || in.opcode <= 11) { // some opcodes only take 2 operands
				printf("%s r%d, %s\n", op, in.reg_op1, op2);
			} else {
				printf("%s r%d, r%d, %s\n", op, in.reg_dst, in.reg_op1, op2);
			}
	 	}
	} else if (in.kind == 0b01) { // branch or load/store
		// operation
		char op[3] = "b";
		strlcat(op + strlen(op), cond[in.cond], 3);

		struct branin brin = *(struct branin *)addr;
		
		printf("%s %d", op, brin.offset);
	} else { 
		// Might come back to populate this section in the future
	}
}


void main(void) 
{
    uart_init();

    unsigned int start = 0x8000;
	for(unsigned int i = start; i < start + NUM_DECODE; i++){
		decode((void *)i);
	}

    uart_putchar(EOT);
}
