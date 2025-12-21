#pragma once
#include "Classes.h"
#include "MA/MA_Classes.h"


void FindBestThreadNumber ();
bool AOD_IO_Init (char **argv);
bool AODRemastered_IO_Init (char **argv);
bool AE_IO_Init (char **argv);
bool SaveTGAResource (int resourceID, string filename);
int Fatal_Error_Terminate ();
Mesh DrawBox (string name, string parent, string layer, Vec3 Vmin, Vec3 Vmax, unsigned int VC_ARGB);
Mesh DrawTriangle (string name, string parent, string layer, Vec3 v0, Vec3 v1, Vec3 v2, unsigned int VC_ARGB);
Mesh DrawRectangle (string name, string parent, string layer, Vec3 v0, Vec3 v1, Vec3 v2, Vec3 v3, unsigned int VC_ARGB);
PolyCube DrawPolyCube (string name, string parent, string layer, string material_name, Vec3 BBmin, Vec3 BBmax, bool visible, unsigned int VC_ARGB);
void Texture_DXT1toRGB (unsigned int Xsize, unsigned int Ysize, char* input_buffer, char* output_buffer);
void Texture_DXT3toRGB_A (unsigned int Xsize, unsigned int Ysize, char* input_buffer, char* output_buffer_rgb, char* output_buffer_alpha);
void Texture_DXT3toRGBA (unsigned int Xsize, unsigned int Ysize, char* input_buffer, char* output_buffer);
void Texture_RGBAtoRGB_A (unsigned int Xsize, unsigned int Ysize, char* input_buffer, char* output_buffer_rgb, char* output_buffer_alpha);
void Texture_RAWtoDDS (string filename, unsigned int Xsize, unsigned int Ysize, unsigned int Mips, unsigned int RAWsize, DDSType Type, bool FlipVertical, char* buffer);
void Texture_RAWtoBMP (string filename, unsigned int Xsize, unsigned int Ysize, unsigned int RAWsize, BMPType Type, bool FlipVertical, char* buffer);
void Texture_RAWtoTGA (string filename, unsigned int Xsize, unsigned int Ysize, unsigned int RAWsize, TGAType Type, bool FlipVertical, char* buffer);