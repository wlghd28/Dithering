#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "blueNoise.h"

// 비트맵 이미지 정보를 받아오기 위한 구조체 변수
BITMAPFILEHEADER bfh;
BITMAPINFOHEADER bih;
RGBQUAD * rgb;
int ms = 256;

int main(void)
{
	FILE *fp = NULL;
	fp = fopen("BNM_256.bmp", "rb");

	fread(&bfh, sizeof(bfh), 1, fp);
	fread(&bih, sizeof(bih), 1, fp);
	rgb = (RGBQUAD*)malloc(sizeof(RGBQUAD) * 256);
	fread(rgb, sizeof(RGBQUAD), 256, fp);
	fclose(fp);

	fp = fopen("new_BNM_256.bmp", "wb");
	fwrite(&bfh, sizeof(bfh), 1, fp);
	fwrite(&bih, sizeof(bih), 1, fp);
	fwrite(rgb, sizeof(RGBQUAD), 256, fp);
	for (int i = 0; i < ms; i++)
	{
		fwrite(blueNoise[i], sizeof(unsigned char), ms, fp);
	}
	fclose(fp);

	free(rgb);
	return 0;
}