#include <stdio.h>
#include <windows.h>
#include <stdlib.h>


int main(void)
{
	int num;
	FILE* fp_BNM = NULL;
	FILE* fp = NULL;
	BYTE* pix_bnm = NULL;

	printf("[1] : 256, [2] : 512\n");
	printf("input number : ");
	scanf("%d", &num);

	if (num == 1)
	{
		fp_BNM = fopen("BNM_256.bmp", "rb");
		fp = fopen("value_BNM_256.txt", "w");

		pix_bnm = (BYTE *)calloc(256 * 256, sizeof(BYTE));


		for (int i = 0; i < 256; i++)
		{
			fread(pix_bnm + (256 * i), sizeof(BYTE), 256, fp_BNM);
			for (int j = 0; j < 256; j++)
			{
				fprintf(fp, "%d, ", pix_bnm[i * 256 + j]);
			}
			fprintf(fp, "\n");
		}
	}
	else if (num == 2)
	{
		fp_BNM = fopen("BNM_512.bmp", "rb");
		fp = fopen("value_BNM_512.txt", "w");

		pix_bnm = (BYTE *)calloc(512 * 512, sizeof(BYTE));


		for (int i = 0; i < 512; i++)
		{
			fread(pix_bnm + (512 * i), sizeof(BYTE), 512, fp_BNM);
			for (int j = 0; j < 512; j++)
			{
				fprintf(fp, "%d, ", pix_bnm[i * 512 + j]);
			}
			fprintf(fp, "\n");
		}
	}
	else 
	{
		exit(0);
	}


	fclose(fp_BNM);
	fclose(fp);
	free(pix_bnm);

	return 0;
}