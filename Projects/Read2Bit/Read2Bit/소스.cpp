#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include "tiff.h"
#include "tiffconf.h"
#include "tiffio.h"
#include "tiffvers.h"


int datasize = 744;
int readsize = 744;

// data 값을 추출한 마스크 값 
BYTE mask[744][744];

typedef struct Data {
	unsigned char bit0 : 1;
	unsigned char bit1 : 1;
	unsigned char bit2 : 1;
	unsigned char bit3 : 1;
	unsigned char bit4 : 1;
	unsigned char bit5 : 1;
	unsigned char bit6 : 1;
	unsigned char bit7 : 1;
};

int main(void)
{
	const char* fn_h = "../onyx_data/input_";
	const char* fn_t = "_1b_K.tif";
	char fn2[60] = { 0, };
	char num[10] = { 0, };
	Data* bit_data = NULL;
	//BYTE* gray_data = NULL;
	int m_size = (readsize + 7) / 8;
	int index = 0;
	//printf("%d\n", sizeof(Data));
	bit_data = (Data*)calloc(m_size, sizeof(Data));
	//gray_data = (BYTE*)calloc(datasize, sizeof(BYTE));

	TIFF* r_data;

	// 0~255 onyx 1bit 데이터를 읽어서 8bit로 변환 후 메모리에 저장한다. 
	for (int i = 1; i < 255; i++)
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
			memset(bit_data, 0, m_size);
			TIFFReadScanline(r_data, bit_data, j, 0);
			for (int k = 0; k < m_size; k++)
			{
				if (bit_data[k].bit7 == 1)
				{
					//if(index < datasize)
					mask[index][j] = i;
					index++;
					//printf("%d ", index);
					//printf("bit7 : %d\n", bit_data[k].bit7);
				}
				else if (bit_data[k].bit7 == 0)
				{

					index++;
					//printf("%d ", index);
					//printf("bit7 : %d\n", bit_data[k].bit7);
				}
				if (bit_data[k].bit6 == 1)
				{
					//if (index < datasize)
					mask[index][j] = i;
					index++;
					//printf("%d ", index);
					//printf("bit6 : %d\n", bit_data[k].bit6);
				}
				else if (bit_data[k].bit6 == 0)
				{
					index++;
					//printf("%d ", index);
					//printf("bit6 : %d\n", bit_data[k].bit6);
				}
				if (bit_data[k].bit5 == 1)
				{
					//if (index < datasize)
					mask[index][j] = i;
					index++;
					//printf("%d ", index);
					//printf("bit5 : %d\n", bit_data[k].bit5);
				}
				else if (bit_data[k].bit5 == 0)
				{
					index++;
					//printf("%d ", index);
					//printf("bit5 : %d\n", bit_data[k].bit5);
				}
				if (bit_data[k].bit4 == 1)
				{
					//if (index < datasize)
					mask[index][j] = i;
					index++;
					//printf("%d ", index);
					//printf("bit4 : %d\n", bit_data[k].bit4);
				}
				else if (bit_data[k].bit4 == 0)
				{
					index++;
					//printf("%d ", index);
					//printf("bit4 : %d\n", bit_data[k].bit4);
				}
				if (bit_data[k].bit3 == 1)
				{
					//if (index < datasize)
					mask[index][j] = i;
					index++;
					//printf("%d ", index);
					//printf("bit3 : %d\n", bit_data[k].bit3);
				}
				else if (bit_data[k].bit3 == 0)
				{
					index++;
					//printf("%d ", index);
					//printf("bit3 : %d\n", bit_data[k].bit3);
				}
				if (bit_data[k].bit2 == 1)
				{
					//if (index < datasize)
					mask[index][j] = i;
					index++;
					//printf("%d ", index);
					//printf("bit2 : %d\n", bit_data[k].bit2);
				}
				else if (bit_data[k].bit2 == 0)
				{
					index++;
					//printf("%d ", index);
					//printf("bit2 : %d\n", bit_data[k].bit2);
				}
				if (bit_data[k].bit1 == 1)
				{
					//if (index < datasize)
					mask[index][j] = i;
					index++;
					//printf("%d ", index);
					//printf("bit1 : %d\n", bit_data[k].bit1);
				}
				else if (bit_data[k].bit1 == 0)
				{
					index++;
					//printf("%d ", index);
					//printf("bit1 : %d\n", bit_data[k].bit1);
				}
				if (bit_data[k].bit0 == 1)
				{
					//if (index < datasize)
					mask[index][j] = i;
					index++;
					//printf("%d ", index);
					//printf("bit0 : %d\n", bit_data[k].bit0);
				}
				else if (bit_data[k].bit0 == 0)
				{
					index++;
					//printf("%d ", index);
					//printf("bit0 : %d\n", bit_data[k].bit0);
				}
			}

		}
		

		TIFFClose(r_data);
	}

	// mask를 txt 파일로 쓴다.
	FILE* fp = fopen("onyx_mask.txt", "w");

	for (int i = 0; i < datasize; i++)
	{
		for (int j = 0; j < datasize; j++)
		{
			fprintf(fp, "%d%c ", mask[i][j], ',');
		}
		fprintf(fp, "\n");
	}

	fclose(fp);

	free(bit_data);
	//free(gray_data);

	return 0;
}