#include "font.h"
#include "gl.h"
#include "fb.h"

void gl_init(unsigned int width, unsigned int height, gl_mode_t mode)
{
    fb_init(width, height, 4, mode);    // use 32-bit depth always for graphics library
}

void gl_swap_buffer(void)
{
	fb_swap_buffer();
}

unsigned int gl_get_width(void)
{
    return fb_get_width();
}

unsigned int gl_get_height(void)
{
    return fb_get_height();
}

color_t gl_color(unsigned char r, unsigned char g, unsigned char b)
{
    return (0xff << 24) + (r << 16) + (g << 8) + b;
}

void gl_clear(color_t c)
{
	gl_draw_rect(0, 0, gl_get_width(), gl_get_height(), c);
}

void gl_draw_pixel(int x, int y, color_t c)
{
	int width = fb_get_pitch() / fb_get_depth(); // fb_get_width() may give different output
	color_t (*im)[width * gl_get_height()] = fb_get_draw_buffer();
	im[x][y] = c;
}

color_t gl_read_pixel(int x, int y)
{
	int width = fb_get_pitch() / fb_get_depth();
	color_t (*im)[width * gl_get_height()] = fb_get_draw_buffer();
    return im[x][y];
}

#define min(a, b) ((a < b) ? a : b)
void gl_draw_rect(int x, int y, int w, int h, color_t c)
{
	// loop implements clipping
	for(; x < min(x + w, gl_get_width()); x++){
		for(; y < min(y + h, gl_get_height()); y++){
			gl_draw_pixel(x, y, c);	
		}
	}
}

void gl_draw_char(int x, int y, int ch, color_t c)
{
	unsigned char buf[font_get_size()];
	font_get_char(ch, buf, sizeof(buf));

	// loop implements clipping
	for(; x < min(x + gl_get_char_width(), gl_get_width()); x++){
		for(; y < min(y + gl_get_char_height(), gl_get_height()); y++){
			if(buf[x + gl_get_width() * y])
				gl_draw_pixel(x, y, c);	
		}
	}
}

void gl_draw_string(int x, int y, const char* str, color_t c)
{
    const char **iter = &str; // iterates over characters of string
	while(**iter){
		gl_draw_char(x, y, **iter, c);
		x += gl_get_char_width();
		*iter += 1;
	}
}

unsigned int gl_get_char_height(void)
{
    return font_get_height();
}

unsigned int gl_get_char_width(void)
{
    return font_get_width();
}
