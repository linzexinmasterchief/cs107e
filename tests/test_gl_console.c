#include "assert.h"
#include "console.h"
#include "fb.h"
#include "gl.h"
#include "printf.h"
#include "strings.h"
#include "timer.h"
#include "uart.h"


#define _WIDTH 640
#define _HEIGHT 512

#define _NROWS 10
#define _NCOLS 20

void test_fb(void)
{
    fb_init(700, 700, 4, FB_SINGLEBUFFER);
    printf("Frame buffer pitch is %d\n", fb_get_pitch());

    unsigned char *cptr = fb_get_draw_buffer();
    int nbytes = fb_get_pitch()*fb_get_height();
    memset(cptr, 0x99, nbytes); // fill entire framebuffer with light gray pixels
    timer_delay(1);

	memset(cptr, 0x49, nbytes / 2); // fill bottom half of buffer with lighter gray pixels
	timer_delay(1);

	memset(cptr + nbytes / 2, 0x25, nbytes / 2); // fill top half with super light gray pixels
	timer_delay(1);

	memset(cptr, 0x0, nbytes); // make the whole screen black
	timer_delay(1);

	// Now with double buffering
    fb_init(700, 700, 4, FB_DOUBLEBUFFER);

    cptr = fb_get_draw_buffer();
    nbytes = fb_get_pitch()*fb_get_height();
    memset(cptr, 0x99, nbytes); // fill entire framebuffer with light gray pixels
	fb_swap_buffer();
    timer_delay(1);

	cptr = fb_get_draw_buffer();
	memset(cptr, 0x49, nbytes / 2); // fill bottom half of buffer with lighter gray pixels
	fb_swap_buffer();
	timer_delay(1);

	cptr = fb_get_draw_buffer();
	memset(cptr + nbytes / 2, 0x25, nbytes / 2); // fill top half with super light gray pixels
	fb_swap_buffer();
	timer_delay(1);

	cptr = fb_get_draw_buffer();
	memset(cptr, 0x0, nbytes); // make the whole screen black
	fb_swap_buffer();
	timer_delay(1);
}

void test_gl(void)
{
    // Single  buffer mode
    gl_init(_WIDTH, _HEIGHT, GL_SINGLEBUFFER);

    // Background is ?
    gl_clear(gl_color(0x45, 0x60, 0x75));

    // Draw white pixel at an arbitrary spot
    gl_draw_pixel(_WIDTH - 1, _HEIGHT - 1, GL_WHITE);
    assert(gl_read_pixel(_WIDTH - 1, _HEIGHT - 1) == GL_WHITE);

    // Blue rectangle in corner of screen
    gl_draw_rect(0, 0, 100, 100, GL_BLUE);

    // Single clipped magenta character
    gl_draw_char(_WIDTH - 5, 0, 'A', GL_MAGENTA);

    timer_delay(1); // time to visually confirm

    // Double buffer mode, make sure you test single buffer too!
    gl_init(_WIDTH, _HEIGHT, GL_DOUBLEBUFFER);

    // Background is purple
    gl_clear(gl_color(0x55, 0, 0x55));

    // Draw green pixel at an arbitrary spot
    gl_draw_pixel(_WIDTH/3, _HEIGHT/3, GL_GREEN);
    assert(gl_read_pixel(_WIDTH/3, _HEIGHT/3) == GL_GREEN);

    // Blue rectangle in center of screen
    gl_draw_rect(_WIDTH/2 - 50, _HEIGHT/2 - 50, 100, 100, GL_BLUE);

    // Single amber character
    gl_draw_char(60, 10, 'A', GL_AMBER);

    // Show buffer with drawn contents
    gl_swap_buffer();
    timer_delay(1); // visually confirm

	// Draw black string on white background
	gl_clear(GL_WHITE);
	gl_draw_string(60, 10, 	"You don't wanna know what time it is", GL_BLACK);
	
	// Show buffer with drawn contents
	gl_swap_buffer();
	timer_delay(1); // visually confirm
}

void test_console(void)
{
    console_init(_NROWS, _NCOLS);

    // 1: "HELLO WORLD"
    console_printf("HELLO WORLD\r");
    timer_delay(3);

    // 1: "HAPPY WORLD"
    // 2: "CODING"
    console_printf("HAPPY\nCODING\n");
    timer_delay(3);

    // Clear
    console_printf("\f");

    // 1: "Goodbye"
    console_printf("Goodbye!\n");
}

void test_triangle(void){
    gl_init(_WIDTH, _HEIGHT, GL_SINGLEBUFFER);
	gl_clear(GL_BLACK);

	// test draw line
	// gl_draw_line(_WIDTH / 2, 0, _WIDTH / 2, _HEIGHT, GL_WHITE);
	// gl_swap_buffer();
	// timer_delay(3);
	// gl_draw_line(0, _HEIGHT / 2, _WIDTH, _HEIGHT / 2, GL_WHITE);
	// gl_swap_buffer();
	// timer_delay(3);
	gl_draw_line(0, 0, _WIDTH, _HEIGHT, GL_WHITE);
	gl_draw_line(_WIDTH, 0, 0, _HEIGHT, GL_WHITE);
	timer_delay(3);

	// test draw triangle
	gl_draw_triangle(_WIDTH / 2, 0, 0, _HEIGHT - 1, _WIDTH - 1, _HEIGHT - 1, GL_AMBER);  
	timer_delay(3);
}

void main(void)
{
    uart_init();
    timer_init();
    printf("Executing main() in test_gl_console.c\n");

    // test_fb();
    // test_gl();
    // test_console();
	test_triangle();
	
    printf("Completed main() in test_gl_console.c\n");
    uart_putchar(EOT);
}
