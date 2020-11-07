#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <err.h>
#include <string.h>
#include <math.h>

int isBigEndian()
{
	uint32_t x = 1;

	if(*(char *)&x)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

void swap_uint16(uint16_t * val)
{
    *val = (*val << 8) | (*val >> 8 );
}

void swap_uint32(uint32_t * val)
{
    *val = ((*val << 8) & 0xFF00FF00 ) | ((*val >> 8) & 0xFF00FF ); 
    *val = (*val << 16) | (*val >> 16);
}

void swap_int32(int32_t  * val)
{
    *val = ((*val << 8) & 0xFF00FF00) | ((*val >> 8) & 0xFF00FF ); 
    *val = (*val << 16) | ((*val >> 16) & 0xFFFF);
}

int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		errx(1, "Invalid number of arguments. Usage: ./program_name src_pic.bmp dest_pic.bmp.");
	}

	int fd, fd2;

	fd = open(argv[1], O_RDONLY);

	if(fd == -1)
	{
		errx(1, "Couldn't read file '%s'.", argv[1]);
	}

	fd2 = open(argv[2], O_WRONLY | O_TRUNC | O_CREAT, 0664);

	if(fd2 == -1)
	{
		errx(1, "Couldn't open file '%s' in write mode.", argv[2]);
	}

	unsigned char bmpHeaders[54];
	if(read(fd, &bmpHeaders, 54) != 54)
	{
		errx(1, "Couldn't read BMP header.");
	}

	//BM header field (offset: 0; size: 2)	
	if(bmpHeaders[0] != 'B' || bmpHeaders[1] != 'M')
	{
		errx(1, "Invalid image format. Only .bmp allowed!");
	}

	int bigEndian = isBigEndian();

	//size of DIB header (offset: 14, size: 4)
	//If the header is BITMAPINFOHEADER it automatically means that there is no colorspace information.
	uint32_t dibHeader;
	dibHeader = *(uint32_t*)(bmpHeaders + 14);
	if(bigEndian)
	{
		swap_uint32(&dibHeader);
	}

	if(dibHeader != 40)
	{
		errx(1, "Invalid DIB Header.");
	}

	//BITMAPINFOHEADER (offset: 14, size 40)
	//width (offset: 18, size: 4), height (offset: 22, size: 4)
	uint32_t width;
	int32_t height;
	width = *(uint32_t*)(bmpHeaders + 18);
	height = *(int32_t*)(bmpHeaders + 22);
	if(bigEndian)
	{
		swap_uint32(&width);
		swap_int32(&height);
	}

	uint32_t absHeight = abs(height);

	if(width < 20 || absHeight < 20)
	{
		errx(1, "Image dimensions (%dx%d) smaller than allowed (20x20).", width, absHeight);
	}

	//color depth of image (offset: 28, size: 2)
	uint16_t colorDepth;
	colorDepth = *(uint16_t*)(bmpHeaders + 28);
	if(bigEndian)
	{
		swap_uint16(&colorDepth);
	}

	if(colorDepth != 24)
	{
		errx(1, "Color depth of image isn't 24-bit. Instead, it's %d-bit.", colorDepth);
	}

	//compression method (offset: 30, size : 4)
	uint32_t compressionMethod;
	compressionMethod = *(uint32_t*)(bmpHeaders + 30);
	if(bigEndian)
	{
		swap_uint32(&compressionMethod);
	}

	if(compressionMethod)
	{
		errx(1, "Compressed images are not allowed.");
	}

	//Write headers to the destination BMP file
	if(write(fd2, bmpHeaders, 54) != 54)
	{
		errx(1, "Couldn't write BMP headers to new image.");
	}

	uint8_t padding = width % 4;

	//allocate memory for color table and read data from file into the dynamically allocated array
	uint8_t *** colorTable = (uint8_t ***) malloc(absHeight * sizeof(uint8_t **));
	if(colorTable == NULL)
	{
		err(1, "Couldn't allocate memory.");
	}

	for(uint32_t i = 0; i < absHeight; ++i)
	{
		colorTable[i] = (uint8_t **) malloc(width * sizeof(uint8_t *));
		if(colorTable[i] == NULL)
		{
			err(1, "Couldn't allocate memory.");
		}

		for(uint32_t j = 0; j < width; ++j)
		{
			colorTable[i][j] = (uint8_t *) malloc(3 * sizeof(uint8_t));
			if(colorTable[i][j] == NULL)
			{
				err(1, "Couldn't allocate memory.");
			}

			if(read(fd, colorTable[i][j], 3) != 3)
			{
				errx(1, "Couldn't read file.");
			}
		}

		if(lseek(fd, padding, SEEK_CUR) == -1)
		{
			errx(1, "Couldn't seek file.");
		}
	}	

	//Order: blue, green, red
	uint8_t red[3] = {0, 0, 255};
	uint8_t green[3] = {0, 255, 0};
	uint8_t blue[3] = {255, 0, 0};
	uint8_t black[3] = {0, 0, 0};

	//horizontal lines
	for(uint32_t i = 5; i < width - 5; ++i)
	{
		if(height > 0)
		{
			//bottom line in red
			memcpy(&colorTable[4][i][0], red, 3);
			//top line in green
			memcpy(&colorTable[absHeight - 5][i][0], green, 3);
		}
		else
		{
			//bottom line in red
			memcpy(&colorTable[absHeight - 5][i][0], red, 3);
			//top line in green
			memcpy(&colorTable[4][i][0], green, 3);
		}
	}

	//vertical lines in blue
	for(uint32_t i = 5; i < absHeight - 5; ++i)
	{
		//left line
		memcpy(&colorTable[i][4][0], blue, 3);
		//right line
		memcpy(&colorTable[i][width - 5][0], blue, 3);
	}

	//black dots in the 4 corners
	memcpy(&colorTable[4][4][0], black, 3);
	memcpy(&colorTable[4][width - 5][0], black, 3);
	memcpy(&colorTable[absHeight - 5][4][0], black, 3);
	memcpy(&colorTable[absHeight - 5][width - 5][0], black, 3);

	//write color table
	for(uint32_t i = 0; i < absHeight; ++i)
	{
		for(uint32_t j = 0; j < width; ++j)
		{
			if(write(fd2, colorTable[i][j], 3) != 3)
			{
				errx(1, "Couldn't write color table to file.");
			}
		}

		for(uint8_t p = 0; p < padding; ++p)
		{
			if(write(fd2, "0", 1) != 1)
			{
				errx(1, "Couldn't write padding to file.");
			}
		}
	}

	//free memory
	for(uint32_t i = 0; i < absHeight; ++i)
	{
		for(uint32_t j = 0; j < width; ++j)
		{
			free(colorTable[i][j]);
		}

		free(colorTable[i]);
	}

	free(colorTable);

	//close file descriptors
	close(fd);
	close(fd2);

	write(1, "Image processed successfully.\n", 30);

	return 0;
}
