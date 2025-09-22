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
// �ð��� ����ϱ� ���� ����
double total_Time_GPU, total_Time_CPU, tmp_time;
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
unsigned char* pix; // ���� �̹���
unsigned char* pix_hvs; // ������ �̹���
unsigned char trash[3] = {0}; // ������ ��

double pi = 3.14159265358979323846264338327950288419716939937510;

double G[7][7];
int fs = 7;	// ����þ� ���� ������
double CPP[13][13]; // �ڱ��� ���(G, G)
int halfcppsize = 6;

/*
double G[11][11];
int fs = 11;	// ����þ� ���� ������
double CPP[21][21];
int halfcppsize = 10;
*/

double* CEP;    // ��ȣ��� ���(err, CPP)
char str[100];				// ���ϸ��� ���� ���ڿ�

void DBS();					// Direct Binary Search ����
void GaussianFilter();		// ����þ� ���� ����
double GaussianRandom(double stddev, double average);		// ���Ժ��� ���� ����
void CONV();	// 2���� ������� ����
void XCORR();	// ��ȣ������� ����
void Halftone();				// �ʱ� ������ �̹��� ����
void FwriteCPU(char *);		// ����� �ȼ����� bmp���Ϸ� �����ϴ� �Լ�

int main(void)
{
	FILE * fp;
	fp = fopen("input.bmp", "rb");
	//fp = fopen("EDIMAGE.bmp", "rb");
	//fp = fopen("newEDIMAGE2.bmp", "rb");
	//fp = fopen("test.bmp", "rb");
	//fp = fopen("bird2_C.bmp", "rb");
	if (fp == NULL)
	{
		printf("File Not Found!!\n");
		return 0;
	}

	fread(&bfh, sizeof(bfh), 1, fp);
	fread(&bih, sizeof(bih), 1, fp);
	width = bih.biWidth;
	height = bih.biHeight;
	pix_size = width * height;
	printf("width : %d , height : %d\n", width, height);
	printf("image size : %d X %d\n", width, height);

	rgb = (RGBQUAD*)malloc(sizeof(RGBQUAD) * 256);
	fread(rgb, sizeof(RGBQUAD), 256, fp);

	// BPL�� �����ֱ� ���ؼ� �ȼ��������� ����� 4�� ����� ����
	bpl = (bih.biWidth + 3) / 4 * 4;
	// ������ �е� �� ���
	pad = bpl - width;

	// �̹��� ������ �д´�
	pix = (unsigned char *)calloc(pix_size, sizeof(unsigned char));
	for (int i = 0; i < height; i++)
	{
		fread(pix + (i * width), sizeof(unsigned char), width, fp);
		fread(&trash, sizeof(unsigned char), pad, fp);
	}

	pix_hvs = (unsigned char *)calloc(pix_size, sizeof(unsigned char));

	CEP = (double *)calloc((width + halfcppsize * 2) * (height + halfcppsize * 2), sizeof(double));
	//memset(CEP, 0, sizeof(double) * (width + halfcppsize * 2) * (height + halfcppsize * 2));


	QueryPerformanceFrequency(&tot_clockFreq);	// �ð��� �����ϱ����� �غ�

	total_Time_CPU = 0;
	QueryPerformanceCounter(&tot_beginClock); // �ð����� ����
	// Direct Binary Search
	DBS();
	QueryPerformanceCounter(&tot_endClock);

	total_Time_CPU = (double)(tot_endClock.QuadPart - tot_beginClock.QuadPart) / tot_clockFreq.QuadPart;
	printf("Total processing Time_DBS : %f ms\n", total_Time_CPU * 1000);
	//system("pause");

	//sprintf(str, "DBS_Dither.bmp");
	//sprintf(str, "new_DBS_Dither2.bmp");	
	//sprintf(str, "test_DBS_Dither.bmp");
	//sprintf(str, "bird_DBS_Dither_C.bmp");
	sprintf(str, "output.bmp");

	FwriteCPU(str);

	free(rgb);
	free(pix);
	free(pix_hvs);
	free(CEP);
	fclose(fp);

	return 0;
}

void DBS()
{
	int count = 0;			// �ּ��������� ���� 0�� �ƴҶ����� �ݺ����� ���������� ī��Ʈ ����
	double eps = 0.0;
	double eps_min = 0.0;
	int a0 = 0;
	int a1 = 0;
	int a0c = 0;
	int a1c = 0;
	int cpx = 0;
	int cpy = 0;

	Halftone();				// �ʱ� �������̹��� ����

	GaussianFilter();		// ����þ� ���� ����

	CONV();		// 2���� ������� ���� ��� ���� (CPP)

	XCORR();	// ��ȣ���迬�� ��� ���� (CEP)

	// DBS ���� ����..
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
					pix_hvs[i * width + j] += a0c * 255;
					pix_hvs[(cpy + i) * width + (j + cpx)] += a1c * 255;
					count++;
				}
			}
		}
		printf("%d\n", count);
		if (count == 0)
			break;
	}
}
// ����þ� ���� ����
void GaussianFilter()
{
	double d = /*(fs - 1) / 6;*/ 1.2;			// sigma = 1.2
	double c;
	int gaulen = (fs - 1) / 2;

	for (int k = (-1) * gaulen; k <= gaulen; k++)
	{
		for (int l = (-1) * gaulen; l <= gaulen; l++)
		{
			c = (k * k + l * l) / (2 * d * d);
			G[k + gaulen][l + gaulen] = exp((-1) * c) / (2 * pi * d * d);
			//sum += G[k + gaulen][l + gaulen];


			// fs = 11 �϶� ����� ���� ������..	
			/*
			c = (k * k + l * l) / (2 * d * d) + 1;
			G[k + gaulen][l + gaulen] = 1 / c;
			*/
		}
	}
}
// 2���� ������� ����
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
						sum += G[i - y][j - x] * G[i][j];
					}
				}
			}
			CPP[(y + fs - 1)][(x + fs - 1)] = (double)sum;
		}
	}
}
// ��ȣ������� ����
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
// ���Ժ��� ���� �߻�
double GaussianRandom(double stddev, double average)
{
	double v1, v2, s, temp;

	do {
		v1 = 2 * ((double)rand() / RAND_MAX) - 1;      // -1.0 ~ 1.0 ������ ��
		v2 = 2 * ((double)rand() / RAND_MAX) - 1;      // -1.0 ~ 1.0 ������ ��
		s = v1 * v1 + v2 * v2;
	} while (s >= 1 || s == 0);

	s = sqrt((-2 * log(s)) / s);

	temp = v1 * s;
	temp = (stddev * temp) + average;

	return temp;
}
void Halftone()
{
	// ���Ժ��� ������ �������̹��� ����
	srand(time(NULL));
	double tmp;
	for (int y = 1; y < height; y++)
	{
		for (int x = 1; x < width; x++)
		{
			tmp = (double)GaussianRandom(1.0, 0);
			if (tmp > 0)
			{
				pix_hvs[y * width + x] = 255;
			}
		}
	}
}
// ������ �ȼ����� bmp���Ϸ� ����.
void FwriteCPU(char * fn)
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