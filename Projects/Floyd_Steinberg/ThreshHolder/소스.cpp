#include <stdio.h>
#include <Windows.h>
#include <stdlib.h>

#pragma warning(disable : 4996)

BITMAPFILEHEADER bfh;
BITMAPINFOHEADER bih;

RGBQUAD* rgb;
unsigned char * pix;

int main(void)
{
	FILE * fp;
	int err;
	
	fp = fopen("EDIMAGE.bmp", "rb");

	if (fp == NULL)
	{
		printf("File not found!!\n");
		return 0;
	}

	fread(&bfh, sizeof(bfh), 1, fp);
	fread(&bih, sizeof(bih), 1, fp);

	rgb = (RGBQUAD*)malloc(sizeof(RGBQUAD) * 256);
	fread(rgb, sizeof(RGBQUAD), 256, fp);

	pix = (unsigned char *)malloc(sizeof(unsigned char) * bih.biWidth * bih.biHeight);

	fread(pix, 1, bih.biWidth * bih.biHeight, fp);
	err = 0;
	for (int i = 0; i < bih.biWidth * bih.biHeight; i++)
	{	
		err = err + pix[i];
		if (err >= 255)
		{
			pix[i] = 255;
			err = err - 255;

		}
		else
		{
			pix[i] = 0;
		}
		/*
		if (pix[i] >= 127) // ThreshHolder의 기준값 설정
		{
			pix[i] = 0;
		}
		else 
		{
			pix[i] = 255;
		}
		//printf("%d\n", pix[i]);
		*/
	}


	HDC hdc;
	hdc = GetDC(NULL);

	
	SetDIBitsToDevice(hdc, 0, 0, bih.biWidth, bih.biHeight, 0, 0, 0, bih.biHeight,
		(BYTE*)pix, (const BITMAPINFO *)&bih, DIB_PAL_COLORS);

	ReleaseDC(NULL, hdc);

	free(rgb);
	free(pix);
	fclose(fp);

	return 0;
}