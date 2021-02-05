// 디더링 알고리즘들을 구현한 코드입니다.
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
//double total_Time_GPU, total_Time_CPU, tmp_time;
//LARGE_INTEGER beginClock, endClock, clockFreq;
//LARGE_INTEGER tot_beginClock, tot_endClock, tot_clockFreq;

// 비트맵 이미지 정보를 받아오기 위한 구조체 변수
BITMAPFILEHEADER bfh;
BITMAPINFOHEADER bih;
RGBQUAD * rgb;

// 이미지 정보를 다루기 위해 사용하는 변수
int bpl, pad;
int width, height;
long pix_size;
unsigned char trash[3] = { 0 }; // 쓰레기 값
unsigned char* pix; // 원본 이미지
unsigned char* pix_h; // 하프톤 이미지

void Fread();	// bmp 파일 읽기
void Fwrite();	// bmp 파일 쓰기

void Floyd_Steinberg();
void Ordered();
void Direct_Binary_Search();


int main(void)
{
	//Floyd_Steinberg();

	//Ordered();

	//Direct_Binary_Search();

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
void Fwrite()
{
	FILE * fp = fopen("output.bmp", "wb");
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

void Floyd_Steinberg()
{
	Fread();
	int* pix_e = 0;
	int quant_error = 0;

	pix_h = (unsigned char *)calloc(pix_size, sizeof(unsigned char));
	memcpy(pix_h, pix, sizeof(unsigned char) * pix_size);
	pix_e = (int *)calloc(pix_size, sizeof(int));

	// Floyd Steinberg 양방향
	for (int y = 1; y < height - 1; y++)
	{
		if (y % 2 == 1)
		{
			for (int x = 1; x < width - 1; x++)
			{
				pix_e[y * width + x] += pix_h[y * width + x];
				pix_h[y * width + x] = pix_e[y * width + x] / 128 * 255;
				quant_error = pix_e[y * width + x] - pix_h[y * width + x];

				pix_e[y * width + x + 1] += quant_error * 7 / 16;
				pix_e[(y + 1) * width + x - 1] += quant_error * 3 / 16;
				pix_e[(y + 1) * width + x] += quant_error * 5 / 16;
				pix_e[(y + 1) * width + x + 1] += quant_error * 1 / 16;
			}
		}
		else
		{
			for (int x = width - 2; x >= 1; x--)
			{
				pix_e[y * width + x] += pix_h[y * width + x];
				pix_h[y * width + x] = pix_e[y * width + x] / 128 * 255;
				quant_error = pix_e[y * width + x] - pix_h[y * width + x];

				pix_e[y * width + x - 1] += quant_error * 7 / 16;
				pix_e[(y + 1) * width + x + 1] += quant_error * 3 / 16;
				pix_e[(y + 1) * width + x] += quant_error * 5 / 16;
				pix_e[(y + 1) * width + x - 1] += quant_error * 1 / 16;
			}
		}
	}
	// Floyd Steinberg 단방향
	/*
	for (int y = 1; y < height - 1; y++)
	{
		for (int x = 1; x < width - 1; x++)
		{

			pix_e[y * width + x] += pix_h[y * width + x];
			pix_h[y * width + x] = pix_e[y * width + x] / 128 * 255;
			quant_error = pixE[y * width + x] - pix_h[y * width + x];

			pix_e[y * width + x + 1] += quant_error * 7 / 16;
			pix_e[(y + 1) * width + x - 1] += quant_error * 3 / 16;
			pix_e[(y + 1) * width + x] += quant_error * 5 / 16;
			pix_e[(y + 1) * width + x + 1] += quant_error * 1 / 16;

		}
	}
	*/
	Fwrite();

	free(rgb);
	free(pix);
	free(pix_h);
	free(pix_e);
}
void Ordered()
{
	Fread();
	int ts = 64;
	double T[64][64];
	pix_h = (unsigned char *)calloc(pix_size, sizeof(unsigned char));

	int count = 2;
	// 임계값 행렬 초기값 설정
	T[0][0] = 0.0;
	T[0][1] = 2.0;
	T[1][0] = 3.0;
	T[1][1] = 1.0;

	while (count < ts)
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

	for (int i = 0; i < ts; i++)
	{
		for (int j = 0; j < ts; j++)
		{
			T[i][j] /= ts * ts;
			//printf("%lf ", (double)T[i][j]);
		}
		//printf("\n");
	}
	// Ordered Dither
	for (int y = 1; y < height - 1; y++)
	{
		for (int x = 1; x < width - 1; x++)
		{
			if (pix[y * width + x] > T[y % ts][x % ts] * 255)
			{
				pix_h[y * width + x] = 255;
				//pix_check[y * bpl + x] = 1;
			}
		}
	}

	Fwrite();
	free(rgb);
	free(pix);
	free(pix_h);
}
void Direct_Binary_Search()
{
	Fread();
	double G[7][7];
	int fs = 7;	// 가우시안 필터 사이즈
	double CPP[13][13];	// 컨볼루션 매트릭스
	int halfcppsize = 6;

	/*
	double G[11][11];
	int fs = 11;	// 가우시안 필터 사이즈
	double CPP[21][21];
	int halfcppsize = 10;
	*/

	double* CEP;
	double pi = 3.14159265358979323846264338327950288419716939937510;

	int count = 0;			// 최소제곱오차 값이 0이 아닐때까지 반복문을 돌리기위한 카운트 변수
	double eps = 0.0;
	double eps_min = 0.0;
	int a0 = 0;
	int a1 = 0;
	int a0c = 0;
	int a1c = 0;
	int cpx = 0;
	int cpy = 0;

	pix_h = (unsigned char *)calloc(pix_size, sizeof(unsigned char));
	CEP = (double *)calloc((width + halfcppsize * 2) * (height + halfcppsize * 2), sizeof(double));

	{	// 정규분포 난수로 하프톤 이미지 생성
		double tmp = 0;
		double v1, v2, s;
		double stddev = 1.0;
		double average = 0;

		srand(time(NULL));

		for (int y = 1; y < height; y++)
		{
			for (int x = 1; x < width; x++)
			{
				{	// 가우시안 랜덤 숫자 생성

					do {
						v1 = 2 * ((double)rand() / RAND_MAX) - 1;      // -1.0 ~ 1.0 까지의 값
						v2 = 2 * ((double)rand() / RAND_MAX) - 1;      // -1.0 ~ 1.0 까지의 값
						s = v1 * v1 + v2 * v2;
					} while (s >= 1 || s == 0);

					s = sqrt((-2 * log(s)) / s);

					tmp = v1 * s;
					tmp = (stddev * tmp) + average;

				}
				//printf("%lf\n", tmp);
				if (tmp > 0)
				{
					//if (pix_check[y * width + x] == 0)
					pix_h[y * width + x] = 255;
				}
			}
		}
	}

	{	// 가우시안 필터
		//double sum = 0;
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
				//sum += G[k + gaulen][l + gaulen];
				*/
			}
		}
		//printf("%lf\n", sum);
	}

	{	// 2차원 컨볼루션 연산
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

	{	// 상호관계연산 행렬 생성 (CEP)
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
							sum += (double)CPP[i - y][j - x] * ((double)pix_h[i * width + j] / 255 - (double)pix[i * width + j] / 255);
						}
					}
				}
				CEP[(y + halfcppsize * 2) * (width + halfcppsize * 2) + (x + halfcppsize * 2)] = (double)sum;
			}
		}
	}

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
				a0c = 0;
				a1c = 0;
				cpx = 0;
				cpy = 0;
				eps_min = 0;
				for (int y = -1; y <= 1; y++)
				{
					//if (i + y < 0 || i + y >= height)
						//continue;			
					for (int x = -1; x <= 1; x++)
					{
						//if (j + x < 0 || j + x >= height)
							//continue;
						if (y == 0 && x == 0)
						{
							if (pix_h[i * width + j] == 255)
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
							if (pix_h[(i + y) * width + (j + x)] != pix_h[i * width + j])
							{
								if (pix_h[i * width + j] == 255)
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
					pix_h[i * width + j] += a0c * 255;
					pix_h[(cpy + i) * width + (j + cpx)] += a1c * 255;
					count++;
				}
			}
		}
		//printf("%d\n", count);
		if (count == 0)
			break;
	}

	Fwrite();
	free(rgb);
	free(pix);
	free(pix_h);
	free(CEP);
}