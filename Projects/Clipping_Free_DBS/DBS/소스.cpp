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
int bpl, pad;
int width, height;
long pix_size;
unsigned char* pix; // 원본 이미지
//unsigned char* pix_ms; // 가우시안 필터링을 한 이미지
unsigned char* pix_hvs; // 하프톤 이미지
unsigned char* pix_check;
unsigned char trash[3] = { 0 }; // 쓰레기 값
//double* err;
//int * pixE;

int D = 12;
double pi = 3.14159265358979323846264338327950288419716939937510;

double G[7][7];
int fs = 7;	// 가우시안 필터 사이즈
double CPP[13][13];
int halfcppsize = 6;

/*
double G[11][11];
int fs = 11;	// 가우시안 필터 사이즈
double CPP[21][21];
int halfcppsize = 10;
*/
double* CEP;

char str[100];				// 파일명을 담을 문자열

//void GaussianFilter(float thresh);		// 가우시안 필터
void DBS();					// Direct Binary Search 연산
void GaussianFilter();		// 가우시안 필터 생성
double GaussianRandom(double stddev, double average);		// 정규분포 난수 생성
void CONV();	// 2차원 컨볼루션 연산
void XCORR();	// 상호상관관계 연산
void Halftone();				// 초기 하프톤 이미지 생성
void Dither();				// 디더링 작업
void Fread(FILE *fp);				// 파일포인터에 있는 파일을 읽는 함수
void Fwrite(char *);		// 연산된 픽셀값을 bmp파일로 저장하는 함수

int main(void)
{
	FILE * fp;
	fp = fopen("input.bmp", "rb");
	//fp = fopen("../Images/newEDIMAGE.bmp", "rb");
	//fp = fopen("../Images/newEDIMAGE2.bmp", "rb");
	//fp = fopen("../Images/test.bmp", "rb");
	//fp = fopen("../Images/bird2_720_C.bmp", "rb");

	if (fp == NULL)
	{
		printf("File not found!!\n");
		return 0;
	}

	Fread(fp);	// 파일을 읽어서 데이터를 추출

	// BPL을 맞춰주기 위해서 픽셀데이터의 사이즈를 4의 배수로 조정
	bpl = (bih.biWidth + 3) / 4 * 4;
	width = bih.biWidth;
	height = bih.biHeight;
	pix_size = width * height;
	pad = bpl - width;
	printf("width : %d , height : %d\n", width, height);
	printf("image size : %d X %d\n", width, height);

	pix = (unsigned char *)calloc(pix_size, sizeof(unsigned char));

	for (int i = 0; i < height; i++)
	{
		fread(pix + (i * width), sizeof(unsigned char), width, fp);
		fread(&trash, sizeof(unsigned char), pad, fp);
	}

	//pix_ms = (unsigned char *)calloc(bpl * bph, sizeof(unsigned char));
	//memcpy(pix_ms, pix, sizeof(unsigned char) * bpl * bph);

	pix_hvs = (unsigned char *)calloc(pix_size, sizeof(unsigned char));
	//memcpy(pix_hvs, pix, sizeof(unsigned char) * bpl * bph);
	pix_check = (unsigned char *)calloc(pix_size, sizeof(unsigned char));

	CEP = (double *)calloc((width + halfcppsize * 2) * (height + halfcppsize * 2), sizeof(double));
	printf("Memory of CEP : %dbyte, pix : %dbyte, pix_hvs : %dbyte \n", _msize(CEP), _msize(pix), _msize(pix_hvs));

	// Memory allocation 예외 처리
	if (CEP == NULL || pix == NULL || pix_hvs == NULL || rgb == NULL)
	{
		printf("Memory allocation fail!\n");
		return 0;
	}

	QueryPerformanceFrequency(&tot_clockFreq);	// 시간을 측정하기위한 준비

	total_Time_CPU = 0;
	QueryPerformanceCounter(&tot_beginClock); // 시간측정 시작
	// Direct Binary Search 디더링
	DBS();
	QueryPerformanceCounter(&tot_endClock);

	total_Time_CPU = (double)(tot_endClock.QuadPart - tot_beginClock.QuadPart) / tot_clockFreq.QuadPart;
	printf("Total processing Time_DBS : %f ms\n", total_Time_CPU * 1000);
	//system("pause");

	//sprintf(str, "../Images/new_DBS_Dither.bmp");
	//sprintf(str, "../Images/new_DBS_Dither2.bmp");
	//sprintf(str, "../Images/test_DBS_Dither.bmp");
	//sprintf(str, "../Images/bird2_DBS_Dither_720_C.bmp");
	sprintf(str, "output.bmp");

	Fwrite(str);

	free(rgb);
	free(pix);
	//free(pix_ms);
	free(pix_hvs);
	free(pix_check);
	free(CEP);
	fclose(fp);

	system("pause");
	return 0;
}

void DBS()
{
	int count = 0;			// 최소제곱오차 값이 0이 아닐때까지 반복문을 돌리기위한 카운트 변수
	double eps = 0.0;
	double eps_min = 0.0;
	int a0 = 0;
	int a1 = 0;
	int a0c = 0;
	int a1c = 0;
	int cpx = 0;
	int cpy = 0;

	Dither();				// Clipping 현상 제거

	Halftone();				// 초기 하프톤이미지 생성

	GaussianFilter();		// 가우시안 필터 생성

	CONV();		// 2차원 컨볼루션 연산 행렬 생성 (CPP)

	XCORR();	// 상호관계연산 행렬 생성 (CEP)

	// DBS 과정 시작..
	while (1)
	{
		count = 0;
		a0 = 0;
		a1 = 0;
		a0c = 0;
		a1c = 0;
		cpx = 0;
		cpy = 0;

		for (int i = 1; i < height - 1; i++)
		{
			for (int j = 1; j < width - 1; j++)
			{
				if (pix_check[i * width + j] == 0)
				{
					a0c = 0;
					a1c = 0;
					cpx = 0;
					cpy = 0;
					eps_min = 0;
					for (int y = -1; y <= 1; y++)
					{
						//if (i + y < 0 || i + y >= bph)
							//continue;			
						for (int x = -1; x <= 1; x++)
						{
							//if (j + x < 0 || j + x >= bpl)
								//continue;
							if (y == 0 && x == 0)
							{
								if (pix_hvs[i * width + j] == 255)
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
								if (pix_hvs[(i + y) * width + (j + x)] != pix_hvs[i * width + j])
								{
									if (pix_hvs[i * width + j] == 255)
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
							eps = (a0 * a0 + a1 * a1) * CPP[halfcppsize][halfcppsize]
								+ 2 * a0 * a1 * CPP[halfcppsize + y][halfcppsize + x]
								+ 2 * a0 * CEP[(i + halfcppsize) * (width + halfcppsize * 2) + (j + halfcppsize)]
								+ 2 * a1 * CEP[(i + y + halfcppsize) * (width + halfcppsize * 2) + (j + x + halfcppsize)];
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
					if (eps_min < 0.0)
					{
						for (int y = (-1) * halfcppsize; y <= halfcppsize; y++)
						{
							for (int x = (-1) * halfcppsize; x <= halfcppsize; x++)
							{
								CEP[(i + y + halfcppsize) * (width + halfcppsize * 2) + (j + x + halfcppsize)] += (double)a0c * CPP[y + halfcppsize][x + halfcppsize];
								CEP[(i + y + cpy + halfcppsize) * (width + halfcppsize * 2) + (j + x + cpx + halfcppsize)] += (double)a1c * CPP[y + halfcppsize][x + halfcppsize];
							}
						}
						//pix_hvs[i * width + j] = (unsigned char)((pix_hvs[i * width + j] / 255 + a0c) % 2) * 255;
						//pix_hvs[(cpy + i) * width + (j + cpx)] = (unsigned char)((pix_hvs[(cpy + i) * width + (j + cpx)] / 255 + a1c) % 2) * 255;
						pix_hvs[i * width + j] += (unsigned char)a0c * 255;
						pix_hvs[(cpy + i) * width + (j + cpx)] += (unsigned char)a1c * 255;
						count++;
					}
				}
			}
		}
		printf("%d\n", count);
		if (count == 0)
			break;
	}
}
// 가우시안 필터 생성
void GaussianFilter()
{
	//double sum = 0;
	double d = (fs - 1) / 6 /*1.2*/;		// sigma
	double c;
	int gaulen = (fs - 1) / 2;

	for (int k = (-1) * gaulen; k <= gaulen; k++)
	{
		for (int l = (-1) * gaulen; l <= gaulen; l++)
		{
			// 기존 가우시안 분포 공식	

			c = (k * k + l * l) / (2 * d * d);
			G[k + gaulen][l + gaulen] = exp((-1) * c) / (2 * pi * d * d);
			//sum += G[k + gaulen][l + gaulen];		


			// fs = 11 일때 결과가 가장 좋았음..
			/*
			c = (k * k + l * l) / (2 * d * d) + 1;
			G[k + gaulen][l + gaulen] = 1 / c;
			//sum += G[k + gaulen][l + gaulen];
			*/
		}
	}
	//printf("%lf\n", sum);
}
// 2차원 컨볼루션 연산
void CONV()
{
	double sum = 0;
	for (int y = (-1) * fs + 1; y < fs; y++)
	{
		for (int x = (-1) * fs + 1; x < fs; x++)
		{
			sum = 0;
			for (int i = y; i < y + fs; i++)
			{
				for (int j = x; j < x + fs; j++)
				{
					if ((i >= 0 && j >= 0) && (i < fs && j < fs))
					{
						sum += (double)G[i - y][j - x] * G[i][j];
					}
				}
			}
			CPP[(y + fs - 1)][(x + fs - 1)] = (double)sum;
		}
	}
}
// 상호상관관계 연산
void XCORR()
{
	double sum = 0;
	for (int y = (-1) * halfcppsize * 2; y < height; y++)
	{
		for (int x = (-1) * halfcppsize * 2; x < width; x++)
		{
			sum = 0;
			for (int i = y; i <= y + halfcppsize * 2; i++)
			{
				for (int j = x; j <= x + halfcppsize * 2; j++)
				{
					if ((i >= 0 && j >= 0) && (i < height && j < width))
					{
						// err * CPP
						sum += (double)CPP[i - y][j - x] * ((double)pix_hvs[i * width + j] / 255 - (double)pix[i * width + j] / 255);
					}
				}
			}
			CEP[(y + halfcppsize * 2) * (width + halfcppsize * 2) + (x + halfcppsize * 2)] = (double)sum;
		}
	}
}

// 가우시안 필터
double GaussianRandom(double stddev, double average)
{
	double v1, v2, s, temp;

	do {
		v1 = 2 * ((double)rand() / RAND_MAX) - 1;      // -1.0 ~ 1.0 까지의 값
		v2 = 2 * ((double)rand() / RAND_MAX) - 1;      // -1.0 ~ 1.0 까지의 값
		s = v1 * v1 + v2 * v2;
	} while (s >= 1 || s == 0);

	s = sqrt((-2 * log(s)) / s);

	temp = v1 * s;
	temp = (stddev * temp) + average;


	return temp;
}
void Halftone()
{
	// 정규분포 난수로 하프톤이미지 생성
	srand(time(NULL));
	double tmp;

	for (int y = 1; y < height; y++)
	{
		for (int x = 1; x < width; x++)
		{
			tmp = (double)GaussianRandom(1.0, 0);
			//printf("%lf\n", tmp);
			if (tmp > 0)
			{
				if (pix_check[y * width + x] == 0)
					pix_hvs[y * width + x] = 255;
			}
		}
	}
}
void Dither()
{
	BITMAPFILEHEADER bfh_bnm;
	BITMAPINFOHEADER bih_bnm;
	RGBQUAD * rgb_bnm;
	unsigned char pix_bnm[256][256];
	int ms = 256;
	FILE* fp_bnm;
	fp_bnm = fopen("BNM_256.bmp", "rb");
	if (fp_bnm == NULL)
	{
		printf("Maskfile Not Found!!\n");
		system("pause");
		exit(0);
	}
	fread(&bfh_bnm, sizeof(bfh), 1, fp_bnm);
	fread(&bih_bnm, sizeof(bih), 1, fp_bnm);
	rgb_bnm = (RGBQUAD*)malloc(sizeof(RGBQUAD) * 256);
	fread(rgb, sizeof(RGBQUAD), 256, fp_bnm);

	for (int i = 0; i < ms; i++)
	{
		fread(pix_bnm[i], sizeof(unsigned char), ms, fp_bnm);
	}
	fclose(fp_bnm);


	// Ordered Dithering based BNM(Blue Noise Mask)
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			if (pix[i * width + j] < D)
			{
				pix_check[i * width + j] = 1;
				if (pix[i * width + j] >= pix_bnm[i % ms][j % ms] && pix[i * width + j] != 0)
					pix_hvs[i * width + j] = 255;
			}
			else if (pix[i * width + j] > 255 - D)
			{
				pix_check[i * width + j] = 1;
				if (pix[i * width + j] >= pix_bnm[i % ms][j % ms] && pix[i * width + j] != 0)
					pix_hvs[i * width + j] = 255;
			}
		}
	}

}
void Fread(FILE *fp)
{
	fread(&bfh, sizeof(bfh), 1, fp);
	fread(&bih, sizeof(bih), 1, fp);

	rgb = (RGBQUAD*)malloc(sizeof(RGBQUAD) * 256);
	fread(rgb, sizeof(RGBQUAD), 256, fp);
}
void Fwrite(char * fn)
{
	FILE * fp2 = fopen(fn, "wb");
	fwrite(&bfh, sizeof(bfh), 1, fp2);
	fwrite(&bih, sizeof(bih), 1, fp2);
	fwrite(rgb, sizeof(RGBQUAD), 256, fp2);
	for (int i = 0; i < height; i++)
	{
		fwrite(pix_hvs + (i * width), sizeof(unsigned char), width, fp2);
		fwrite(&trash, sizeof(unsigned char), pad, fp2);
	}
	fclose(fp2);
}