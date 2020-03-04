#include "shell.h"
#include "shell_commands.h"
#include "uart.h"
#include "keyboard.h"
#include "malloc.h"
#include "strings.h"
#include "pi.h"
#include "gprof.h"

#define LINE_LEN 80

static formatted_fn_t shell_printf;

/*
 * Returns a pointer to a new null-terminated string containing at most `n`
 * bytes copied from the string pointed to by `src`.
 *
 * Example: strndup("cs107e", 4) == "cs10"
 */
static char *strndup(const char *src, size_t n)
{
	char *dst = malloc(n + 1);
	dst[0] = '\0';
	strlcat(dst, src, n + 1);
	return dst;
}

// From Lab 4
static bool isspace(char ch)
{
    return ch == ' ' || ch == '\t' || ch == '\n';
}

// From Lab 4
static int tokenize(const char *line, char *array[],  int max)
{
    int ntokens = 0;
    const char *cur = line;

    while (ntokens < max) {
        while (isspace(*cur)) cur++;    // skip spaces (stop non-space/null)
        if (*cur == '\0') break;        // no more non-space chars
        const char *start = cur;
        while (*cur != '\0' && !isspace(*cur)) cur++; // advance to end (stop space/null)
        array[ntokens++] = strndup(start, cur - start);   // make heap-copy, add to array
    }
    return ntokens;
}

static const int NUM_COMMANDS = 6;
static const command_t commands[] = {
    {"help",   "<cmd> prints a list of commands or description of cmd", cmd_help},
    {"echo",   "<...> echos the user input to the screen", cmd_echo},
	{"reboot", "reboot the Raspberry Pi back to the bootloader using `pi_reboot", cmd_reboot},
	{"peek", "<address> prints the contents (4 bytes) of memory at address", cmd_peek},
	{"poke", "<address> <value> stores `value` into the memory at `address`", cmd_poke},
	{"profile", "[on | off | status | results] tool that tracks where in the code a program is spending its execution time", cmd_profile}
};

int cmd_echo(int argc, const char *argv[]) 
{
    for (int i = 1; i < argc; i++) 
        shell_printf("%s ", argv[i]);
    shell_printf("\n");
    return 0;
}

int cmd_help(int argc, const char *argv[]) 
{
    if(argc == 1){ // input: "help"
		for(int i = 0; i < NUM_COMMANDS; i++){
			shell_printf("%s: %s\n", commands[i].name, commands[i].description);
		}
		return 0;
	} else{ // input: "help [cmd]"
		for(int i = 0; i < NUM_COMMANDS; i++){
			if(strcmp(commands[i].name, argv[1]) == 0){
				shell_printf("%s: %s\n", commands[i].name, commands[i].description);
				return 0;
			}
		}
	}
	shell_printf("error: no such command '%s'", argv[1]);
    return -2;
}

int cmd_reboot(int argc, const char *argv[]){
	uart_putchar(EOT);
	pi_reboot();
	return 0;
}

int to_num(const char *arg){
	int addr = strtonum(arg, NULL);
	if(addr == 0){
		shell_printf("error: peek cannot convert '%s'\n", arg);
		return -4;
	}

	if(addr % 4 != 0){
		shell_printf("error: peek address must be 4-byte aligned\n");
		return -5;
	}

	return addr;
}

int cmd_peek(int argc, const char *argv[]){
	if(argc != 2){
		shell_printf("error: peek expects 1 argument [address]\n");
		return -3;
	}
	
	int addr = to_num(argv[1]);
	if(addr < 0) return addr;

	shell_printf("%p: %08d\n", (void *)addr, *(unsigned int *)addr);
	return 0;
}

int cmd_poke(int argc, const char *argv[]) {
	if(argc != 3){
		shell_printf("error: poke expects 2 argument [address] [value]\n");
		return -6;
	}
	
	int addr = to_num(argv[1]);
	if(addr < 0) return addr;
	
	int val = strtonum(argv[2], NULL);
	if(val == 0){
		shell_printf("error: peek cannot convert '%s'\n", argv[2]);
		return -4;
	}

	*(unsigned int *)addr = val;
	return 0;
}

int cmd_profile(int argc, const char *argv[]) {
	if(argc != 2){
		shell_printf("error: requires additional arguments [on | off | status | results]\n");
		return -7;
	}

	if(strcmp(argv[1], "on") == 0){
		gprof_on();
		return 0;
	}

	if(strcmp(argv[1], "off") == 0){
		gprof_off();
		return 0;
	} 

	if(strcmp(argv[1], "status") == 0){
		if(gprof_is_active()){
			shell_printf("Status: on\n");
		} else {
			shell_printf("Status: off\n");
		}
		return 0;
	}	

	if(strcmp(argv[1], "results") == 0){
		gprof_dump(); // prints to console
		return 0;
	}

	return -8;
}

void shell_init(formatted_fn_t print_fn)
{
    shell_printf = print_fn;
}

void shell_bell(void)
{
    uart_putchar('\a');
}

void shell_readline(char buf[], size_t bufsize)
{
	int line_size = 0;
    while(1){
		char next = keyboard_read_next();

		if(next == '\n'){
			shell_printf("\n");
			buf[line_size] = '\0';
			break;
		} else if(next == '\b'){
			if(line_size == 0) {
				shell_bell();
			}
			else{
				line_size--;

				shell_printf("%c", '\b');
				shell_printf(" ");
				shell_printf("%c", '\b');
			}	
		} else if(line_size >= bufsize - 1){ // leaves space for null terminator
			shell_bell();
		} else{
			shell_printf("%c", next);
			buf[line_size] = next;
			line_size++;
		}
	}
	buf[line_size] = '\0';
}

int shell_evaluate(const char *line)
{
	int max = strlen(line);
	char *arr[max];
	int n_tokens = tokenize(line, arr, max); 
	
	for(int i = 0; i < NUM_COMMANDS; i++){
		if(strcmp(commands[i].name, arr[0]) == 0){
			return commands[i].fn(n_tokens, (const char **)arr);
		}
	}
	
	shell_printf("error: no such command %s\n", arr[0]);
	return -1;
}

void shell_run(void)
{
    shell_printf("Welcome to the CS107E shell. Remember to type on your PS/2 keyboard!\n");
    while (1) 
    {
        char line[LINE_LEN];

        shell_printf("Pi> ");
        shell_readline(line, sizeof(line));
        shell_evaluate(line);
    }
}
