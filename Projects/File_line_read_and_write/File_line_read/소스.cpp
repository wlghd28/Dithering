#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

#pragma warning(disable : 4996)

// 비트맵 이미지 정보를 받아오기 위한 구조체 변수
BITMAPFILEHEADER bfh;
BITMAPINFOHEADER bih;
RGBQUAD * rgb;

// 이미지 정보를 다루기 위해 사용하는 변수
int bpl, bph;
unsigned char* pix; // 라인단위로 읽어서 이미지 데이터를 저장해놓을 메모리
int* pixE;	// 에러디퓨전 에러값

long point_write; // 파일포인터 쓰기 위치변수
long point_read; // 파일포인터 읽기 위치변수
long point_end; // 파일의 끝 위치변수

void ErrorDiff();

int main(void)
{
	FILE* fp;
	//fp = fopen("EDIMAGE.bmp", "r+b");
	fp = fopen("newEDIMAGE2_720.bmp", "r+b");
	if (fp == NULL)
	{
		printf("File not found!!\n");
		return 0;
	}

	// 파일의 마지막 지점을 구한다
	fseek(fp, 0, SEEK_END);
	point_end = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	printf("file size : %d\n", point_end);

	// 파일을 읽어서 데이터를 추출
	fread(&bfh, sizeof(bfh), 1, fp);
	fread(&bih, sizeof(bih), 1, fp);
	rgb = (RGBQUAD*)malloc(sizeof(RGBQUAD) * 256);
	fread(rgb, sizeof(RGBQUAD), 256, fp);
	point_write = ftell(fp);
	// BPL을 맞춰주기 위해서 픽셀데이터의 사이즈를 4의 배수로 조정
	bpl = (bih.biWidth + 3) / 4 * 4;
	bph = (bih.biHeight + 3) / 4 * 4;
	printf("image width : %d, height : %d\n", bpl, bph);
	
	// 2줄씩 데이터를 읽어서 메모리에 담아 놓는다
	pix = (unsigned char*)calloc(bpl * 2, sizeof(unsigned char));
	pixE = (int*)calloc(bpl * 2, sizeof(int));
	fread(pix, sizeof(unsigned char), bpl * 2, fp);
	point_read = ftell(fp);
	//printf("%d\n", ftell(fp));

	
	while (1)
	{	
		ErrorDiff();

		// 디퓨전 계산이 끝난 줄을 write 한다
		fseek(fp, point_write, SEEK_SET);
		fwrite(pix, sizeof(unsigned char), bpl, fp);
		point_write = ftell(fp);
		//printf("%d\n", point_write);
		
		// 현재 파일포인터가 파일의 끝일 경우 반복문 탈출
		if (ftell(fp) >= point_end)
		{
			printf("파일의 끝!\n");
			break;
		}
		
		// pix에 저장되어 있는 값을 한 줄씩 밀어 올린다
		for (int i = 0; i < bpl; i++)
		{
			pix[i] = pix[i + bpl];
			pixE[i] = pixE[i + bpl];
			pixE[i + bpl] = 0;
		}

		fseek(fp, point_read, SEEK_SET);
		fread(pix + bpl, sizeof(unsigned char), bpl, fp);
		point_read = ftell(fp);
		//printf("%d\n", point_read);
	}
	
	free(rgb);
	free(pix);
	free(pixE);
	
	fclose(fp);

	return 0;
}
void ErrorDiff()
{
	// Floyd Steinberg 단방향	
	int quant_error = 0;	// 초기 디더링 작업을 할때 쓰일 에러 가중치 변수

	for (int y = 0; y < 1; y++)
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

		}
	}
}