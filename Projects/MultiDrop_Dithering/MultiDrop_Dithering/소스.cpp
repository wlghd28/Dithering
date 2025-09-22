#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

#include "tiff.h"
#include "tiffconf.h"
#include "tiffio.h"
#include "tiffvers.h"

#include "mask.h"

//#define TEST

//#define TIFFWRITE
#define BMPWRITE

#define STOCHASTIC
//#define FLOYD_STEINBERG
//#define JWJI



typedef struct DATA
{
	BYTE bit3 : 2;
	BYTE bit2 : 2;
	BYTE bit1 : 2;
	BYTE bit0 : 2;
}BIT2;

int ApplyCurve(BYTE* byte_Curve, const char* str_CurveFileName);

int main(void)
{
	BYTE byte_Bit_Array[8] = { 128, 64, 32, 16, 8, 4, 2, 1 };
#ifdef TIFFWRITE
	int int_width = 0;
	int int_height = 0;
	float float_xres = 0;
	float float_yres = 0;
	int int_row_per_stript = 0;
	int int_bits_per_sample = 0;
	int int_samples_per_pixel = 0;
	int int_photometric = 0;

	int int_2bit_data_size = 0;

	TIFF* fp_r = TIFFOpen("test.tif", "r4");

	TIFFGetField(fp_r, TIFFTAG_IMAGEWIDTH, &int_width);
	TIFFGetField(fp_r, TIFFTAG_IMAGELENGTH, &int_height);
	TIFFGetField(fp_r, TIFFTAG_XRESOLUTION, &float_xres);
	TIFFGetField(fp_r, TIFFTAG_YRESOLUTION, &float_yres);
	TIFFGetField(fp_r, TIFFTAG_ROWSPERSTRIP, &int_row_per_stript);
	TIFFGetField(fp_r, TIFFTAG_BITSPERSAMPLE, &int_bits_per_sample);
	TIFFGetField(fp_r, TIFFTAG_SAMPLESPERPIXEL, &int_samples_per_pixel);
	TIFFGetField(fp_r, TIFFTAG_PHOTOMETRIC, &int_photometric);

	int_2bit_data_size = (int_width + 3) / 4;

	TIFF* fp_w = TIFFOpen("test_2bit.tif", "w4");

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
	TIFFSetField(fp_w, TIFFTAG_PHOTOMETRIC, int_photometric);

#endif

#ifdef BMPWRITE
	int int_width = 0;
	int int_height = 0;
	float float_xres = 0;
	float float_yres = 0;

	int iBpl_8bit = 0;
	int iBpl_4bit = 0;

	// 파일 헤더
	BITMAPFILEHEADER bfh;
	BITMAPINFOHEADER bih;


	FILE* fp_r = fopen("test.bmp", "rb");

	// 헤더 정보 GET
	fread(&bfh, sizeof(BITMAPFILEHEADER), 1, fp_r);
	fread(&bih, sizeof(BITMAPINFOHEADER), 1, fp_r);

	// Gray 8bit 파일은 팔레트까지 읽어줘야 한다.
	RGBQUAD rgbq_palRGB[256];        // 팔레트 정보 구조체 배열
	fread(rgbq_palRGB, sizeof(RGBQUAD) * 256, 1, fp_r);

	int_width = bih.biWidth;
	int_height = bih.biHeight;
	float_xres = (float)bih.biXPelsPerMeter;
	float_yres = (float)bih.biYPelsPerMeter;

	iBpl_8bit = (int_width + 3) / 4 * 4;
	iBpl_4bit = (((int_width + 1) / 2) + 3) / 4 * 4;


	FILE* fp_w = fopen("test_2bit.bmp", "wb");

	// 헤더 정보 SET
	bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 16;
	bfh.bfOffBits = 0;
	bih.biBitCount = 4;
	bih.biSizeImage = (int)(iBpl_4bit * int_height);
	bih.biClrUsed = 16;
	bih.biWidth = int_width;
	bih.biHeight = int_height;
	bih.biXPelsPerMeter = (int)float_xres;
	bih.biYPelsPerMeter = (int)float_yres;
	fwrite(&bfh, sizeof(BITMAPFILEHEADER), 1, fp_w);
	fwrite(&bih, sizeof(BITMAPINFOHEADER), 1, fp_w);


	// index 2bit 파일 팔레트 정보 SET
	RGBQUAD rgbq_pal4bit[16];        // 팔레트 정보 구조체 배열
	for (int i = 0; i < 16; i++)
	{
		rgbq_pal4bit[i].rgbRed = (BYTE)rgbq_palRGB[i * 17].rgbRed;
		rgbq_pal4bit[i].rgbGreen = (BYTE)rgbq_palRGB[i * 17].rgbGreen;
		rgbq_pal4bit[i].rgbBlue = (BYTE)rgbq_palRGB[i * 17].rgbBlue;
		rgbq_pal4bit[i].rgbReserved = 0;
	}
	fwrite(rgbq_pal4bit, sizeof(RGBQUAD) * 16, 1, fp_w);

#endif
#ifdef TEST
	BYTE* byte_pix_2bit_read = NULL;
	BYTE* byte_pix_2bit_write = NULL;
	byte_pix_2bit_read = (BYTE*)calloc(int_2bit_data_size, sizeof(BYTE));
	byte_pix_2bit_write = (BYTE*)calloc(int_2bit_data_size, sizeof(BYTE));

	for (int h = 0; h < int_height; h++)
	{
		memset(byte_pix_2bit_read, 0, int_2bit_data_size);
		memset(byte_pix_2bit_write, 0, int_2bit_data_size);

		TIFFReadScanline(fp_r, byte_pix_2bit_read, h, 0);

		for (int w = 0; w < int_width; w++)
		{
			//if(((byte_pix_2bit_read[w / 4] >> ((3 - (w % 4)) * 2)) & 0x03) == 1)
				//byte_pix_2bit_write[w / 4] += byte_Bit_Array[((w % 4) * 2) + 1];

			//if(((byte_pix_2bit_read[w / 4] >> ((3 - (w % 4)) * 2)) & 0x03) == 2)
				//byte_pix_2bit_write[w / 4] += byte_Bit_Array[(w % 4) * 2];

			if (((byte_pix_2bit_read[w / 4] >> ((3 - (w % 4)) * 2)) & 0x03) == 3)
			{
				byte_pix_2bit_write[w / 4] += byte_Bit_Array[((w % 4) * 2) + 1];
				byte_pix_2bit_write[w / 4] += byte_Bit_Array[(w % 4) * 2];
			}
		}

		TIFFWriteScanline(fp_w, byte_pix_2bit_write, h, 0);
	}


	free(byte_pix_2bit_read);
	free(byte_pix_2bit_write);
#endif



#ifdef STOCHASTIC
#ifdef TIFFWRITE
	//ApplyCurve(byte_Curve_Lv1, "Level1_cuv.txt");
	//ApplyCurve(byte_Curve_Lv2, "Level2_cuv.txt");
	//ApplyCurve(byte_Curve_Lv3, "Level3_cuv.txt");

	ApplyCurve(byte_Curve_Lv1, "Level1_ONYX.txt");
	ApplyCurve(byte_Curve_Lv2, "Level2_ONYX.txt");
	ApplyCurve(byte_Curve_Lv3, "Level3_ONYX.txt");

	BYTE* byte_pix_gray = NULL;
	BYTE* byte_pix_2bit = NULL;
	byte_pix_gray = (BYTE*)calloc(int_2bit_data_size * 4, sizeof(BYTE));
	byte_pix_2bit = (BYTE*)calloc(int_2bit_data_size, sizeof(BYTE));


	int iValue = 0;

	// 2bit Dithering
	for (int h = 0; h < int_height; h++)
	{
		memset(byte_pix_2bit, 0, int_2bit_data_size);
		TIFFReadScanline(fp_r, byte_pix_gray, h, 0);

		for (int w = 0; w < int_width; w++)
		{
			// Level 1
			if (byte_Curve_Lv1[byte_pix_gray[w]] + byte_Curve_Lv2[byte_pix_gray[w]] + byte_Curve_Lv3[byte_pix_gray[w]] > byte_mask[h % int_mask_col][w % int_mask_row])
			{
				byte_pix_2bit[w / 4] += 0x01 << ((3 - (w % 4)) * 2);
			}
			

			// Level 2
			if (byte_Curve_Lv2[byte_pix_gray[w]] + byte_Curve_Lv3[byte_pix_gray[w]] > byte_mask[h % int_mask_col][w % int_mask_row])
			{
				iValue = 2 - ((byte_pix_2bit[w / 4] >> ((3 - (w % 4)) * 2)) & 0x03);
				byte_pix_2bit[w / 4] += iValue << ((3 - (w % 4)) * 2);
			}

			// Level 3
			if (byte_Curve_Lv3[byte_pix_gray[w]] > byte_mask[h % int_mask_col][w % int_mask_row])
			{
				iValue = 3 - ((byte_pix_2bit[w / 4] >> ((3 - (w % 4)) * 2)) & 0x03);
				byte_pix_2bit[w / 4] += iValue << ((3 - (w % 4)) * 2);
			}
		}
		TIFFWriteScanline(fp_w, byte_pix_2bit, h, 0);

	}


	free(byte_pix_gray);
	free(byte_pix_2bit);
#endif

#ifdef BMPWRITE
	// BMP는 1, 4, 8, 16, 24, 32 비트를 지원한다. (중요!!)
	//ApplyCurve(byte_Curve_Lv1, "Level1_cuv.txt");
	//ApplyCurve(byte_Curve_Lv2, "Level2_cuv.txt");
	//ApplyCurve(byte_Curve_Lv3, "Level3_cuv.txt");

	ApplyCurve(byte_Curve_Lv1, "Level1_ONYX.txt");
	ApplyCurve(byte_Curve_Lv2, "Level2_ONYX.txt");
	ApplyCurve(byte_Curve_Lv3, "Level3_ONYX.txt");


	BYTE* byte_pix_gray = NULL;
	BYTE* byte_pix_4bit = NULL;
	byte_pix_gray = (BYTE*)calloc(iBpl_8bit, sizeof(BYTE));
	byte_pix_4bit = (BYTE*)calloc(iBpl_4bit, sizeof(BYTE));

	// 2bit Dithering
	for (int h = 0; h < int_height; h++)
	{
		memset(byte_pix_4bit, 0, iBpl_4bit);

		fread(byte_pix_gray, sizeof(BYTE), iBpl_8bit, fp_r);

		int iValue = 0;

		for (int w = 0; w < int_width; w++)
		{
			// Level 1
			if (byte_Curve_Lv1[byte_pix_gray[w]] + byte_Curve_Lv2[byte_pix_gray[w]] + byte_Curve_Lv3[byte_pix_gray[w]] > byte_mask[h % int_mask_col][w % int_mask_row])
			{
				//byte_pix_4bit[w / 2] += byte_Bit_Array[((w % 2) * 4) + 1];
				//byte_pix_4bit[w / 2] += byte_Bit_Array[((w % 2) * 4) + 3];

				byte_pix_4bit[w / 2] += 0x05 << ((1 - (w % 2)) * 4);
			}

			// Level 2
			if (byte_Curve_Lv2[byte_pix_gray[w]] + byte_Curve_Lv3[byte_pix_gray[w]] > byte_mask[h % int_mask_col][w % int_mask_row])
			{
				//byte_pix_4bit[w / 2] -= byte_Bit_Array[((w % 2) * 4) + 1];
				//byte_pix_4bit[w / 2] -= byte_Bit_Array[((w % 2) * 4) + 3];
				//byte_pix_4bit[w / 2] += byte_Bit_Array[(w % 2) * 4];
				//byte_pix_4bit[w / 2] += byte_Bit_Array[((w % 2) * 4) + 2];

				iValue = 10 - ((byte_pix_4bit[w / 2] >> ((1 - (w % 2)) * 4)) & 0xF);
				byte_pix_4bit[w / 2] += iValue << ((1 - (w % 2)) * 4);
			}

			// Level 3
			if (byte_Curve_Lv3[byte_pix_gray[w]] > byte_mask[h % int_mask_col][w % int_mask_row])
			{
				//byte_pix_4bit[w / 2] += byte_Bit_Array[((w % 2) * 4) + 1];
				//byte_pix_4bit[w / 2] += byte_Bit_Array[((w % 2) * 4) + 3];
				iValue = 15 - ((byte_pix_4bit[w / 2] >> ((1 - (w % 2)) * 4)) & 0xF);
				byte_pix_4bit[w / 2] += iValue << ((1 - (w % 2)) * 4);
			}
		}
		fwrite(byte_pix_4bit, sizeof(BYTE), iBpl_4bit, fp_w);

	}

	free(byte_pix_gray);
	free(byte_pix_4bit);
#endif
#endif


#ifdef FLOYD_STEINBERG
#ifdef TIFF
	int iBpl = int_2bit_data_size * 4;

	BYTE* byte_pix_gray = NULL;
	BYTE* byte_pix_Error = NULL;
	BYTE* byte_pix_2bit = NULL;
	byte_pix_gray = (BYTE*)calloc(iBpl * int_height, sizeof(BYTE));
	byte_pix_Error = (BYTE*)calloc(iBpl * (int_height + 1), sizeof(BYTE));
	byte_pix_2bit = (BYTE*)calloc(int_2bit_data_size, sizeof(BYTE));


	// Read Data
	for (int h = 0; h < int_height; h++)
	{
		TIFFReadScanline(fp_r, byte_pix_gray + (h * iBpl), h, 0);
	}


	// 2bit Dithering
	int iQuantError = 0;
	int iThreshold1 = 82;
	int iThreshold2 = 171;
	int iThreshold3 = 256;
	for (int h = 0; h < int_height - 1; h++)
	{
		memset(byte_pix_2bit, 0, int_2bit_data_size);

		for (int w = 1; w < int_width - 1; w++)
		{
			// Level 1
			if (byte_pix_gray[h * iBpl + w] < iThreshold1)
			{
				byte_pix_Error[h * iBpl + w] += byte_pix_gray[h * iBpl + w];
				byte_pix_gray[h * iBpl + w] = byte_pix_Error[h * iBpl + w] / 128 * 255;
				iQuantError = byte_pix_Error[h * iBpl + w] - byte_pix_gray[h * iBpl + w];

				byte_pix_Error[h * iBpl + w + 1] += iQuantError * 7 / 16;
				byte_pix_Error[(h + 1) * iBpl + w - 1] += iQuantError * 3 / 16;
				byte_pix_Error[(h + 1) * iBpl + w] += iQuantError * 5 / 16;
				byte_pix_Error[(h + 1) * iBpl + w + 1] += iQuantError * 1 / 16;

				if (byte_pix_gray[h * iBpl + w] == 255)
				{
					byte_pix_2bit[w / 4] += byte_Bit_Array[((w % 4) * 2) + 1];
				}
				else
				{

				}
			}
			// Level 2
			else if (byte_pix_gray[h * iBpl + w] < iThreshold2)
			{
				byte_pix_Error[h * iBpl + w] += byte_pix_gray[h * iBpl + w];
				byte_pix_gray[h * iBpl + w] = byte_pix_Error[h * iBpl + w] / 128 * 255;
				iQuantError = byte_pix_Error[h * iBpl + w] - byte_pix_gray[h * iBpl + w];

				byte_pix_Error[h * iBpl + w + 1] += iQuantError * 7 / 16;
				byte_pix_Error[(h + 1) * iBpl + w - 1] += iQuantError * 3 / 16;
				byte_pix_Error[(h + 1) * iBpl + w] += iQuantError * 5 / 16;
				byte_pix_Error[(h + 1) * iBpl + w + 1] += iQuantError * 1 / 16;

				if (byte_pix_gray[h * iBpl + w] == 255)
				{
					byte_pix_2bit[w / 4] += byte_Bit_Array[(w % 4) * 2];
				}
				else
				{
					byte_pix_2bit[w / 4] += byte_Bit_Array[((w % 4) * 2) + 1];
				}
			}
			// Level 3
			else if (byte_pix_gray[h * iBpl + w] < iThreshold3)
			{
				byte_pix_Error[h * iBpl + w] += byte_pix_gray[h * iBpl + w];
				byte_pix_gray[h * iBpl + w] = byte_pix_Error[h * iBpl + w] / 128 * 255;
				iQuantError = byte_pix_Error[h * iBpl + w] - byte_pix_gray[h * iBpl + w];

				byte_pix_Error[h * iBpl + w + 1] += iQuantError * 7 / 16;
				byte_pix_Error[(h + 1) * iBpl + w - 1] += iQuantError * 3 / 16;
				byte_pix_Error[(h + 1) * iBpl + w] += iQuantError * 5 / 16;
				byte_pix_Error[(h + 1) * iBpl + w + 1] += iQuantError * 1 / 16;

				if (byte_pix_gray[h * iBpl + w] == 255)
				{
					byte_pix_2bit[w / 4] += byte_Bit_Array[((w % 4) * 2) + 1];
					byte_pix_2bit[w / 4] += byte_Bit_Array[(w % 4) * 2];
				}
				else
				{
					byte_pix_2bit[w / 4] += byte_Bit_Array[(w % 4) * 2];
				}
			}
		}
		TIFFWriteScanline(fp_w, byte_pix_2bit, h, 0);

	}


	free(byte_pix_gray);
	free(byte_pix_Error);
	free(byte_pix_2bit);
#endif

#ifdef BMPWRITE

#endif
#endif


#ifdef JWJI
	//ApplyCurve(byte_Curve_Lv1, "Level1_JWJI.txt");
	//ApplyCurve(byte_Curve_Lv2, "Level2_JWJI.txt");
	ApplyCurve(byte_Curve_Lv3, "Level3_JWJI.txt");

	BYTE* byte_pix_gray = NULL;
	BYTE* byte_pix_2bit = NULL;
	byte_pix_gray = (BYTE*)calloc(int_2bit_data_size * 4, sizeof(BYTE));
	byte_pix_2bit = (BYTE*)calloc(int_2bit_data_size, sizeof(BYTE));


	int iThreshold1 = 85;
	int iThreshold2 = 170;
	int iThreshold3 = 255;

	// 2bit Dithering
	for (int h = 0; h < int_height; h++)
	{
		memset(byte_pix_2bit, 0, int_2bit_data_size);

		TIFFReadScanline(fp_r, byte_pix_gray, h, 0);

		for (int w = 0; w < int_width; w++)
		{

			// Level 1
			if (byte_Curve_Lv1[byte_pix_gray[w]] > byte_mask[h % int_mask_col][w % int_mask_row])
			{
				byte_pix_2bit[w / 4] += 0x01 << ((3 - (w % 4)) * 2);
			}

			// Level 2
			if (byte_Curve_Lv2[byte_pix_gray[w]] > byte_mask[h % int_mask_col][w % int_mask_row])
			{
				byte_pix_2bit[w / 4] += 0x01 << ((3 - (w % 4)) * 2);
			}

			// Level 3
			if (byte_Curve_Lv3[byte_pix_gray[w]] > byte_mask[h % int_mask_col][w % int_mask_row])
			{
				byte_pix_2bit[w / 4] += 0x01 << ((3 - (w % 4)) * 2);
			}


		}

		TIFFWriteScanline(fp_w, byte_pix_2bit, h, 0);

	}

	free(byte_pix_gray);
	free(byte_pix_2bit);
#endif



#ifdef TIFFWRITE
	TIFFClose(fp_r);
	TIFFClose(fp_w);
#endif 

#ifdef BMPWRITE
	fclose(fp_r);
	fclose(fp_w);
#endif

	return 0;
}






int ApplyCurve(BYTE* byte_Curve, const char* str_CurveFileName)
{
	int iIndex_Array[256] = { 0, };
	FILE* fp;
	// Curve txt파일 Open
	fp = fopen(str_CurveFileName, "r");
	if (fp == NULL)
	{
		printf("Curve File was not found!!\n");

		system("PAUSE");

		// Curve 파일이 없을 경우 0~255 기본 값 적용
		for (int i = 0; i < 256; i++)
		{
			byte_Curve[i] = i;
		}

		return 0;
	}


	for (int i = 0; i < 256; i++)
	{
		iIndex_Array[i] = ((double)1000 / (double)255 * (double)i);
		//printf("%d | ", iIndex_Array[i]);
	}
	//system("PAUSE");

	// Curve 파일이 있을 경우 데이터를 읽고 배열에 저장한다.
	double dRead_In = 0;
	double dRead_Out = 0;
	char cRead[20] = { 0, };
	char c = 0;
	int iIndex = 0;

	while (!feof(fp))
	{
		fgets(cRead, 20, fp);
		sscanf(cRead, "%c%lf%c%c%lf%c", &c, &dRead_In, &c, &c, &dRead_Out, &c);
		//printf("%lf\n", dRead_In);

		if (dRead_Out < 0)
			dRead_Out = 0;
		if (dRead_Out > 1000)
			dRead_Out = 1000;

		if (dRead_In == iIndex_Array[iIndex])
		{
			byte_Curve[iIndex] = (int)((dRead_Out / (double)1000) * (double)255);
			iIndex++;
		}
		if (iIndex > 255)
			break;

		//byte_Curve[(int)((dRead_In / (double)1000) * (double)255)] = (int)((dRead_Out / (double)1000) * (double)255);
	}

	fclose(fp);


	//for (int i = 0; i < 256; i++)
	//{
	//	printf("%d | ", byte_Curve[i]);
	//}
	//system("PAUSE");

	// 커브 데이터 txt파일로 쓴다.
	{
		FILE* fp = fopen("CurveArray.txt", "w");

		for (int i = 0; i < 256; i++)
		{
			fprintf(fp, "%d,", byte_Curve[i]);
		}

		fclose(fp);
	}


	return 0;
}