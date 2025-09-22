#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

#include "tiff.h"
#include "tiffconf.h"
#include "tiffio.h"
#include "tiffvers.h"

#include "mask.h"

typedef struct BIT2_DATA
{
	unsigned char bit3 : 2;
	unsigned char bit2 : 2;
	unsigned char bit1 : 2;
	unsigned char bit0 : 2;
}BIT2;



int main(void)
{

	int int_width = 0;
	int int_height = 0;
	float float_xres = 0;
	float float_yres = 0;
	int int_row_per_stript = 0;
	int int_bits_per_sample = 0;
	int int_samples_per_pixel = 0;

	int int_2bit_data_size = 0;

	BYTE* byte_pix_gray = NULL;
	BIT2* byte_pix_2bit = NULL;

	TIFF* fp_r = TIFFOpen("titan.tif", "r4");

	TIFFGetField(fp_r, TIFFTAG_IMAGEWIDTH, &int_width);
	TIFFGetField(fp_r, TIFFTAG_IMAGELENGTH, &int_height);
	TIFFGetField(fp_r, TIFFTAG_XRESOLUTION, &float_xres);
	TIFFGetField(fp_r, TIFFTAG_YRESOLUTION, &float_yres);
	TIFFGetField(fp_r, TIFFTAG_ROWSPERSTRIP, &int_row_per_stript);
	TIFFGetField(fp_r, TIFFTAG_BITSPERSAMPLE, &int_bits_per_sample);
	TIFFGetField(fp_r, TIFFTAG_SAMPLESPERPIXEL, &int_samples_per_pixel);

	int_2bit_data_size = (int_width + 3) / 4;

	byte_pix_gray = (BYTE*)calloc(int_2bit_data_size * 4, sizeof(BYTE));
	byte_pix_2bit = (BIT2*)calloc(int_2bit_data_size, sizeof(BIT2));

	TIFF* fp_w = TIFFOpen("titan_2bit.tif", "w4");

	TIFFSetField(fp_w, TIFFTAG_IMAGEWIDTH, int_width);
	TIFFSetField(fp_w, TIFFTAG_IMAGELENGTH, int_height);
	TIFFSetField(fp_w, TIFFTAG_XRESOLUTION, float_xres);
	TIFFSetField(fp_w, TIFFTAG_YRESOLUTION, float_yres);
	TIFFSetField(fp_w, TIFFTAG_SAMPLESPERPIXEL, 1);
	TIFFSetField(fp_w, TIFFTAG_BITSPERSAMPLE, 2);
	TIFFSetField(fp_w, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
	TIFFSetField(fp_w, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	TIFFSetField(fp_w, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(fp_w, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
	TIFFSetField(fp_w, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
	TIFFSetField(fp_w, TIFFTAG_ROWSPERSTRIP, int_row_per_stript);
	TIFFSetField(fp_w, TIFFTAG_PHOTOMETRIC, 0);

	int int_index = 0;
	for (int h = 0; h < int_height; h++)
	{
		memset(byte_pix_2bit, 0, int_2bit_data_size);

		TIFFReadScanline(fp_r, byte_pix_gray, h, 0);
		int_index = 0;

		for (int w = 0; w < int_2bit_data_size; w++)
		{
			// bit0
			if (byte_pix_gray[int_index] > mask_2bit[h % int_mask_col][int_index % int_mask_row].threshold3) byte_pix_2bit[w].bit0 = 3;
			else if (byte_pix_gray[int_index] > mask_2bit[h % int_mask_col][int_index % int_mask_row].threshold2) byte_pix_2bit[w].bit0 = 2;
			else if (byte_pix_gray[int_index] > mask_2bit[h % int_mask_col][int_index % int_mask_row].threshold1) byte_pix_2bit[w].bit0 = 1;
			else byte_pix_2bit[w].bit0 = 0;
			int_index++;

			// bit1
			if (byte_pix_gray[int_index] > mask_2bit[h % int_mask_col][int_index % int_mask_row].threshold3) byte_pix_2bit[w].bit1 = 3;
			else if (byte_pix_gray[int_index] > mask_2bit[h % int_mask_col][int_index % int_mask_row].threshold2) byte_pix_2bit[w].bit1 = 2;
			else if (byte_pix_gray[int_index] > mask_2bit[h % int_mask_col][int_index % int_mask_row].threshold1) byte_pix_2bit[w].bit1 = 1;
			else byte_pix_2bit[w].bit1 = 0;
			int_index++;

			// bit2
			if (byte_pix_gray[int_index] > mask_2bit[h % int_mask_col][int_index % int_mask_row].threshold3) byte_pix_2bit[w].bit2 = 3;
			else if (byte_pix_gray[int_index] > mask_2bit[h % int_mask_col][int_index % int_mask_row].threshold2) byte_pix_2bit[w].bit2 = 2;
			else if (byte_pix_gray[int_index] > mask_2bit[h % int_mask_col][int_index % int_mask_row].threshold1) byte_pix_2bit[w].bit2 = 1;
			else byte_pix_2bit[w].bit2 = 0;
			int_index++;

			// bit3
			if (byte_pix_gray[int_index] > mask_2bit[h % int_mask_col][int_index % int_mask_row].threshold3) byte_pix_2bit[w].bit3 = 3;
			else if (byte_pix_gray[int_index] > mask_2bit[h % int_mask_col][int_index % int_mask_row].threshold2) byte_pix_2bit[w].bit3 = 2;
			else if (byte_pix_gray[int_index] > mask_2bit[h % int_mask_col][int_index % int_mask_row].threshold1) byte_pix_2bit[w].bit3 = 1;
			else byte_pix_2bit[w].bit3 = 0;
			int_index++;
		}
		TIFFWriteScanline(fp_w, byte_pix_2bit, h, 0);

	}


	TIFFClose(fp_r);
	TIFFClose(fp_w);

	free(byte_pix_gray);
	free(byte_pix_2bit);

	return 0;
}