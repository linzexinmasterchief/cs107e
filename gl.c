#include "font.h"
#include "gl.h"
#include "fb.h"
#include "printf.h"
#include "timer.h"

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

unsigned int width(void){
	return fb_get_pitch() / fb_get_depth();
}

color_t gl_color(unsigned char r, unsigned char g, unsigned char b)
{
    return (0xff << 24) + (r << 16) + (g << 8) + b;
}

void gl_clear(color_t c)
{
	gl_draw_rect(0, 0, width(), gl_get_height(), c);
}

void gl_draw_pixel(int x, int y, color_t c)
{
	color_t (*im)[width()] = fb_get_draw_buffer();
	im[y][x] = c;
}

color_t gl_read_pixel(int x, int y)
{
	color_t (*im)[width()] = fb_get_draw_buffer();
    return im[y][x];
}

#define min(a, b) ((a) < (b) ? (a) : (b))
void gl_draw_rect(int x, int y, int w, int h, color_t c)
{
	// loop implements clipping
	for(int i = x; i < min(x + w, width()); i++){
		for(int j = y; j < min(y + h, gl_get_height()); j++){
			gl_draw_pixel(i, j, c);	
		}
	}
}

void gl_draw_char(int x, int y, int ch, color_t c)
{
	unsigned char buf[font_get_size()];
	font_get_char(ch, buf, sizeof(buf));

	// loop implements clipping
	int ind = 0;
	for(int i = x; i < min(x + gl_get_char_width(), gl_get_width()); i++){
		for(int j = y; j < min(y + gl_get_char_height(), gl_get_height()); j++){
			if(buf[(i - x) + gl_get_char_width() * (j - y)])
				gl_draw_pixel(i, j, c);	
			ind++;
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

#define abs(a) ((a > 0) ? (a) : (-a)) // This doesn't work, could you please tell me why?
// This implementation is based on the pseudocode for Bresenham's algorithm available on Wikipedia
// en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
void gl_draw_line(int x1, int y1, int x2, int y2, color_t c){
	int dx = (x2 - x1 > 0) ? x2 - x1 : x1 - x2; // x displacement
	int sx = x1 < x2 ? 1 : -1; // left or right
	int dy = (y2 - y1 < 0) ? y2 - y1 : y1 - y2; // y displacement
	int sy = y1 < y2 ? 1 : -1; // down or up
	int err = dx + dy; // total error

	// special cases
	if(y1 == y2){
		for(; x1 != x2; x1+=sx) 
			if(x1 < width() && y1 < gl_get_height()) gl_draw_pixel(x1, y1, c); 
		return;
	}
	if(x1 == x2){
		for(; y1 != y2; y1+=sy) 
			if(x1 < width() && y1 < gl_get_height()) gl_draw_pixel(x1, y1, c); 
		return;
	}
	
	while(1){
		if(x1 < width() && y1 < gl_get_height()) gl_draw_pixel(x1, y1, c); 
		if(x1 == x2 && y1 == y2) {
			break; // We're done!
		}
	
		int e2 = 2 * err;
		if(e2 <= dx){ // threshold for incrementing y1
			err += dx;
			y1 += sy;
		}
		if(e2 >= dy){ // threshold for incrementing x1
			err += dy;
			x1 += sx;
		}
	}
}

// Formula for area of a triangle given points found at
// www.mathopenref.com/coordtrianglearea.html/ memcpy(contents[r], contents[r + 1], MAX_COLS);
double area(int x1, int y1, int x2, int y2, int x3, int y3){
	float a = x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2);
	a = (a < 0) ? -a : a;
	return a /2.;
}

// Idea to use area came from http://mathworld.wolfram.com/TriangleInterior.html
// Substituted determinants for area here
int in_triangle(int x, int y, int x1, int y1, int x2, int y2, int x3, int y3){
	double full_area = area(x1, y1, x2, y2, x3, y3);
	double area_1 = area(x, y, x2, y2, x3, y3);
	double area_2 = area(x1, y1, x, y, x3, y3);
	double area_3 = area(x1, y1, x2, y2, x, y);
	return full_area - (area_1 + area_2 + area_3) == 0;
}

void gl_draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3, color_t c){
	gl_draw_line(x1, y1, x2, y2, c);
	gl_draw_line(x2, y2, x3, y3, c);
	gl_draw_line(x3, y3, x1, y1, c);

	for(int x = 0; x < width(); x++){
		for(int y = 0; y < gl_get_height(); y++){
			if(in_triangle(x, y, x1, y1, x2, y2, x3, y3))
				gl_draw_pixel(x, y, c);
		}
	}
}
