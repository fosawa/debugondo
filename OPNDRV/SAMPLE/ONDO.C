	/*
	�f�o�b�O�����̎��\���p�v���O����
	
						Programmed by F.Osawa
	*/
	
	#include <stdio.h>
	#include "opnlib.h"
	#include <alloc.h>
	#include <io.h>
	#include <dos.h>
	#include <stdlib.h>
	#include <string.h>
	
int main()
{
	FILE *fp;
	unsigned char huge *dat;
	unsigned char huge *dat2;
	int opnnum;
	int handle;
	unsigned long length;
	int i;
	unsigned char leng1,leng2;
	unsigned long j;
	int pt;
	int dataver;
	char buf[8192];

	unsigned char karaoke[]="��\0��\0��\0��\0��\0��\0��\0��\0��\0��  \0"
							"��\0��\0��\0��\0��\0��\0��\0��\0��\n\0"
							"��\0��\0��\0��\0��\0��\0��\0�B\0��  \0"
							"��\0��\0��\0��\0��\0��\0��\0��\0��\0��\0��\0��\0�Ȃ�\n\0"
							"��\0��\0��\0��\0��\0��\0�Q\0��\0��\0��\0��\n\0"
							
  							"�o\0�O\0��\0�T\0��\0��\0��\0�T\0��\n\0"
  							"�o\0�O\0��\0��\0��\0��\0��\0�T\0��\n\0"
  							"��\0�C\0��\0��\0��\0��\0��\0�O\0�T\0��\n\0"
  							"��\0��\0��\0��\0��\0��\0��\0��\0��\0��\n\0"
  							
  							"��\0��\0�O\0��\0��\0��\0��\0��\0��  \0"
  							"��\0��\0��\0��\0�H\0�X\0�p\0�Q\0�e�B\0�[\n\0"
  							"��\0��\0��\0��\0��\0��\0��\0��\0��\0��\0��\0��\0�Ȃ�  \0"
  							"��\0��\0��\0��\0�Q\0��\0��\0��\0��\0��\0��\0��\0��\n\0"
							"��\0��\0��\0��\0��\0��\0�Q\0��\0��\0��\0��\n\0"
							
  							"�o\0�O\0��\0�T\0��\0��\0��\0�T\0��\n\0"
  							"�o\0�O\0��\0��\0��\0��\0��\0�T\0��\n\0"
  							"��\0�C\0��\0��\0��\0��\0��\0�O\0�T\0��\n\0"
  							"��\0��\0��\0��\0��\0��\0��\0��\0��\0��\n\0"
  							
  							"�o\0�O\0�o\0�O\0�o\0�O\0�o\0�O\0�o\0�O\0�o�C\0�o�C\n\0"
  							"�o\0�O\0�o\0�O\0�o\0�O\0�o\0�O\0�o\0�O\0�o�C\0�o�C\n\0"
  							"��\0��\0��\0��\0��\0��\0��\0��\0��\0��\0��\0��\n\0"
  							
  							"�o\0�O\0�o\0�O\0�o\0�O\0�o\0�O\0�o\0�O\0�o�C\0�o�C\n\0"
  							"�o\0�O\0�o\0�O\0�o\0�O\0�o\0�O\0�o\0�O\0�o�C\0�o�C\n\0"
  							"��\0��\0��\0��\0��\0��\0��\0��\0��\0��\0��\0��\0"
  							
							"*";
	
	
	opnnum = opncheck();
	if (opnnum == 0)
	{
		printf("OPNDRV���o�^����Ă��܂���\n");
		exit(1);
	}
	
	/*�t�@�C�����[�h*/
	if ((fp = fopen("ondo.opn", "rb")) == NULL)
	{
		printf("ondo.opn��������܂���\n");
		return 1;
	}
	
	setvbuf(fp, buf, _IOFBF, 8192);
	
	/*�t�@�C���T�C�Y�̎擾*/
	
	handle = fileno(fp);
	length = filelength(handle);
	
	if ((dat = farmalloc(length)) == NULL)
		{
		fprintf(stderr, "���t�p�o�b�t�@���m�ۂł��܂���ł���");
		exit(1);
		}
	
	dat2 = dat;
	for (j = 0; j < length; j++)
	{
		*dat2 = fgetc(fp);
		dat2++;
	}
	
	fclose(fp);
	
	
	printf ("\x1b[>5h\x1b[2J�f�o�b�O����\n");
	printf ("                                         �쎌�E��ȁF���V���F\n");
	printf ("                                         ���F�ɓ��Đ�\n");
	printf ("\n",0);
	
	/*�̎���΂ŕ\��*/
	pt=0;
	printf ("\x1b[s\x1b[32m");

	while (karaoke[pt]!='*')
		{
		printf ("%s",&karaoke[pt]);
		pt+=strlen(&karaoke[pt])+1;
		}

	printf ("\x1b[u");
	
	
	dataver = opnstart(dat);
	if (dataver == 0)
	{
		printf("OPN�t�@�C���ł͂���܂���\n");
		return 1;
	}
	
	if (dataver < opnnum)
	{
		printf("OPNDRV�̃o�[�W�������f�[�^�̃o�[�W���������Â����߉��t�ł��܂���\n");
		return 1;
	}
	
	pt=0;
	leng1=0;
	while (kbhit()==0) 
	{
		int status;
		
		leng2=varread(0);

		if ((karaoke[pt]!='*') && (leng1 != leng2))
		{
			printf ("%s",&karaoke[pt]);
			pt+=strlen(&karaoke[pt])+1;
		}
	    leng1=leng2;
		status = getstatus();
		if ((status & 1) != 0) break;
	}

	mend();

	printf ("\x1b[>5l\n");
	
	return 0;
}
