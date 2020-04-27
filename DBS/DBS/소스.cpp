#include <stdio.h>
#include <process.h>
#include <Windows.h>
#include <string.h>
#include <tchar.h>
#include <stdbool.h>
#include <stdlib.h>
#pragma warning(disable : 4996)

// 시간을 계산하기 위한 변수
double total_Time_GPU, total_Time_CPU, tmp_time;
LARGE_INTEGER beginClock, endClock, clockFreq;
LARGE_INTEGER tot_beginClock, tot_endClock, tot_clockFreq;

// 비트맵 이미지 정보를 받아오기 위한 구조체 변수
BITMAPFILEHEADER bfh;
BITMAPINFOHEADER bih;
RGBQUAD * rgb;

// 이미지 정보를 다루기 위해 사용하는 변수
int bpl, bph;
unsigned char* pix; // 원본 이미지 픽셀
unsigned char* pix_ms; // 디더링을 할 픽셀값
int * pixE;

char str[100];				// 파일명을 담을 문자열


void GaussianFilter(float thresh);		// 가우시안 필터 적용.
void FwriteCPU(char *);		// 연산된 픽셀값을 bmp파일로 저장하는 함수
void DBS();					// Direct Binary Search 연산
void Swap(int p1, int p2);	// p1, p2 좌표에 있는 픽셀값 교환

int main(void)
{
	FILE * fp;
	fp = fopen("EDIMAGE.bmp", "rb");

	fread(&bfh, sizeof(bfh), 1, fp);
	fread(&bih, sizeof(bih), 1, fp);

	rgb = (RGBQUAD*)malloc(sizeof(RGBQUAD) * 256);
	fread(rgb, sizeof(RGBQUAD), 256, fp);

	// BPL을 맞춰주기 위해서 픽셀데이터의 사이즈를 4의 배수로 조정
	bpl = (bih.biWidth + 3) / 4 * 4;
	bph = (bih.biHeight + 3) / 4 * 4;

	pix = (unsigned char *)malloc(sizeof(unsigned char) * bpl * bph);
	memset(pix, 0, sizeof(unsigned char) * bpl * bph);
	fread(pix, sizeof(unsigned char), bpl * bph, fp);

	pix_ms = (unsigned char *)malloc(sizeof(unsigned char) * bpl * bph);
	memset(pix_ms, 0, sizeof(unsigned char) * bpl * bph);
	//memcpy(pix_ms, pix, sizeof(unsigned char) * bpl * bph);

	pixE = (int *)malloc(sizeof(int) * bpl * bph);
	memset(pixE, 0, sizeof(int) * bpl * bph);

	QueryPerformanceFrequency(&tot_clockFreq);	// 시간을 측정하기위한 준비
	total_Time_CPU = 0;

	
	QueryPerformanceCounter(&tot_beginClock); // 시간측정 시작
	// Direct Binary Search 디더링
	//DBS();
	GaussianFilter(256);
	QueryPerformanceCounter(&tot_endClock);

	total_Time_CPU = (double)(tot_endClock.QuadPart - tot_beginClock.QuadPart) / tot_clockFreq.QuadPart;
	printf("Total processing Time_CPU_Single : %f ms\n", total_Time_CPU * 1000);
	system("pause");


	sprintf(str, "DBS_Dither.bmp");
	FwriteCPU(str);




	free(rgb);
	free(pix);
	free(pix_ms);
	free(pixE);
	fclose(fp);
	
	return 0;
}



void DBS()
{
	int quant_error = 0;	// 초기 디더링 작업을 할때 쓰일 에러 가중치 변수
	int count = 0;			// 최소제곱오차 값이 0이 아닐때까지 반복문을 돌리기위한 카운트 변수
	//int eps = 0;
	//int eps_min = 0;
	int pix1 = 0;
	int pix2 = 0;
	int pix_min = 0;
	int err_min = 0;
	int err = 0;
	unsigned char pix_toggle = 0;
	int flag = 0;

	// 초기 디더링 작업
	for (int y = 1; y < bph - 1; y++)
	{
		for (int x = 1; x < bpl - 1; x++)
		{
			pixE[y * bpl + x] += pix_ms[y * bpl + x];
			pix_ms[y * bpl + x] = pixE[y * bpl + x] / 128 * 255;
			quant_error = pixE[y * bpl + x] - pix_ms[y * bpl + x];

			pixE[y * bpl + x + 1] += quant_error * 7 / 16;
			pixE[(y + 1) * bpl + x - 1] += quant_error * 3 / 16;
			pixE[(y + 1) * bpl + x] += quant_error * 5 / 16;
			pixE[(y + 1) * bpl + x + 1] += quant_error * 1 / 16;
		}
	}


	// 오차제곱이 최소가 될 때까지 반복...
	while(1)
	{
		count = 0;
		for (int y = 2; y < bph - 2; y++)
		{
			for (int x = 2; x < bpl - 2; x++)
			{
				flag = 0;
				pix1 = y * bpl + x;
				pix2 = y * bpl + x;
				// 현재픽셀값을 토글링
				//pix_toggle = (unsigned char)(pix_ms[y * bpl + x] + 255) / 255 * 255;
				//pixE[y * bpl + x] *= -1;
				err_min = pix_ms[pix1] - pix[pix2];
				// 현재픽셀과 인접한 8개의 픽셀과 교환하는 작업		
				pix1 = (y - 1) * bpl + x - 1;
				//Swap(pix1, pix2);
				err = pix_ms[pix1] - pix[pix2];
				if (err < err_min)
				{
					pix_min = pix1;
					err_min = err;
					//Swap(pix1, pix2);
					flag = 1;
				}								
				pix1 = (y - 1) * bpl + x;
				//Swap(pix1, pix2);
				err = pix_ms[pix1] - pix[pix2];
				if (err < err_min)
				{
					pix_min = pix1;
					err_min = err;
					//Swap(pix1, pix2);
					flag = 1;
				}					
				pix1 = (y - 1) * bpl + x + 1;
				//Swap(pix1, pix2);
				err = pix_ms[pix1] - pix[pix2];
				if (err < err_min)
				{
					pix_min = pix1;
					err_min = err;
					//Swap(pix1, pix2);
					flag = 1;
				}					
				pix1 = y * bpl + x - 1;
				//Swap(pix1, pix2);
				err = pix_ms[pix1] - pix[pix2];
				if (err < err_min)
				{
					pix_min = pix1;
					err_min = err;
					//Swap(pix1, pix2);
					flag = 1;
				}
				pix1 = y * bpl + x + 1;
				//Swap(pix1, pix2);
				err = pix_ms[pix1] - pix[pix2];
				if (err < err_min)
				{
					pix_min = pix1;
					err_min = err;
					//Swap(pix1, pix2);
					flag = 1;
				}					
				pix1 = (y + 1) * bpl + x - 1;
				//Swap(pix1, pix2);
				err = pix_ms[pix1] - pix[pix2];
				if (err < err_min)
				{
					pix_min = pix1;
					err_min = err;
					//Swap(pix1, pix2);
					flag = 1;
				}
				pix1 = (y + 1) * bpl + x + 1;
				//Swap(pix1, pix2);
				err = pix_ms[pix1] - pix[pix2];
				if (err < err_min)
				{
					pix_min = pix1;
					err_min = err;
					//Swap(pix1, pix2);
					flag = 1;
				}	
				if (flag == 0)
					pix_ms[y * bpl + x] = (unsigned char)(pix_ms[y * bpl + x] + 255) / 255 * 255;
				else
					Swap(pix_min, pix2);
				//if (err_min < 0)
					//count++;
			}
		}

		if (count == 0)
			break;
	}
}
void GaussianFilter(float thresh)
{
	float g = thresh;
	float a = 1 / g;
	float b = 4 / g;
	float c = 6 / g;
	float d = 16 / g;
	float e = 24 / g;
	float f = 36 / g;
	// 가우시안 필터 마스크 배열
	float G[5][5] =
	{
		a, b, c, b, a,
		b, d, e, d, b,
		c, e, f, e, c,
		b, d, e, d, b,
		a, b, c, b, a
	};

	for (int y = 2; y < bph - 2; y++)
	{
		for (int x = 2; x < bpl - 2; x++)
		{
			for (int k = 0; k <= 4; k++)
			{
				for (int l = 0; l < 4; l++)
				{
					pix_ms[y * bpl + x] += pix[(y + k - 2) * bpl + (x + l - 2)] * G[k][l];
				}
			}
		}
	}


}

// 픽셀값들을 교환하는 함수
void Swap(int p1, int p2)
{
	
	unsigned char pix_tmp = 0;
	pix_tmp = pix_ms[p1];
	pix_ms[p1] = pix_ms[p2];
	pix_ms[p2] = pix_tmp;
	/*
	int pixE_tmp = 0;
	pixE_tmp = pixE[p1];
	pixE[p1] = pixE[p2];
	pixE[p2] = pixE_tmp;
	*/
}

void FwriteCPU(char * fn)
{
	// 데이터 픽셀값을 bmp파일로 쓴다.
	FILE * fp2 = fopen(fn, "wb");
	fwrite(&bfh, sizeof(bfh), 1, fp2);
	fwrite(&bih, sizeof(bih), 1, fp2);
	fwrite(rgb, sizeof(RGBQUAD), 256, fp2);

	fwrite(pix_ms, sizeof(unsigned char), bpl * bph, fp2);
	//fwrite(pixE, sizeof(int), bpl * bph, fp2);
	fclose(fp2);
}