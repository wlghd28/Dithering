#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include "tiff.h"
#include "tiffconf.h"
#include "tiffio.h"
#include "tiffvers.h"

#define _CRT_SEECURE_NO_WARNINGS

typedef struct MASK
{
	BYTE threshold1;
	BYTE threshold2;
	BYTE threshold3;
	int count;
	BYTE before_data;
}MASK_2Bit;

typedef struct Data
{
	unsigned char bit3 : 2;
	unsigned char bit2 : 2;
	unsigned char bit1 : 2;
	unsigned char bit0 : 2;
}Data_2Bit;


int datasize = 744;
int readsize = 744;

// data 값을 추출한 마스크 값 
MASK_2Bit mask[744][744];

int check[16] = { 0, };

int main(void)
{
	// 마스크 값 초기화
	for (int i = 0; i < 744; i++)
	{
		for (int j = 0; j < 744; j++)
		{
			mask[i][j].threshold1 = 0;
			mask[i][j].threshold2 = 0;
			mask[i][j].threshold3 = 0;
			mask[i][j].count = 0;
			mask[i][j].before_data = 3;
		}
	}

	const char* fn_h = "../onyx_data/input_";
	const char* fn_t = "_2b_K.tif";
	char fn2[60] = { 0, };
	char num[10] = { 0, };

	Data_2Bit* data_2bit = NULL;
	int m_size = (readsize + 3) / 4;
	int index = 0;
	data_2bit = (Data_2Bit*)calloc(m_size, sizeof(Data_2Bit));

	TIFF* r_data;

	// 0~255 onyx 1bit 데이터를 읽어서 8bit로 변환 후 메모리에 저장한다. 
	for (int i = 0; i < 256; i++)
	{
		// 읽어들일 파일명 작업
		fn2[20] = { 0, };	// 문자열 초기화
		strcpy(fn2, fn_h);
		itoa(i, num, 10);
		strcat(fn2, num);
		strcat(fn2, fn_t);

		// tiff 파일 오픈
		r_data = TIFFOpen(fn2, "rb");

		for (int j = 0; j < datasize; j++)
		{
			index = 0;
			memset(data_2bit, 0, m_size);
			TIFFReadScanline(r_data, data_2bit, j, 0);

			//printf("\n");
			for (int k = 0; k < m_size; k++)
			{
				// bit 0
				// 현재 데이터가 이전 데이터와 다르면 그 때의 (픽셀 값 - 1)을 저장 후 인덱스 1 증가
	
				if (data_2bit[k].bit0 != mask[j][index].before_data)
				{
					switch (mask[j][index].count)
					{
					case 0:	mask[j][index].threshold1 = i - 1; break;	// 0 >> 1
					case 1:	mask[j][index].threshold2 = i - 1; break;	// 1 >> 2
					case 2:	mask[j][index].threshold3 = i - 1; break;	// 2 >> 3
					default: break;
					}
					mask[j][index].count++;
				}

				// 현재 데이터를 이전 데이터로 저장
				mask[j][index].before_data = data_2bit[k].bit0;

				// 인덱스 증가
				index++;

				// bit 1
				// 현재 데이터가 이전 데이터와 다르면 그 때의 (픽셀 값 - 1)을 저장 후 인덱스 1 증가
				//check[data_2bit[k].bit1]++;
				if (data_2bit[k].bit1 != mask[j][index].before_data)
				{
					switch (mask[j][index].count)
					{
					case 0:	mask[j][index].threshold1 = i - 1; break;	// 0 >> 1
					case 1:	mask[j][index].threshold2 = i - 1; break;	// 1 >> 2
					case 2:	mask[j][index].threshold3 = i - 1; break;	// 2 >> 3
					default: break;
					}
					mask[j][index].count++;
				}

				// 현재 데이터를 이전 데이터로 저장
				mask[j][index].before_data = data_2bit[k].bit1;

				// 인덱스 증가
				index++;

				// bit2
				// 현재 데이터가 이전 데이터와 다르면 그 때의 (픽셀 값 - 1)을 저장 후 인덱스 1 증가
				if (data_2bit[k].bit2 != mask[j][index].before_data)
				{
					switch (mask[j][index].count)
					{
					case 0:	mask[j][index].threshold1 = i - 1; break;	// 0 >> 1
					case 1:	mask[j][index].threshold2 = i - 1; break;	// 1 >> 2
					case 2:	mask[j][index].threshold3 = i - 1; break;	// 2 >> 3
					default: break;
					}
					mask[j][index].count++;
				}

				// 현재 데이터를 이전 데이터로 저장
				mask[j][index].before_data = data_2bit[k].bit2;

				// 인덱스 증가
				index++;


				// bit3
				// 현재 데이터가 이전 데이터와 다르면 그 때의 (픽셀 값 - 1)을 저장 후 인덱스 1 증가
				if (data_2bit[k].bit3 != mask[j][index].before_data)
				{
					switch (mask[j][index].count)
					{
					case 0:	mask[j][index].threshold1 = i - 1; break;	// 0 >> 1
					case 1:	mask[j][index].threshold2 = i - 1; break;	// 1 >> 2
					case 2:	mask[j][index].threshold3 = i - 1; break;	// 2 >> 3
					default: break;
					}
					mask[j][index].count++;
				}

				// 현재 데이터를 이전 데이터로 저장
				mask[j][index].before_data = data_2bit[k].bit3;

				// 인덱스 증가
				index++;
			}

		}


		TIFFClose(r_data);
	}

	// mask를 txt 파일로 쓴다.

	FILE* fp = fopen("onyx_mask_2Bit.txt", "w");
	for (int i = 0; i < datasize; i++)
	{
		fprintf(fp, "{");
		for (int j = 0; j < datasize; j++)
		{

			fprintf(fp, "{%d,%d,%d}, ", 
				mask[i][j].threshold1, mask[i][j].threshold2, mask[i][j].threshold3);

		}
		fprintf(fp, "},\n");
	}
	fclose(fp);
	

	free(data_2bit);
	//free(bit_data);
	//free(gray_data);

	return 0;
}