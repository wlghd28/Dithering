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
unsigned char* pix; // 원본 이미지
unsigned char* pix_ms; // 가우시안 필터링을 한 이미지
unsigned char* pix_hvs; // 하프톤 이미지
double* err;
int * pixE;

// 가우시안 필터 가중치
double g = 256;
double a = 1 / g;
double b = 4 / g;
double c = 6 / g;
double d = 12 / g;
double e = 24 / g;
double f = 36 / g;

// 가우시안 필터 마스크 배열
double G[5][5] =
{
	a, b, c, b, a,
	b, d, e, d, b,
	c, e, f, e, c,
	b, d, e, d, b,
	a, b, c, b, a
};

double CPP[9][9];
int halfcppsize = 4;
double* CEP;

char str[100];				// 파일명을 담을 문자열

//void GaussianFilter(float thresh);		// 가우시안 필터
void DBS();					// Direct Binary Search 연산
void CONV();	// 2차원 컨볼루션 연산
void XCORR();	// 상호상관관계 연산
void FwriteCPU(char *);		// 연산된 픽셀값을 bmp파일로 저장하는 함수

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

	pix_hvs = (unsigned char *)malloc(sizeof(unsigned char) * (bpl + 5) * (bph + 5) * 2);
	memset(pix_hvs, 0, sizeof(unsigned char) * (bpl + 5) * (bph + 5) * 2);
	memcpy(pix_hvs, pix, sizeof(unsigned char) * bpl * bph);

	err = (double *)malloc(sizeof(double) * bpl * bph);
	memset(err, 0, sizeof(double) * bpl * bph);

	CEP = (double *)malloc(sizeof(double) * (bpl + 5) * (bph + 5) * 2);
	memset(CEP, 0, sizeof(double) * (bpl + 5) * (bph + 5) * 2);

	pixE = (int *)malloc(sizeof(int) * bpl * bph);
	memset(pixE, 0, sizeof(int) * bpl * bph);

	QueryPerformanceFrequency(&tot_clockFreq);	// 시간을 측정하기위한 준비
	total_Time_CPU = 0;

	QueryPerformanceCounter(&tot_beginClock); // 시간측정 시작
	// Direct Binary Search 디더링
	DBS();
	QueryPerformanceCounter(&tot_endClock);

	total_Time_CPU = (double)(tot_endClock.QuadPart - tot_beginClock.QuadPart) / tot_clockFreq.QuadPart;
	printf("Total processing Time_CPU_Single : %f ms\n", total_Time_CPU * 1000);
	system("pause");

	sprintf(str, "DBS_Dither.bmp");
	FwriteCPU(str);

	free(rgb);
	free(pix);
	free(pix_ms);
	free(pix_hvs);
	free(err);
	free(CEP);
	free(pixE);
	fclose(fp);
	
	return 0;
}

void DBS()
{
	int quant_error = 0;	// 초기 디더링 작업을 할때 쓰일 에러 가중치 변수
	int count = 0;			// 최소제곱오차 값이 0이 아닐때까지 반복문을 돌리기위한 카운트 변수
	double eps = 0;
	double eps_min = 0;
	double a0 = 0;
	double a1 = 0;
	double a0c = 0;
	double a1c = 0;
	unsigned char cpx = 0;
	unsigned char cpy = 0;

	// 초기 디더링 작업 (양방향 Floyd and Steinberg Dithering) 
	for (int y = 1; y < bph - 1; y++)
	{
		if (y % 2 == 1)
		{
			for (int x = 1; x < bpl - 1; x++)
			{
				pixE[y * bpl + x] += pix_hvs[y * bpl + x];
				pix_hvs[y * bpl + x] = pixE[y * bpl + x] / 128 * 255;
				quant_error = pixE[y * bpl + x] - pix_hvs[y * bpl + x];

				pixE[y * bpl + x + 1] += quant_error * 7 / 16;
				pixE[(y + 1) * bpl + x - 1] += quant_error * 3 / 16;
				pixE[(y + 1) * bpl + x] += quant_error * 5 / 16;
				pixE[(y + 1) * bpl + x + 1] += quant_error * 1 / 16;
			}
		}
		else
		{
			for (int x = bpl - 2; x >= 1; x--)
			{
				pixE[y * bpl + x] += pix_hvs[y * bpl + x];
				pix_hvs[y * bpl + x] = pixE[y * bpl + x] / 128 * 255;
				quant_error = pixE[y * bpl + x] - pix_hvs[y * bpl + x];

				pixE[y * bpl + x - 1] += quant_error * 7 / 16;
				pixE[(y + 1) * bpl + x + 1] += quant_error * 3 / 16;
				pixE[(y + 1) * bpl + x] += quant_error * 5 / 16;
				pixE[(y + 1) * bpl + x - 1] += quant_error * 1 / 16;
			}
		}
	}

	CONV();		// 2차원 컨볼루션 연산 행렬 생성 (CPP)
	for (int y = 0; y < bph; y++)
	{
		for (int x = 0; x < bpl; x++)
		{
			err[y * bpl + x] = pix_hvs[y * bpl + x] / 255 - (double)pix[y * bpl + x] / 255;
		}
	}
	XCORR();	// 상호관계연산 행렬 생성 (CEP)

	// DBS 과정 시작..
	while(1)
	{
		count = 0;
		a0 = 0;
		a1 = 0;
		a0c = 0;
		a1c = 0;
		cpx = 0;
		cpy = 0;

		for (int i = 1; i < bph - 1; i++)
		{
			for (int j = 1; j < bpl - 1; j++)
			{
				a0c = 0;
				a1c = 0;
				cpx = 0;
				cpy = 0;
				eps_min = 0;
				for (int y = -1; y < 1; y++)
				{
					if (i + y < 1 || i + y > bph)
						continue;
					
					for (int x = -1; x < 1; x++)
					{
						if (j + x < 1 || j + x > bpl)
							continue;
						if (y == 0 && x == 0)
						{
							if (pix_hvs[i * bpl + j] == 255)
							{
								a0 = -1;
								a1 = 0;
							}
							else
							{
								a0 = 1;
								a1 = 0;
							}
						}
						else
						{
							if (pix_hvs[(i + x) * bpl + (j + y)] != pix_hvs[i * bpl + j])
							{
								if (pix_hvs[i * bpl + j] == 255)
								{
									a0 = -1;
									a1 = (-1) * a0;
								}
								else
								{
									a0 = 1;
									a1 = (-1) * a0;
								}
							}
							else
							{
								a0 = 0;
								a1 = 0;
							}
						}
						eps = (a0 * a0 + a1 * a1) * CPP[halfcppsize + 1][halfcppsize + 1]
							+ 2 * a0 * a1 * CPP[halfcppsize + y + 1][halfcppsize + x + 1]
							+ 2 * a0 * CEP[(i + halfcppsize) * (bpl + 5 - 1) + (j + halfcppsize)]
							+ 2 * a1 * CEP[(i + y + halfcppsize) * (bpl + 5 - 1) + (j + x + halfcppsize)];
						if (eps_min > eps)
						{
							eps_min = eps;
							a0c = a0;
							a1c = a1;
							cpx = x;
							cpy = y;
						}
					}
				}
				if (eps_min < 0)
				{
					for (int y = (-1) * halfcppsize; y < halfcppsize; y++)
					{
						for (int x = (-1) * halfcppsize; x < halfcppsize; x++)
						{
							CEP[(i + y + halfcppsize) * (bpl + 5 - 1) + (j + x + halfcppsize)] += a0c * CPP[y + halfcppsize + 1][x + halfcppsize + 1];
						}
					}
					for (int y = (-1) * halfcppsize; y < halfcppsize; y++)
					{
						for (int x = (-1) * halfcppsize; x < halfcppsize; x++)
						{
							CEP[(i + y + cpy + halfcppsize) * (bpl + 5 - 1) + (j + x + cpx + halfcppsize)] += a1c * CPP[y + halfcppsize + 1][x + halfcppsize + 1];
							//printf("%d\n", (i + y + cpy + halfcppsize) * (bpl + 5 - 1) + (j + x + cpx + halfcppsize));
						}
					}
					pix_hvs[i * bpl + j] += (unsigned char)(a0c) * 255;
					pix_hvs[(cpy + i) * bpl + (j + cpx)] += (unsigned char)(a1c) * 255;
					//printf("%d\n", (cpy + i) * bpl + (j + cpx));
					count++;
				}
			}
		}
		printf("%d\n", count);
		if (count == 0)
			break;
	}
}
// 2차원 컨볼루션 연산
void CONV()
{
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			CPP[i][j] = 0;
		}
	}
	double sum = 0;
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			sum = 0;
			for(int k = -2; k < 3; k++)
			{
				for (int l = -2; l < 3; l++)
				{
					if(i + k >= 0 && j + l < 5)
						sum += G[i + k][j + l] * G[i][j];
				}
			}
			CPP[i + 2][j + 2] = sum;
		}
	}
}
// 상호상관관계 연산
void XCORR()
{
	double sum = 0;
	for (int y = -4; y < bph; y++)
	{
		for (int x = -4; x < bpl; x++)
		{
			sum = 0;
			for (int i = y; i < y + 5; i++)
			{
				for (int j = x; j < x + 5; j++)
				{
					if ((i >= 0 && j >= 0) && (i < bph && j < bpl))
					{
						sum += CPP[i - y][j - x] * err[i * bpl + j];
					}
				}
			}
			CEP[(y + 4) * (bpl + 5 - 1) + (x + 4)] = (double)sum;
		}
	}
}

/*
// 가우시안 필터
void GaussianFilter(float thresh)
{
	float g = thresh;
	// 5 X 5 마스크
	float a = 1 / g;
	float b = 4 / g;
	float c = 6 / g;
	float d = 12 / g;
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
	for (int y = 1; y < bph - 1; y++)
	{
		for (int x = 1; x < bpl - 1; x++)
		{
			for (int k = 0; k < 5; k++)
			{
				for (int l = 0; l < 5; l++)
				{
					pix_ms[y * bpl + x] += pix[(y + k - 2) * bpl + (x + l - 2)] * G[k][l];
				}
			}
		}
	}
	
	// 3 X 3 마스크
	float a = 0 / g;
	float b = 1 / g;
	float c = 2 / g;
	float G[3][3] =
	{
		a, b, a,
		b, c, b,
		a, b, a
	};
	for (int y = 1; y < bph - 1; y++)
	{
		for (int x = 1; x < bpl - 1; x++)
		{
			for (int k = 0; k < 3; k++)
			{
				for (int l = 0; l < 3; l++)
				{
					pix_ms[y * bpl + x] += pix[(y + k - 1) * bpl + (x + l - 1)] * G[k][l];
				}
			}
		}
	}
}
*/

void FwriteCPU(char * fn)
{
	// 데이터 픽셀값을 bmp파일로 쓴다.
	FILE * fp2 = fopen(fn, "wb");
	fwrite(&bfh, sizeof(bfh), 1, fp2);
	fwrite(&bih, sizeof(bih), 1, fp2);
	fwrite(rgb, sizeof(RGBQUAD), 256, fp2);

	fwrite(pix_hvs, sizeof(unsigned char), bpl * bph, fp2);
	//fwrite(pixE, sizeof(int), bpl * bph, fp2);
	fclose(fp2);
}