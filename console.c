#include "console.h"
#include "malloc.h"
#include "printf.h"
#include "gl.h"
#include "strings.h"

static const int MAX_LENGTH = 1000;
static const color_t FORE = GL_GREEN;
static const color_t BACK = GL_BLACK;
static char *contents;
static int curr_x, curr_y;
static int MAX_ROWS, MAX_COLS;

void console_init(unsigned int nrows, unsigned int ncols)
{
	curr_x = 0;
	curr_y = 0;
	MAX_ROWS = nrows;
	MAX_COLS = ncols;

	gl_init(MAX_COLS * gl_get_char_width(), MAX_ROWS * gl_get_char_height(), GL_DOUBLEBUFFER);
	gl_clear(BACK);

	contents = malloc(MAX_ROWS * MAX_COLS);
	for(int i = 0; i < MAX_ROWS * MAX_COLS; i++) memcpy(contents + i, " ", MAX_ROWS * MAX_COLS);
	// contents = malloc(nrows * sizeof(char *));
	// for(int i = 0; i < MAX_ROWS; i++){
	// 	contents[i] = malloc(MAX_COLS);
	// }
}

/**
* Move the cursor forward by amt taking into account the wrapping schemes
*/
static void increment(int amt){
	if(curr_x + amt < 0){ // backspace wrapping
		curr_x = MAX_COLS - (curr_x + amt);
		if(curr_y > 0) curr_y--;
	} else {
		if(curr_x + amt >= MAX_COLS && curr_y == MAX_ROWS - 1){ // vertical wrapping
			for(int r = 0; r < MAX_ROWS - 1; r++){
				memcpy(contents + r * MAX_ROWS, contents + (r + 1) * MAX_ROWS, MAX_COLS);
				// memcpy(contents[r], contents[r + 1], MAX_COLS);
			}
		} else curr_y += (curr_x + amt) / MAX_COLS; // regular horizontal
		curr_x = (curr_x + amt) % MAX_COLS; // horizontal wrapping
	}
}

void draw_char(int x, int y, char ch){
	gl_draw_rect(x * gl_get_char_width(), y * gl_get_char_height(), gl_get_char_width(), gl_get_char_height(), BACK);
	gl_draw_char(x * gl_get_char_width(), y * gl_get_char_height(), ch, FORE);
}

int console_printf(const char *format, ...); // required bc of console_clear implementation

void console_clear(void)
{
	curr_x = 0;
	curr_y = 0;
	for(int i = 0; i < MAX_ROWS * MAX_COLS; i++) memcpy(contents + i, " ", MAX_ROWS * MAX_COLS);
	gl_clear(BACK);
}

void process_char(char ch)
{
	switch(ch){
		case '\r':
			increment(-1 * curr_x);
			break;
		case '\n':
			increment(MAX_COLS - curr_x);
			break;
		case '\f':
			console_clear();
			break;
		case '\b':
			increment(-1);
			break;
		default:
			// clear slot and draw character
			contents[curr_x + curr_y * MAX_COLS] = ch;
			draw_char(curr_x, curr_y, ch);
			increment(1);
	}
}

void copy(){
	for(int i = 0; i < MAX_ROWS * MAX_COLS; i++){
		draw_char(i % MAX_COLS, i / MAX_COLS, contents[i]);
	}
}

int console_printf(const char *format, ...)
{
	char buf[MAX_LENGTH];
	va_list args;
	va_start(args, format);
	int str_len = vsnprintf(buf, sizeof(buf), format, args); 
	va_end(args);

	copy();
	for(int i = 0; i < str_len; i++){
		process_char(buf[i]);
	}
	gl_swap_buffer();

    return str_len;
}

