#include "stdafx.h"


void Texture_DXT3toRGBA (unsigned int Xsize, unsigned int Ysize, char* input_buffer, char* output_buffer)
{
	for (unsigned int row = 0; row < Ysize; row += 4)
	{
		for (unsigned int column = 0; column < Xsize; column += 4)
		{
			unsigned long long alpha;
			unsigned short c0, c1;
			unsigned char c0r, c0g, c0b, c1r, c1g, c1b, c2r, c2g, c2b, c3r, c3g, c3b, row1, row2, row3, row4;
			vector < vector <unsigned char> > indextable (4), alphatable (4);

			// Lettura alpha channel, tabella colori e tabella indici
			memcpy(&alpha, input_buffer + column * 4 + row * Xsize, 8);
			memcpy(&c0, input_buffer + column * 4 + row * Xsize + 8, 2);
			memcpy(&c1, input_buffer + column * 4 + row * Xsize + 10, 2);
			memcpy(&row1, input_buffer + column * 4 + row * Xsize + 12, 1);
			memcpy(&row2, input_buffer + column * 4 + row * Xsize + 13, 1);
			memcpy(&row3, input_buffer + column * 4 + row * Xsize + 14, 1);
			memcpy(&row4, input_buffer + column * 4 + row * Xsize + 15, 1);

			// Estrazione informazioni alpha channel dei singoli pixel
			alphatable[0].push_back((alpha & 0b1111) * 17);
			alphatable[0].push_back(((alpha >> 4) & 0b1111) * 17);
			alphatable[0].push_back(((alpha >> 8) & 0b1111) * 17);
			alphatable[0].push_back(((alpha >> 12) & 0b1111) * 17);
			alphatable[1].push_back(((alpha >> 16) & 0b1111) * 17);
			alphatable[1].push_back(((alpha >> 20) & 0b1111) * 17);
			alphatable[1].push_back(((alpha >> 24) & 0b1111) * 17);
			alphatable[1].push_back(((alpha >> 28) & 0b1111) * 17);
			alphatable[2].push_back(((alpha >> 32) & 0b1111) * 17);
			alphatable[2].push_back(((alpha >> 36) & 0b1111) * 17);
			alphatable[2].push_back(((alpha >> 40) & 0b1111) * 17);
			alphatable[2].push_back(((alpha >> 44) & 0b1111) * 17);
			alphatable[3].push_back(((alpha >> 48) & 0b1111) * 17);
			alphatable[3].push_back(((alpha >> 52) & 0b1111) * 17);
			alphatable[3].push_back(((alpha >> 56) & 0b1111) * 17);
			alphatable[3].push_back(((alpha >> 60) & 0b1111) * 17);

			// Ricostruzione componenti colori
			c0r = ((c0 >> 11) * 527 + 23) >> 6;
			c0g = (((c0 >> 5) & 0b111111) * 259 + 33) >> 6;
			c0b = ((c0 & 0b11111) * 527 + 23) >> 6;
			c1r = ((c1 >> 11) * 527 + 23) >> 6;
			c1g = (((c1 >> 5) & 0b111111) * 259 + 33) >> 6;
			c1b = ((c1 & 0b11111) * 527 + 23) >> 6;
			c2r = (unsigned char)(0.6666667 * c0r + 0.3333333 * c1r);
			c2g = (unsigned char)(0.6666667 * c0g + 0.3333333 * c1g);
			c2b = (unsigned char)(0.6666667 * c0b + 0.3333333 * c1b);
			c3r = (unsigned char)(0.6666667 * c1r + 0.3333333 * c0r);
			c3g = (unsigned char)(0.6666667 * c1g + 0.3333333 * c0g);
			c3b = (unsigned char)(0.6666667 * c1b + 0.3333333 * c0b);

			// Estrazione indici pixels
			indextable[0].push_back(row1 & 0b11);
			indextable[0].push_back((row1 >> 2) & 0b11);
			indextable[0].push_back((row1 >> 4) & 0b11);
			indextable[0].push_back(row1 >> 6);
			indextable[1].push_back(row2 & 0b11);
			indextable[1].push_back((row2 >> 2) & 0b11);
			indextable[1].push_back((row2 >> 4) & 0b11);
			indextable[1].push_back(row2 >> 6);
			indextable[2].push_back(row3 & 0b11);		
			indextable[2].push_back((row3 >> 2) & 0b11);
			indextable[2].push_back((row3 >> 4) & 0b11);
			indextable[2].push_back(row3 >> 6);
			indextable[3].push_back(row4 & 0b11);
			indextable[3].push_back((row4 >> 2) & 0b11);
			indextable[3].push_back((row4 >> 4) & 0b11);
			indextable[3].push_back(row4 >> 6);

			// Scrittura pixels nel buffer di output colore e alpha
			for (unsigned int r = 0; r < 4; r++)
				for (unsigned int c = 0; c < 4; c++)
				{
					// Scrittura colore
					unsigned int write_pos = row * Xsize * 4 +  r * Xsize * 4 + column * 4 + c * 4;
					switch (indextable[r][c])
					{
					case 0:
						output_buffer[write_pos] = c0b;
						output_buffer[write_pos + 1] = c0g;
						output_buffer[write_pos + 2] = c0r;
						break;
					case 1:
						output_buffer[write_pos] = c1b;
						output_buffer[write_pos + 1] = c1g;
						output_buffer[write_pos + 2] = c1r;
						break;
					case 2:
						output_buffer[write_pos] = c2b;
						output_buffer[write_pos + 1] = c2g;
						output_buffer[write_pos + 2] = c2r;
						break;
					case 3:
						output_buffer[write_pos] = c3b;
						output_buffer[write_pos + 1] = c3g;
						output_buffer[write_pos + 2] = c3r;
					}

					// Scrittura alpha
					output_buffer[write_pos + 3] = alphatable[r][c];
				}
		}
	}
}