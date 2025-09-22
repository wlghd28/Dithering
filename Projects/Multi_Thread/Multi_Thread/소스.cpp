#include <stdio.h>
#include <process.h>
#include <Windows.h>
#include <string.h>
#include <tchar.h>
#include <stdbool.h>
#include <stdlib.h>

#pragma warning(disable : 4996)

//int cpu1_end = 0;
//int cpu2_end = 0;


double total_Time_GPU, total_Time_CPU, tmp_time;
LARGE_INTEGER beginClock, endClock, clockFreq;
LARGE_INTEGER tot_beginClock, tot_endClock, tot_clockFreq;

BITMAPFILEHEADER bfh;
BITMAPINFOHEADER bih;
RGBQUAD * rgb;
int bpl, bph;

unsigned char* pix;
unsigned char* pix_tmp;
int * pixE;

int x_1, y_1;
int x_2, y_2;
//int flag1 = 0;
//int flag2 = 0;
//int quant_error1 = 0;
//int quant_error2 = 0;
int vsum, v1, v2;
char str[100];

void FwriteCPU(char *);

//unsigned __stdcall CpuCal1();
//unsigned __stdcall CpuCal2();
void OneCpuCal(int v1, int vsum);

int main(void)
{
	FILE * fp;
	fp = fopen("EDIMAGE2.bmp", "rb");
	if (fp == NULL)
		return 0;
	fread(&bfh, sizeof(bfh), 1, fp);
	fread(&bih, sizeof(bih), 1, fp);

	rgb = (RGBQUAD*)malloc(sizeof(RGBQUAD) * 256);
	fread(rgb, sizeof(RGBQUAD), 256, fp);

	// BPL�� �����ֱ� ���ؼ� �ȼ��������� ����� 4�� ����� ����
	bpl = (bih.biWidth + 3) / 4 * 4;
	bph = (bih.biHeight + 3) / 4 * 4;
	printf("%d %d", bpl, bph);
	pix = (unsigned char *)malloc(sizeof(unsigned char) * bpl * bph);
	memset(pix, 0, sizeof(unsigned char) * bpl * bph);
	fread(pix, sizeof(unsigned char), bpl * bph, fp);

	pix_tmp = (unsigned char *)malloc(sizeof(unsigned char) * bpl * bph);
	memset(pix_tmp, 0, sizeof(unsigned char) * bpl * bph);
	memcpy(pix_tmp, pix, sizeof(unsigned char) * bpl * bph);

	pixE = (int *)malloc(sizeof(int) * bpl * bph);
	memset(pixE, 0, sizeof(int) * bpl * bph);

	// �����忡�� �̿��� ��ǥ���� �̸� �ʱ�ȭ �����ش�.
	x_1 = 1;
	x_2 = 1;
	y_1 = 1;
	y_2 = 2;

	// ����� ����ġ ����
	v1 = 87;
	v2 = 42;
	vsum = v1 + v2;


	QueryPerformanceFrequency(&tot_clockFreq);	// �ð��� �����ϱ����� �غ�
	total_Time_CPU = 0;

	/*
	HANDLE hThread[2]; // ������ �ڵ鷯
	unsigned threadID[2];

	QueryPerformanceCounter(&tot_beginClock); // �ð����� ����
	// _beginthreadex
	hThread[0] = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)CpuCal1, NULL, 0, &threadID[0]);
	hThread[1] = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)CpuCal2, NULL, 0, &threadID[1]);
	WaitForMultipleObjects(2, (const HANDLE *)hThread, TRUE, INFINITE);
	QueryPerformanceCounter(&tot_endClock);
	total_Time_CPU = (double)(tot_endClock.QuadPart - tot_beginClock.QuadPart) / tot_clockFreq.QuadPart;
	printf("Total processing Time_CPU_Multi : %f ms\n", total_Time_CPU * 1000);
	*/


	QueryPerformanceCounter(&tot_beginClock); // �ð����� ����
	
	//for (int i = 0; i < 10000; i++)
	//{
		OneCpuCal(4008, 10000);
		sprintf(str, "Dither/Dither2(%d,%d).bmp", 4008, 10000);
		FwriteCPU(str);
		//memset(pixE, 0, sizeof(int) * bpl * bph);
		//memset(pix, 0, sizeof(unsigned char) * bpl * bph);
		//memcpy(pix, pix_tmp, sizeof(unsigned char) * bpl * bph);
		//printf(str, "v1= %d,  v2= %d", v1, 100);

		//sprintf(str, "v1= %d,  v2= %d", i, 100);
	//}
	QueryPerformanceCounter(&tot_endClock);

	total_Time_CPU = (double)(tot_endClock.QuadPart - tot_beginClock.QuadPart) / tot_clockFreq.QuadPart;
	printf("Total processing Time_CPU_Single : %f ms\n", total_Time_CPU * 1000);
	system("pause");

	/*
	sprintf(str, "Dither(%d,%d).bmp", v1, v2);
	FwriteCPU(str);

	printf(str, "v1= %d,  v2= %d", v1, v2);

	sprintf(str, "v1= %d,  v2= %d", v1, v2);
	*/
	//MessageBox(NULL, str, "Check", MB_OK);


	free(rgb);

	free(pix);
	free(pix_tmp);
	free(pixE);

	fclose(fp);


	//CloseHandle(hThread);

	return 0;
}
/*
unsigned __stdcall CpuCal1()
{
	// �� �˰��� 
	for (y_1 = 1; y_1 < bph - 1; y_1 += 2)
	{
		x_1 = 1;
		while(x_1 < bpl - 1)
		{
			if (y_1 == 1)
			{
				while (x_1 < bpl - 1)
				{
					pixE[y_1 * bpl + x_1] += pix[y_1 * bpl + x_1];
					pix[y_1 * bpl + x_1] = pixE[y_1 * bpl + x_1] / 128 * 255;
					quant_error1 = pixE[y_1 * bpl + x_1] - pix[y_1 * bpl + x_1];
					pixE[y_1 * bpl + x_1 + 1] += quant_error1 * v1 / vsum;
					pixE[(y_1 + 1) * bpl + x_1 - 1] += quant_error1 * v2 / vsum;
					x_1++;
				}
				cpu1_end = 1;
				break;
			}
			else
			{
				if (cpu2_end == 1)
				{
					while (x_1 < bpl - 1)
					{
						pixE[y_1 * bpl + x_1] += pix[y_1 * bpl + x_1];
						pix[y_1 * bpl + x_1] = pixE[y_1 * bpl + x_1] / 128 * 255;
						quant_error1 = pixE[y_1 * bpl + x_1] - pix[y_1 * bpl + x_1];
						pixE[y_1 * bpl + x_1 + 1] += quant_error1 * v1 / vsum;
						pixE[(y_1 + 1) * bpl + x_1 - 1] += quant_error1 * v2 / vsum;
						x_1++;
					}
					cpu2_end = 0;
				}
				else
				{
					while (x_1 <= x_2 - 3)
					{
						pixE[y_1 * bpl + x_1] += pix[y_1 * bpl + x_1];
						pix[y_1 * bpl + x_1] = pixE[y_1 * bpl + x_1] / 128 * 255;
						quant_error1 = pixE[y_1 * bpl + x_1] - pix[y_1 * bpl + x_1];
						pixE[y_1 * bpl + x_1 + 1] += quant_error1 * v1 / vsum;
						pixE[(y_1 + 1) * bpl + x_1 - 1] += quant_error1 * v2 / vsum;
						x_1++;
					}
				}
			}
		}
		cpu1_end = 1;
	}
	//HANDLE thread = GetCurrentThread();
	// ����� �˰���
	for (y_1 = 1; y_1 < bph - 1; y_1 += 2)
	{
		for (x_1 = 1; x_1 < bpl - 1; x_1++)
		{
			// �̺κп��� ���ϰ� �ɸ�.
			while (y_1 == y_2 + 1 && x_1 > x_2 - 3)
			{
				SetThreadPriority(thread, THREAD_PRIORITY_LOWEST);
			}
			pixE[y_1 * bpl + x_1] += pix[y_1 * bpl + x_1];
			pix[y_1 * bpl + x_1] = pixE[y_1 * bpl + x_1] / 128 * 255;
			quant_error1 = pixE[y_1 * bpl + x_1] - pix[y_1 * bpl + x_1];
			pixE[y_1 * bpl + x_1 + 1] += quant_error1 * v1 / vsum;
			pixE[(y_1 + 1) * bpl + x_1 - 1] += quant_error1 * v2 / vsum;

			//printf("x_1 :%d, y_1 : %d\n", x_1, y_1);
		}
	}
	_endthreadex(0);
	return 0;
}
*/
/*
unsigned __stdcall CpuCal2()
{
	// �� �˰���
	for (y_2 = 2; y_2 < bph - 1; y_2 += 2)
	{
		x_2 = 1;
		while(x_2 < bpl - 1)
		{
			if (cpu1_end == 1)
			{
				while (x_2 < bpl - 1)
				{
					pixE[y_2 * bpl + x_2] += pix[y_2 * bpl + x_2];
					pix[y_2 * bpl + x_2] = pixE[y_2 * bpl + x_2] / 128 * 255;
					quant_error2 = pixE[y_2 * bpl + x_2] - pix[y_2 * bpl + x_2];
					pixE[y_2 * bpl + x_2 + 1] += quant_error2 * v1 / vsum;
					pixE[(y_2 + 1) * bpl + x_2 - 1] += quant_error2 * v2 / vsum;
					x_2++;
				}
				cpu1_end = 0;
				break;
			}
			else
			{
				while (x_2 <= x_1 - 3)
				{
					pixE[y_2 * bpl + x_2] += pix[y_2 * bpl + x_2];
					pix[y_2 * bpl + x_2] = pixE[y_2 * bpl + x_2] / 128 * 255;
					quant_error2 = pixE[y_2 * bpl + x_2] - pix[y_2 * bpl + x_2];
					pixE[y_2 * bpl + x_2 + 1] += quant_error2 * v1 / vsum;
					pixE[(y_2 + 1) * bpl + x_2 - 1] += quant_error2 * v2 / vsum;
					x_2++;
				}
			}
		}
		cpu2_end = 1;
	}
	//HANDLE thread = GetCurrentThread();
	// ����� �˰���
	for (y_2 = 2; y_2 < bph - 1; y_2 += 2)
	{
		for (x_2 = 1; x_2 < bpl - 1; x_2++)
		{
			// �̺κп��� ���ϰ� �ɸ�.
			while (y_2 == y_1 + 1 && x_2 > x_1 - 3)
			{
				SetThreadPriority(thread, THREAD_PRIORITY_LOWEST);
			}
			pixE[y_2 * bpl + x_2] += pix[y_2 * bpl + x_2];
			pix[y_2 * bpl + x_2] = pixE[y_2 * bpl + x_2] / 128 * 255;
			quant_error2 = pixE[y_2 * bpl + x_2] - pix[y_2 * bpl + x_2];
			pixE[y_2 * bpl + x_2 + 1] += quant_error2 * v1 / vsum;
			pixE[(y_2 + 1) * bpl + x_2 - 1] += quant_error2 * v2 / vsum;
			//printf("x_2 :%d, y_2 : %d\n", x_2, y_2);
		}
	}
	_endthreadex(0);
	return 0;
}
*/
void OneCpuCal(int v1, int vsum)
{
	// �������� �����ϴ� ���
	/*
	int quant_error = 0;
	for (int y = 1; y < bph - 1; y++)
	{
		for (int x = 1; x < bpl - 1; x++)
		{
			pixE[y * bpl + x] += pix[y * bpl + x];
			pix[y * bpl + x] = pixE[y * bpl + x] / 128 * 255;
			quant_error = pixE[y * bpl + x] - pix[y * bpl + x];

			pixE[y * bpl + x + 1] += quant_error * 7 / 16;
			pixE[(y + 1) * bpl + x - 1] += quant_error * 3 / 16;
			pixE[(y + 1) * bpl + x] += quant_error * 5 / 16;
			pixE[(y + 1) * bpl + x + 1] += quant_error * 1 / 16;
			//printf("x_2 :%d, y_2 : %d\n", x_2, y_2);
		}
	}
	*/

	/*
	// �������� ������ ���
	int err7, err3, err5, err1;
	int quant_error = 0;
	for (int y = 1; y < bph - 1; y++)
	{
		for (int x = 1; x < bpl - 1; x++)
		{
			err7 = pixE[y * bpl + x - 1] - pix[y * bpl + x - 1];
			err3 = pixE[(y - 1) * bpl + x + 1] - pix[(y - 1) * bpl + x + 1];
			err5 = pixE[(y - 1) * bpl + x] - pix[(y - 1) * bpl + x];
			err1 = pixE[(y - 1) * bpl + x - 1] - pix[(y - 1) * bpl + x - 1];
			pixE[y * bpl + x] += err7 * 7 / 16;
			pixE[y * bpl + x] += err3 * 3 / 16;
			pixE[y * bpl + x] += err5 * 5 / 16;
			pixE[y * bpl + x] += err1 * 1 / 16;
			pixE[y * bpl + x] += pix[y * bpl + x];
			pix[y * bpl + x] = pixE[y * bpl + x] / 128 * 255;
		}
	}
	*/

	/*
	// ���ο� ����� ��� (�ܹ���)
	int err7, err3, err5, err1;
	for (int y = 1; y < bph - 1; y++)
	{
		for (int x = 1; x < bpl - 1; x++)
		{
			// �� �ȼ��� 7, 3, 5, 1 ����ġ�� ���� ���� �� ���
			err7 = pixE[y * bpl + x - 1] - pix[y * bpl + x - 1];
			err3 = pixE[(y - 1) * bpl + x + 1] - pix[(y - 1) * bpl + x + 1];
			err5 = pixE[(y - 1) * bpl + x] - pix[(y - 1) * bpl + x];
			err1 = pixE[(y - 1) * bpl + x - 1] - pix[(y - 1) * bpl + x - 1];

			// pixE : �ȼ��� ���� ��
			pixE[y * bpl + x] += err7 * 7 / 16;
			pixE[y * bpl + x] += err3 * 3 / 16;
			pixE[y * bpl + x] += err5 * 5 / 16;
			pixE[y * bpl + x] += err1 * 1 / 16;
			pixE[y * bpl + x] += pix[y * bpl + x];

			// ���� ������ ����
			pix[y * bpl + x] = (pixE[y * bpl + x] - (pix[y * bpl + x] * v1 / vsum)) / 128 * 255;
		}
	}
	*/

	
	// ���ο� ����� ��� (�����)
	// �� ����ġ 7, 3, 5, 1�� ���� ���� ��
	int err7, err3, err5, err1;

	for (int y = 1; y < bph - 1; y++)
	{
		if (y % 2 == 1)
		{
			for (int x = 1; x < bpl - 1; x++)
			{
				// �� �ȼ��� 7, 3, 5, 1 ����ġ�� ���� ���� �� ���
				err7 = pixE[y * bpl + x - 1] - pix[y * bpl + x - 1];
				err1 = pixE[(y - 1) * bpl + x + 1] - pix[(y - 1) * bpl + x + 1];
				err5 = pixE[(y - 1) * bpl + x] - pix[(y - 1) * bpl + x];
				err3 = pixE[(y - 1) * bpl + x - 1] - pix[(y - 1) * bpl + x - 1];

				// pixE : �ȼ��� ���� ��, ���� ���� ����ش�.
				pixE[y * bpl + x] += err7 * 7 / 16;
				pixE[y * bpl + x] += err1 * 1 / 16;
				pixE[y * bpl + x] += err5 * 5 / 16;
				pixE[y * bpl + x] += err3 * 3 / 16;
				pixE[y * bpl + x] += pix[y * bpl + x];

				// ���� ������ ����
				// v1 / vsum �� ����ġ ������ 2 / 5 ���� ����� ���� ��������.
				pix[y * bpl + x] = (pixE[y * bpl + x] - (pix[y * bpl + x] * v1 / vsum)) / 128 * 255;
			}
		}
		else
		{
			for (int x = bpl - 2; x >= 1; x--)
			{
				// �� �ȼ��� 7, 3, 5, 1 ����ġ�� ���� ���� �� ���
				err7 = pixE[y * bpl + x + 1] - pix[y * bpl + x + 1];
				err1 = pixE[(y - 1) * bpl + x - 1] - pix[(y - 1) * bpl + x - 1];
				err5 = pixE[(y - 1) * bpl + x] - pix[(y - 1) * bpl + x];
				err3 = pixE[(y - 1) * bpl + x + 1] - pix[(y - 1) * bpl + x + 1];

				// pixE : �ȼ��� ���� ��, ���� ���� ����ش�.
				pixE[y * bpl + x] += err7 * 7 / 16;
				pixE[y * bpl + x] += err1 * 1 / 16;
				pixE[y * bpl + x] += err5 * 5 / 16;
				pixE[y * bpl + x] += err3 * 3 / 16;
				pixE[y * bpl + x] += pix[y * bpl + x];

	
				// ���� ������ ����
				// v1 / vsum �� ����ġ ������ 2 / 5 ���� ����� ���� ��������.
				pix[y * bpl + x] = (pixE[y * bpl + x] - (pix[y * bpl + x] * v1 / vsum)) / 128 * 255;
			}
		}		
	}
}

void FwriteCPU(char * fn)
{
	// ������ �ȼ����� bmp���Ϸ� ����.
	FILE * fp2 = fopen(fn, "wb");
	fwrite(&bfh, sizeof(bfh), 1, fp2);
	fwrite(&bih, sizeof(bih), 1, fp2);
	fwrite(rgb, sizeof(RGBQUAD), 256, fp2);

	fwrite(pix, sizeof(unsigned char), bpl * bph, fp2);
	//fwrite(pixE, sizeof(int), bpl * bph, fp2);
	fclose(fp2);
}