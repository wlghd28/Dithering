// Blue Noise Mask ����
#include <stdio.h>
#include <process.h>
#include <Windows.h>
#include <string.h>
#include <tchar.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <malloc.h>
#include "mask.h"

#pragma warning(disable : 4996)
// �ð��� ����ϱ� ���� ����
double total_Time;
LARGE_INTEGER beginClock, endClock, clockFreq;
LARGE_INTEGER tot_beginClock, tot_endClock, tot_clockFreq;

// ��Ʈ�� �̹��� ������ �޾ƿ��� ���� ����ü ����
BITMAPFILEHEADER bfh;
BITMAPINFOHEADER bih;
RGBQUAD * rgb;

// �̹��� ������ �ٷ�� ���� ����ϴ� ����
int bpl, pad;
int width, height;
long pix_size;
unsigned char trash[3] = { 0 }; // ������ ��
unsigned char* pix; // ���� �̹���
unsigned char* pix_h; // ������ �̹���

void Fread();	// bmp ���� �б�
void Fwrite(const char* fn);	// bmp ���� ����

int main(void)
{
	Fread();
	BITMAPFILEHEADER bfh_bnm;
	BITMAPINFOHEADER bih_bnm;
	RGBQUAD * rgb_bnm;
	unsigned char* pix_bnm;
	int ms = 512;

	pix_h = (unsigned char *)calloc(pix_size, sizeof(unsigned char));
	pix_bnm = (unsigned char *)calloc(ms * ms, sizeof(unsigned char));
	FILE* fp;
	fp = fopen("BNM_512.bmp", "rb");
	if (fp == NULL)
	{
		printf("Maskfile Not Found!!\n");
		system("pause");
		exit(0);
	}
	fread(&bfh_bnm, sizeof(bfh), 1, fp);
	fread(&bih_bnm, sizeof(bih), 1, fp);
	rgb_bnm = (RGBQUAD*)malloc(sizeof(RGBQUAD) * 256);
	fread(rgb_bnm, sizeof(RGBQUAD), 256, fp);


	for (int i = 0; i < ms; i++)
	{
		fread(pix_bnm + (ms * i), sizeof(unsigned char), ms, fp);
	}

	fclose(fp);

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{	
			
			/*
			if (pix[y * width + x] != 0 && pix[y * width + x] >= pix_bnm[(y % ms) * ms + (x % ms)])
			{
				pix_h[y * width + x] = 255;
			}
			*/
			
			if (pix[y * width + x] != 0 && pix[y * width + x] >= mask_BNM_c[(y % 512)][(x % 512)])
			{
				pix_h[y * width + x] = 255;
			}
			
		}
	}

	Fwrite("output_BNM_new.bmp");
	free(rgb);
	free(rgb_bnm);
	free(pix);
	free(pix_h);
	free(pix_bnm);

	return 0;
}

void Fread()
{
	FILE * fp;
	fp = fopen("input.bmp", "rb");

	if (fp == NULL)
	{
		printf("File not found!!\n");
		system("pause");
		exit(0);
	}

	fread(&bfh, sizeof(bfh), 1, fp);
	fread(&bih, sizeof(bih), 1, fp);
	bpl = (bih.biWidth + 3) / 4 * 4;
	width = bih.biWidth;
	height = bih.biHeight;
	pix_size = width * height;
	pad = bpl - width;
	printf("width : %d , height : %d\n", width, height);
	printf("image size : %d X %d\n", width, height);

	rgb = (RGBQUAD*)malloc(sizeof(RGBQUAD) * 256);
	fread(rgb, sizeof(RGBQUAD), 256, fp);

	pix = (unsigned char *)calloc(pix_size, sizeof(unsigned char));
	for (int i = 0; i < height; i++)
	{
		fread(pix + (i * width), sizeof(unsigned char), width, fp);
		fread(&trash, sizeof(unsigned char), pad, fp);
	}
	fclose(fp);
}
void Fwrite(const char* fn)
{
	FILE * fp = fopen(fn, "wb");
	fwrite(&bfh, sizeof(bfh), 1, fp);
	fwrite(&bih, sizeof(bih), 1, fp);
	fwrite(rgb, sizeof(RGBQUAD), 256, fp);
	for (int i = 0; i < height; i++)
	{
		fwrite(pix_h + (i * width), sizeof(unsigned char), width, fp);
		fwrite(&trash, sizeof(unsigned char), pad, fp);
	}
	fclose(fp);
}
