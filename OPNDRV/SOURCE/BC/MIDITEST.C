	/*
	OPNDRV MIDI+
	
	  �ŁA�W��MIDI�t�@�C�������t������T���v��
	  
								Programmed by F.Osawa
	*/

	#include <stdio.h>
	#include <dos.h>
	#include <io.h>
	#include <stdlib.h>
	#include <string.h>
	#include <alloc.h>
	#include "opnlib.h"
	
int main(int argv, char **argc)
{
	long int fsize;
	long int i;
	unsigned char huge *dat;
	int	drv;
	FILE *fp;
	int key;
	int tempo = 256;
	int rc;
	char filename[FILENAME_MAX];
	int filenum = 0;
	int intf = 1;
	int optflag = 0;

	/*OPNDRV�̃`�F�b�N*/
	drv = opncheck();
	if (drv == 0) 
		{printf ("OPNDRV���g�ݍ��܂�Ă��܂���\n"); return(1);}
		
	if (drv < 200)
		{printf ("OPNDRV�̃o�[�W�������Â����܂�\n");
		 return(1);
		 }
	

	if (argv <= 1)
		{
		msg:
		
		puts ("MIDITEST  Ver 1.00 : �W��MIDI�`���t�@�C�������t���܂�");
		puts ("                                    Programmed by F.Osawa");
		puts ("Usage:");
		puts ("     MIDITEST [/m] [/r] MIDIfilename");
		puts ("Options:");
		puts ("     /m MPU-401�C���^�[�t�F�[�X���g�p(Default)");
		puts ("     /r RS-232C�C���^�[�t�F�[�X���g�p");
		puts ("     /z MIDI�f�[�^���풓���t������");
		puts ("     /e z�I�v�V�����ŏ풓���t����MIDI���~����");
		puts ("     /c e�I�v�V�����Ŏ~�߂����̂��ĊJ����");
		return(1);
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
		
		if (strstr (argc[i], "m") == argc[i])
		{
			intf = 1;
			continue;
		}
		
		if (strstr (argc[i], "r") == argc[i])
		{
			intf = 2;
			continue;
		}
		
		if (strstr (argc[i], "z") == argc[i])
		{
			optflag = 1;
			continue;
		}
		
		if (strstr(argc[i], "e") == argc[i])
		{
			midistop();
			return 0;
		}
		
		if (strstr(argc[i], "c") == argc[i])
		{
			midicont();
			return 0;
		}
		
		if ((strstr (argc[i], "h") == argc[i]) || (strstr (argc[i], "?") == argc[i])) goto msg;
		
		fprintf (stderr, "Incorrect option -[%s]", argc[i] - 1);
		return(1);
		
		}
	
	if (filenum == 0)
	{
		printf("�t�@�C����������܂���\n");
		return 1;
	}
	
	if (strchr(filename, '.') == NULL)
		{
		strcat(filename, ".MID");
		}

	
	if ((fp = fopen(filename, "rb")) == NULL)
		{
		printf ("�t�@�C��[%s]��������܂���\n", filename);
		return(1);
		}
	
	/*�t�@�C���̒������擾*/
	
	/*���̕��̃��������m��*/
	
	fsize = filelength(fileno(fp));

	if ((dat = farmalloc(fsize)) == NULL)
		{
		printf ("���������m�ۂł��܂���ł���\n");
		fclose(fp);
		return(1);
		}
	
	/*���[�h*/
	/*�{���́Afread�֐����g�������̂����ǁAfar������˂�*/
	
	
	for (i = 0; i < fsize; i++)
		{
		dat[i] = fgetc(fp);
		}
	
	
	fclose(fp);
	
	/*�C�j�V�����C�Y*/
	
	if (midiinit(intf) != 0) 
	{
		char *intfstr[] ={"MPU-401", "RS-232C"};
		printf("�w�肵��MIDI�C���^�[�t�F�[�X[%s]�͎g�p�ł��܂���\n", intfstr[intf - 1]);
		return 1;
	}
	
	/*���t�J�n*/
	midiext();
	
	/*���Z�b�g�I�[���R���g���[���[�𑗂�*/
	
	for (i = 0; i < 16; i++)
		{
		midisend(0xb0 + i);
		midisend(0x79);
		midisend(0x00);
		}
	
	if (optflag == 1)
	{
		/*�f�[�^�풓*/
		midistop();
		rc = midistart2(dat, fsize);
		
		farfree(dat);
		
		if (rc == -1)
		{
			printf("OPNDRV��MIDI�o�b�t�@������܂���\n");
			printf("%dKB�ȏ�̃o�b�t�@���m�ۂ��Ă�������\n", (fsize + 1024) / 1024);
			midiclose();
			return 1;
		}
		
		if (rc != 0)
		{
			printf("�t�@�C����MIDI�t�@�C���Ƃ݂͂Ȃ��܂���\n");
			midiclose();
			return 1;
		}
		
		return 0;
	}
	
	if ((rc = midistart(dat))!= 0) 
		{
		printf ("�t�@�C����MIDI�t�@�C���Ƃ݂͂Ȃ��܂���\n");
		midiclose();
		farfree(dat);
		return(1);
		}
	
	printf ("���t��!\n");
	printf ("�L�[����:\n");
	printf ("[e]: end\n");
	printf ("[u]: tempo up\n");
	printf ("[d]: tempo down\n");
	printf ("[p]: pause\n");
	printf ("[c]: continue\n");
	
	do
	{
	
	while (kbhit()== 0)
	{
	if ((getmidi() & 0xff) == 2)
		{
		printf ("���t�I��\n");
		goto	end;
		}
	}

	key = getch();
	if ((key == 'd') || (key == 'D'))
		{
		/*�e���|�_�E��*/
		tempo++;
		if (tempo > 512) tempo = 512;
		printf ("tempo = [%f] �{\n", tempo / 256.0);
		miditempo(tempo);
		}
		
	if ((key == 'u') || (key == 'U'))
		{
		/*�e���|�A�b�v*/
		tempo--;
		if (tempo < 128) tempo = 128;
		printf ("tempo = [%f] �{\n", tempo / 256.0);
		miditempo(tempo);
		}
	
	if ((key == 'p') || (key == 'P'))
		{
		/*�ꎞ��~*/
		printf ("[pause]\n");
		midistop();
		}
	
	if ((key == 'c') || (key == 'C'))
		{
		/*�ĊJ*/
		printf ("[continue]\n");
		midicont();
		}
	
	} while ((key != 'e') && (key != 'E'));
	
end:	
	/*���t�I��*/
	
	midistop();
	
	/*MIDI�g�p�I��*/
	
	midiclose();
	
	
	farfree(dat);
	
	return(0);
}
