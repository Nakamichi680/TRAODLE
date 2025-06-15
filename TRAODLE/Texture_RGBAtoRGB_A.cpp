#include "stdafx.h"


void Texture_RGBAtoRGB_A (unsigned int Xsize, unsigned int Ysize, char* input_buffer, char* output_buffer_rgb, char* output_buffer_alpha)
{
	for (unsigned int pixel = 0; pixel < Xsize * Ysize; pixel++)
	{
		output_buffer_rgb[pixel * 3] = input_buffer[pixel * 4];
		output_buffer_rgb[pixel * 3 + 1] = input_buffer[pixel * 4 + 1];
		output_buffer_rgb[pixel * 3 + 2] = input_buffer[pixel * 4 + 2];
		output_buffer_alpha[pixel] = input_buffer[pixel * 4 + 3];
	}
}