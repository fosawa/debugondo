	/* PCM PLAY TEST Program 
	
	
				Programmed by F.Osawa
	*/
	
	#include <stdio.h>
	#include <dos.h>
	#include <io.h>
	#include <stdlib.h>
	#include <string.h>
	#include <malloc.h>
	#include "opnlib.h"

int main(int argv, char **argc)
{
	FILE *fp;
	char filename[FILENAME_MAX];
	unsigned char huge *dat;
	unsigned char huge *dat2;
	
	int handle;
	unsigned long length;
	int i;
	int opnnum;
	unsigned int offset;
	int filenum = 0;
	unsigned long k;
	int pcmver;
	int optflag = 0;
	
	if (argv <= 1)
		{
		msg:
		
		puts ("PCMTEST  Ver 1.00 : PCM�`���t�@�C�������t���܂�");
		puts ("                                    Programmed by F.Osawa");
		puts ("Usage:");
		puts ("     PCMTEST [Options] PCMfilename");
		puts ("Options:");
		puts ("     /z �������ɏ풓���܂�");
		puts ("     /e �Đ�����PCM�������~�߂܂�");
		return(1);
		}
	
	
	/*OPNDRV�̃`�F�b�N*/
	opnnum = opncheck();
	if (opnnum == 0)
		{
		printf("OPNDRV���o�^����Ă��܂���\n");
		exit(1);
		}
	
	if (opnnum < 200)
		{
		printf("OPNDRV��Ver2.00�ȍ~�����g����������\n");
		exit(1);
		}
	
	for (i = 1; i < argv; i++)
		{
		if ((*(argc[i]) != '/') && (*(argc[i]) != '-'))
			{
			/*�t�@�C����*/
			switch (filenum)
				{
				case 0:
							strcpy(filename, argc[i]);
							break;
				default:	
							fputs("... too many filenames", stderr);
							return(1);
				}
			filenum++;
			continue;
			}
	
		strlwr(argc[i]); /*�啶��->������*/
		argc[i]++; /* /��-���΂� */
		
		if (strstr (argc[i], "z") == argc[i])
		{
			optflag = 1;
			continue;
		}
		
		if (strstr (argc[i], "e") == argc[i])
		{
			pcmstop();
			return 0;
		}
		
		if ((strstr (argc[i], "h") == argc[i]) || (strstr (argc[i], "?") == argc[i])) goto msg;
		
		fprintf (stderr, "Incorrect option -[%s]", argc[i] - 1);
		return(1);
		
		}
	
	if (filenum == 0)
	{
		printf("�t�@�C�������w�肵�Ă�������\n");
		return 1;
	}
	
	if (strchr(filename, '.') == NULL)
		{
		strcat(filename, ".PCM");
		}
	
	if ((fp = fopen(filename, "rb")) == NULL)
	{
		printf("�t�@�C��[%s]��������܂���\n", filename);
		exit(1);
	}
	
	
	/*�t�@�C���T�C�Y�̎擾*/
	
	handle = fileno(fp);
	length = filelength(handle);
	
	if ((dat = farmalloc(length)) == NULL)
		{
		fprintf(stderr, "���t�p�o�b�t�@���m�ۂł��܂���ł���");
		exit(1);
		}
	
	dat2 = dat;
	
	for (k = 0; k < length; k++)
	{
		*dat2 = fgetc(fp);
		dat2++;
	}
	
	if (optflag == 1)
	{
		/*�풓���t*/
		
		pcmver = pcmplay2(dat, length);
		
		if (pcmver == -2)
		{
			printf("PCM�t�@�C���ł͂���܂���\n");
			return 1;
		}
		
		if (pcmver == -1)
		{
			printf("OPNDRV��PCM�o�b�t�@������܂���\n");
			printf("%dKB�ȏ�̃o�b�t�@���m�ۂ��Ă�������\n", (length + 1024) / 1024);
			return 1;
		}
		
		return 0;
	}

	pcmver = pcmplay(dat);

	if (pcmver == 0)
	{
		printf("PCM�t�@�C���ł͂���܂���\n");
		exit(1);
	}
	
	if (pcmver < opnnum)
	{
		printf("PCM�f�[�^�̃o�[�W������OPNDRV�����V�����̂ŁA�Đ��ł��܂���\n");
		exit(1);
	}
	
	while (kbhit() == 0)
	{
		int status;
		status = getstatus();
		if ((status & 2) != 0) break;
	}
	
	pcmstop();
	
	farfree(dat);

	return 0;
}
