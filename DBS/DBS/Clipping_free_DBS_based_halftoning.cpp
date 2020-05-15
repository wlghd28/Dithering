#include <stdio.h>
#include <process.h>
#include <Windows.h>
#include <string.h>
#include <tchar.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>


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
//unsigned char* pix_ms; // 가우시안 필터링을 한 이미지
unsigned char* pix_hvs; // 하프톤 이미지
double* err;
int * pixE;
/*
int T[8][8] =
{
	0, 48, 12, 60, 3, 51, 15, 63,
	32, 16, 44, 28, 35, 19, 47, 31,
	8, 56, 4, 52, 11, 59, 7, 55,
	40, 24, 3, 20, 43, 27, 39, 23,
	2, 50, 14, 62, 1, 49, 13, 61,
	34, 18, 46, 30, 33, 17, 45, 29,
	10, 58, 6, 54, 9, 57, 5, 53,
	42, 26, 38, 22, 41, 25, 37, 21
};
*/
int T[512][512];
int ts = 512;	// threshold array size

double pi = 3.14159265358979323846264338327950288419716939937510;
double G[9][9];
int fs = 9;	// 가우시안 필터 사이즈
double CPP[17][17];
int halfcppsize = 8;

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
void BayerMatrix(int n);		// 정렬 디더링 할 때 쓸 임계값 행렬 생성.

void Halftone();				// 초기 하프톤 이미지 생성
void Dither();				// 디더링 작업
void FwriteCPU(char *);		// 연산된 픽셀값을 bmp파일로 저장하는 함수

int main(void)
{
	FILE * fp;
	//fp = fopen("EDIMAGE.bmp", "rb");
	//fp = fopen("newEDIMAGE2.bmp", "rb");
	fp = fopen("test.bmp", "rb");
	//fp = fopen("bird_K.bmp", "rb");

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

	/*
	pix_ms = (unsigned char *)malloc(sizeof(unsigned char) * bpl * bph);
	memset(pix_ms, 0, sizeof(unsigned char) * bpl * bph);
	memcpy(pix_ms, pix, sizeof(unsigned char) * bpl * bph);
	*/
	pix_hvs = (unsigned char *)malloc(sizeof(unsigned char) * bpl * bph);
	memset(pix_hvs, 0, sizeof(unsigned char) * bpl * bph);
	//memcpy(pix_hvs, pix, sizeof(unsigned char) * bpl * bph);

	err = (double *)malloc(sizeof(double) * bpl * bph);
	memset(err, 0, sizeof(double) * bpl * bph);

	CEP = (double *)malloc(sizeof(double) * (bpl + halfcppsize * 2) * (bph + halfcppsize * 2));
	memset(CEP, 0, sizeof(double) * (bpl + halfcppsize * 2) * (bph + halfcppsize * 2));

	pixE = (int *)malloc(sizeof(int) * bpl * bph);
	memset(pixE, 0, sizeof(int) * bpl * bph);

	QueryPerformanceFrequency(&tot_clockFreq);	// 시간을 측정하기위한 준비

	total_Time_CPU = 0;
	QueryPerformanceCounter(&tot_beginClock); // 시간측정 시작
	//Dither();
	// Direct Binary Search 디더링
	Halftone();
	//DBS();
	QueryPerformanceCounter(&tot_endClock);

	total_Time_CPU = (double)(tot_endClock.QuadPart - tot_beginClock.QuadPart) / tot_clockFreq.QuadPart;
	printf("Total processing Time_DBS : %f ms\n", total_Time_CPU * 1000);
	//system("pause");

	//sprintf(str, "DBS_Dither.bmp");
	//sprintf(str, "new_DBS_Dither2.bmp");	
	sprintf(str, "test_DBS_Dither.bmp");
	//sprintf(str, "bird_DBS_Dither_K.bmp");

	FwriteCPU(str);

	free(rgb);
	free(pix);
	//free(pix_ms);
	free(pix_hvs);
	free(err);
	free(CEP);
	free(pixE);
	fclose(fp);
	
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


	//Dither();
	Halftone();				// 초기 하프톤이미지 생성

	GaussianFilter();		// 가우시안 필터 생성
	/*
	for (int i = 0; i < fs; i++)
	{
		for (int j = 0; j < fs; j++)
		{
			//G[i][j] /= 273;
			printf("%lf ", G[i][j]);
		}
		printf("\n");
	}
	*/
	CONV();		// 2차원 컨볼루션 연산 행렬 생성 (CPP)
	
	for (int y = 0; y < bph; y++)
	{
		for (int x = 0; x < bpl; x++)
		{
			err[y * bpl + x] = (double)pix_hvs[y * bpl + x] / 255 - (double)pix[y * bpl + x] / 255;
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
							if (pix_hvs[(i + y) * bpl + (j + x)] != pix_hvs[i * bpl + j])
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
						eps = (a0 * a0 + a1 * a1) * CPP[halfcppsize][halfcppsize]
							+ 2 * a0 * a1 * CPP[halfcppsize + y][halfcppsize + x]
							+ 2 * a0 * CEP[(i + halfcppsize) * (bpl + halfcppsize * 2) + (j + halfcppsize)]
							+ 2 * a1 * CEP[(i + y + halfcppsize) * (bpl + halfcppsize * 2) + (j + x + halfcppsize)];
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
							CEP[(i + y + halfcppsize) * (bpl + halfcppsize * 2) + (j + x + halfcppsize)] += (double)a0c * CPP[y + halfcppsize][x + halfcppsize];
							CEP[(i + y + cpy + halfcppsize) * (bpl + halfcppsize * 2) + (j + x + cpx + halfcppsize)] += (double)a1c * CPP[y + halfcppsize][x + halfcppsize];
						}
					}
					//pix_hvs[i * bpl + j] = (unsigned char)((pix_hvs[i * bpl + j] / 255 + a0c) % 2) * 255;
					//pix_hvs[(cpy + i) * bpl + (j + cpx)] = (unsigned char)((pix_hvs[(cpy + i) * bpl + (j + cpx)] / 255 + a1c) % 2) * 255;
					pix_hvs[i * bpl + j] += (unsigned char)a0c * 255;
					pix_hvs[(cpy + i) * bpl + (j + cpx)] += (unsigned char)a1c * 255;
					count++;
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
	double sum = 0;
	double d = /*(fs - 1) / 6*/ 1.2;		// sigma
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
			sum += G[k + gaulen][l + gaulen];
			*/	
		}
	}
	printf("%lf\n", sum);
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
	for (int y = (-1) * halfcppsize * 2; y < bph; y++)
	{
		for (int x = (-1) * halfcppsize * 2; x < bpl; x++)
		{
			sum = 0;
			for (int i = y; i <= y + halfcppsize * 2; i++)
			{
				for (int j = x; j <= x + halfcppsize * 2; j++)
				{
					if ((i >= 0 && j >= 0) && (i < bph && j < bpl))
					{
						sum += (double)CPP[i - y][j - x] * (double)err[i * bpl + j];
					}
				}
			}
			CEP[(y + halfcppsize * 2) * (bpl + halfcppsize * 2) + (x + halfcppsize * 2)] = (double)sum;
		}
	}
}
void BayerMatrix(int n)
{
	int count = 2;
	// 임계값 행렬 초기값 설정
	T[0][0] = 0;
	T[0][1] = 2;
	T[1][0] = 3;
	T[1][1] = 1;

	while (count < n)
	{
		for (int i = 0; i < count; i++)
		{
			for (int j = 0; j < count; j++)
			{
				T[i][j] *= 4;
				T[i][j + count] = T[i][j] + 2;
				T[i + count][j] = T[i][j] + 3;
				T[i + count][j + count] = T[i][j] + 1;
			}
		}
		count *= 2;
	}

	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			printf("%d ", T[i][j]);
		}
		printf("\n");
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
// 
double GaussianRandom(double stddev, double average)
{
	/*
	double v1, v2, s;

	do {
		v1 = 2 * ((double)rand() / RAND_MAX) - 1;      // -1.0 ~ 1.0 까지의 값
		v2 = 2 * ((double)rand() / RAND_MAX) - 1;      // -1.0 ~ 1.0 까지의 값
		s = v1 * v1 + v2 * v2;
	} while (s >= 1 || s == 0);

	s = sqrt((-2 * log(s)) / s);

	return v1 * s;
	*/
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
	// Thresh Hold
	/*
	for (int y = 1; y < bph - 1; y++)
	{
		for (int x = 1; x < bpl - 1; x++)
		{
			pix_hvs[y * bpl + x] = pix[y * bpl + x] / 128 * 255;
		}
	}
	*/

	// 정규분포 난수로 하프톤이미지 생성
	/*
	srand(time(NULL));
	double tmp;

	for (int y = 1; y < bph - 1; y++)
	{
		for (int x = 1; x < bpl - 1; x++)
		{
			tmp = (double)GaussianRandom(2.5, 0);
			//printf("%lf\n", tmp);
			if (tmp > 0)
			{
				pix_hvs[y * bpl + x] = 255;
			}
		}
	}
	*/

	// Ordered Dither
	BayerMatrix(ts);
}
void Dither()
{
	// 양방향
	
	int quant_error = 0;	// 초기 디더링 작업을 할때 쓰일 에러 가중치 변수

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
	
	// 단방향
	/*
	int quant_error = 0;	// 초기 디더링 작업을 할때 쓰일 에러 가중치 변수

	for (int y = 1; y < bph - 1; y++)
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
	*/
}
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