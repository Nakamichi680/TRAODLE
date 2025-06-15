#pragma once
#include "Classes.h"


void FindBestThreadNumber ();
bool AOD_IO_Init (char **argv);
bool AE_IO_Init (char **argv);
int Fatal_Error_Terminate ();
Mesh DrawBox (string name, string parent, string layer, XYZ Vmin, XYZ Vmax, unsigned int VC_ARGB);
Mesh DrawTriangle (string name, string parent, string layer, XYZ v0, XYZ v1, XYZ v2, unsigned int VC_ARGB);
Mesh DrawRectangle (string name, string parent, string layer, XYZ v0, XYZ v1, XYZ v2, XYZ v3, unsigned int VC_ARGB);
void Texture_DXT1toRGB (unsigned int Xsize, unsigned int Ysize, char* input_buffer, char* output_buffer);
void Texture_DXT3toRGB_A (unsigned int Xsize, unsigned int Ysize, char* input_buffer, char* output_buffer_rgb, char* output_buffer_alpha);
void Texture_DXT3toRGBA (unsigned int Xsize, unsigned int Ysize, char* input_buffer, char* output_buffer);
void Texture_RGBAtoRGB_A (unsigned int Xsize, unsigned int Ysize, char* input_buffer, char* output_buffer_rgb, char* output_buffer_alpha);
void Texture_RAWtoDDS (string filename, unsigned int Xsize, unsigned int Ysize, unsigned int Mips, unsigned int RAWsize, DDSType Type, bool FlipVertical, char* buffer);
void Texture_RAWtoBMP (string filename, unsigned int Xsize, unsigned int Ysize, unsigned int RAWsize, BMPType Type, bool FlipVertical, char* buffer);
void Texture_RAWtoTGA (string filename, unsigned int Xsize, unsigned int Ysize, unsigned int RAWsize, TGAType Type, bool FlipVertical, char* buffer);