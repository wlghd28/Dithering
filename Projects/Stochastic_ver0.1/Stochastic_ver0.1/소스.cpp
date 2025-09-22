#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "mask.h"
#include "tiff.h"
#include "tiffconf.h"
#include "tiffio.h"
#include "tiffvers.h"

//#define MASK
#define MASKDATA
//#define DITHER
void *icc_profile = NULL;
uint32 count = 0;
BYTE* pix_buf;
BYTE* pix_out;
long width, height;
long pix_size;
float xres, yres;
uint32 min_color;
//int pad;
//BYTE trash[3] = { 0 }; // 쓰레기 값

unsigned int bit8_array[8] = { 128, 64, 32, 16, 8, 4, 2, 1 };
int main(void)
{
#ifdef MASK
	// mask 값 추출
	{
		int mask_width, mask_height;
		BYTE* pix_mask;
		TIFF* tiffmask;
		FILE* fp;
		fp = fopen("mask.txt", "w");
		tiffmask = TIFFOpen("mask.tiff", "rb");
		TIFFGetField(tiffmask, TIFFTAG_IMAGEWIDTH, &mask_width);
		TIFFGetField(tiffmask, TIFFTAG_IMAGELENGTH, &mask_height);

		pix_mask = (BYTE*)calloc(mask_width, sizeof(BYTE));

		for (int i = 0; i < mask_height; i++)
		{
			TIFFReadScanline(tiffmask, pix_mask, i, 0);
			for (int j = 0; j < mask_width; j++)
			{
				fprintf(fp, "%d%c ", pix_mask[j], ',');
			}
			fprintf(fp, "\n");
		}
		TIFFClose(tiffmask);
		free(pix_mask);
	}
#endif
#ifdef MASKDATA
	// 0~255 값 512x512 tif 이미지 생성
	{
		int masksize = 744;
		BYTE value[744];
		char fn[256] = "data/input_";
		char tmp[256] = { 0, };
		char num[10] = { 0, };
		TIFF *data;


		for (int i = 0; i < 256; i++)
		{
			itoa(i, num, 10);
			strcpy(tmp, fn);
			strcat(tmp, num);
			strcat(tmp, ".tif");
			data = TIFFOpen(tmp, "wb");

			TIFFSetField(data, TIFFTAG_IMAGEWIDTH, masksize);
			TIFFSetField(data, TIFFTAG_IMAGELENGTH, masksize);
			//TIFFSetField(output, TIFFTAG_ICCPROFILE, icc_profile);
			TIFFSetField(data, TIFFTAG_BITSPERSAMPLE, 8);
			TIFFSetField(data, TIFFTAG_SAMPLESPERPIXEL, 1);
			TIFFSetField(data, TIFFTAG_ROWSPERSTRIP, height);
			TIFFSetField(data, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
			TIFFSetField(data, TIFFTAG_XRESOLUTION, (float)1440);
			TIFFSetField(data, TIFFTAG_YRESOLUTION, (float)1440);
			TIFFSetField(data, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
			//TIFFSetField(output, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
			TIFFSetField(data, TIFFTAG_PHOTOMETRIC, 1);
			TIFFSetField(data, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
			TIFFSetField(data, TIFFTAG_COMPRESSION, COMPRESSION_NONE);

			for (int j = 0; j < masksize; j++)
			{
				value[j] = i;
			}
			for (int k = 0; k < masksize; k++)
			{
				TIFFWriteScanline(data, value, k, 0);
			}
			TIFFClose(data);
		}
	}
#endif
#ifdef DITHER
	TIFF *input;
	TIFF *output;
	// tiff 파일 읽기
	input = TIFFOpen("input.tif", "rb");

	if (input == NULL)
	{
		printf("File Not Found!!\n");
		exit(0);
	}

	TIFFGetField(input, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(input, TIFFTAG_IMAGELENGTH, &height);
	TIFFGetField(input, TIFFTAG_ICCPROFILE, &count, &icc_profile);
	TIFFGetField(input, TIFFTAG_XRESOLUTION, &xres);
	TIFFGetField(input, TIFFTAG_YRESOLUTION, &yres);
	TIFFGetField(input, TIFFTAG_PHOTOMETRIC, &min_color);
	pix_buf = (BYTE *)calloc(width, sizeof(BYTE));	// line scan memory
	pix_out = (BYTE *)calloc((width + 7) / 8, sizeof(BYTE));	// line output memory
	pix_size = width * height;
	printf("width : %d , height : %d\n", width, height);
	printf("image size : %d X %d\n", width, height);
	printf("resolution : %f X %f\n", xres, yres);
	printf("min_color : %d\n", min_color);

	// 디더링 결과 tiff 파일 쓰기
	output = TIFFOpen("output_Stochastic.tif", "wb");
	TIFFSetField(output, TIFFTAG_IMAGEWIDTH, width);
	TIFFSetField(output, TIFFTAG_IMAGELENGTH, height);
	//TIFFSetField(output, TIFFTAG_ICCPROFILE, icc_profile);
	TIFFSetField(output, TIFFTAG_BITSPERSAMPLE, 1);
	TIFFSetField(output, TIFFTAG_SAMPLESPERPIXEL, 1);
	TIFFSetField(output, TIFFTAG_ROWSPERSTRIP, height);
	TIFFSetField(output, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
	TIFFSetField(output, TIFFTAG_XRESOLUTION, xres);
	TIFFSetField(output, TIFFTAG_YRESOLUTION, yres);
	TIFFSetField(output, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	//TIFFSetField(output, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(output, TIFFTAG_PHOTOMETRIC, min_color);
	TIFFSetField(output, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
	TIFFSetField(output, TIFFTAG_COMPRESSION, COMPRESSION_NONE);

	for (int y = 0; y < height; y++)
	{
		TIFFReadScanline(input, pix_buf, y, 0);
		memset(pix_out, 0, (width + 7) / 8);
		for (int x = 0; x < width; x++)
		{

			if (pix_buf[x] > mask[(y % ms_y)][(x % ms_x)])
			{
				pix_out[x / 8] += bit8_array[x % 8];
			}
		}
		TIFFWriteScanline(output, pix_out, y, 0);
	}

	TIFFClose(input);
	TIFFClose(output);

	free(pix_buf);
	free(pix_out);
#endif
	return 0;
}

