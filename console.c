#include "console.h"
#include "malloc.h"
#include "printf.h"
#include "gl.h"
#include "strings.h"

static char **contents;
static int curr_x, curr_y;
static int MAX_ROWS, MAX_COLS;
static const color_t CON_COLOR = GL_GREEN;

void console_init(unsigned int nrows, unsigned int ncols)
{
	contents = malloc(nrows * ncols);
	curr_x = 0;
	curr_y = 0;
	MAX_ROWS = nrows;
	MAX_COLS = ncols;
}

/**
* Move the cursor forward by amt taking into account the wrapping schemes
*/
static void increment(int amt){
	if(curr_x + amt < 0){ // backspace wrapping
		curr_x = MAX_ROWS - (curr_x + amt);
		if(curr_y > 0) curr_y--;
	} else {
		if(curr_x + amt > MAX_ROWS && curr_y == MAX_COLS - 1){ // vertical wrapping
			for(int r = 0; r < MAX_ROWS - 1; r++){
				memcpy(contents[r], contents[r + 1], MAX_COLS);
			}
		} else curr_y = (curr_x + amt) / MAX_ROWS; // regular horizontal
		curr_x = (curr_x + amt) % MAX_ROWS; // horizontal wrapping
	}
}

static void process_char(char ch); // required bc of console_clear implementation

void console_clear(void)
{
    for(int i = 0; i < curr_x + curr_y * MAX_ROWS; i++)
		process_char('\b');
}

static void process_char(char ch)
{
	switch(ch){
		case '\r':
			increment(-1 * curr_x);
			break;
		case '\n':
			increment(MAX_ROWS - curr_x);
			break;
		case '\f':
			console_clear();
			break;
		case '\b':
			increment(-1);
			gl_draw_char(curr_x * gl_get_char_width(), curr_y * gl_get_char_height(), ' ', CON_COLOR);					
			break;
		default:
			gl_draw_char(curr_x * gl_get_char_width(), curr_y * gl_get_char_height(), ch, CON_COLOR);
			increment(1);
	}
}

int console_printf(const char *format, ...)
{
	int offset = curr_x + MAX_ROWS * curr_y;
	int size = MAX_ROWS * MAX_COLS;

	va_list args;
	va_start(args, format);
	int str_len = vsnprintf((char *)contents + offset, size - offset, format, args); 
	va_end(args);

	for(int i = offset; i < offset + str_len; i++){
		process_char(contents[curr_x][curr_y]);
	}
    return str_len;
}

