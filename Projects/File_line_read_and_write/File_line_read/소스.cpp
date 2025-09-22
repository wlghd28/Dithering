#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

#pragma warning(disable : 4996)

// ��Ʈ�� �̹��� ������ �޾ƿ��� ���� ����ü ����
BITMAPFILEHEADER bfh;
BITMAPINFOHEADER bih;
RGBQUAD * rgb;

// �̹��� ������ �ٷ�� ���� ����ϴ� ����
int bpl, bph;
unsigned char* pix; // ���δ����� �о �̹��� �����͸� �����س��� �޸�
int* pixE;	// ������ǻ�� ������

long point_write; // ���������� ���� ��ġ����
long point_read; // ���������� �б� ��ġ����
long point_end; // ������ �� ��ġ����

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

	// ������ ������ ������ ���Ѵ�
	fseek(fp, 0, SEEK_END);
	point_end = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	printf("file size : %d\n", point_end);

	// ������ �о �����͸� ����
	fread(&bfh, sizeof(bfh), 1, fp);
	fread(&bih, sizeof(bih), 1, fp);
	rgb = (RGBQUAD*)malloc(sizeof(RGBQUAD) * 256);
	fread(rgb, sizeof(RGBQUAD), 256, fp);
	point_write = ftell(fp);
	// BPL�� �����ֱ� ���ؼ� �ȼ��������� ����� 4�� ����� ����
	bpl = (bih.biWidth + 3) / 4 * 4;
	bph = (bih.biHeight + 3) / 4 * 4;
	printf("image width : %d, height : %d\n", bpl, bph);
	
	// 2�پ� �����͸� �о �޸𸮿� ��� ���´�
	pix = (unsigned char*)calloc(bpl * 2, sizeof(unsigned char));
	pixE = (int*)calloc(bpl * 2, sizeof(int));
	fread(pix, sizeof(unsigned char), bpl * 2, fp);
	point_read = ftell(fp);
	//printf("%d\n", ftell(fp));

	
	while (1)
	{	
		ErrorDiff();

		// ��ǻ�� ����� ���� ���� write �Ѵ�
		fseek(fp, point_write, SEEK_SET);
		fwrite(pix, sizeof(unsigned char), bpl, fp);
		point_write = ftell(fp);
		//printf("%d\n", point_write);
		
		// ���� ���������Ͱ� ������ ���� ��� �ݺ��� Ż��
		if (ftell(fp) >= point_end)
		{
			printf("������ ��!\n");
			break;
		}
		
		// pix�� ����Ǿ� �ִ� ���� �� �پ� �о� �ø���
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
	// Floyd Steinberg �ܹ���	
	int quant_error = 0;	// �ʱ� ����� �۾��� �Ҷ� ���� ���� ����ġ ����

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