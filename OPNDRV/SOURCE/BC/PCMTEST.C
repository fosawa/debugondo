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
		
		puts ("PCMTEST  Ver 1.00 : PCM形式ファイルを演奏します");
		puts ("                                    Programmed by F.Osawa");
		puts ("Usage:");
		puts ("     PCMTEST [Options] PCMfilename");
		puts ("Options:");
		puts ("     /z メモリに常駐します");
		puts ("     /e 再生中のPCM音源を止めます");
		return(1);
		}
	
	
	/*OPNDRVのチェック*/
	opnnum = opncheck();
	if (opnnum == 0)
		{
		printf("OPNDRVが登録されていません\n");
		exit(1);
		}
	
	if (opnnum < 200)
		{
		printf("OPNDRVはVer2.00以降をお使いください\n");
		exit(1);
		}
	
	for (i = 1; i < argv; i++)
		{
		if ((*(argc[i]) != '/') && (*(argc[i]) != '-'))
			{
			/*ファイル名*/
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
	
		strlwr(argc[i]); /*大文字->小文字*/
		argc[i]++; /* /と-を飛ばす */
		
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
		printf("ファイル名を指定してください\n");
		return 1;
	}
	
	if (strchr(filename, '.') == NULL)
		{
		strcat(filename, ".PCM");
		}
	
	if ((fp = fopen(filename, "rb")) == NULL)
	{
		printf("ファイル[%s]が見つかりません\n", filename);
		exit(1);
	}
	
	
	/*ファイルサイズの取得*/
	
	handle = fileno(fp);
	length = filelength(handle);
	
	if ((dat = farmalloc(length)) == NULL)
		{
		fprintf(stderr, "演奏用バッファが確保できませんでした");
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
		/*常駐演奏*/
		
		pcmver = pcmplay2(dat, length);
		
		if (pcmver == -2)
		{
			printf("PCMファイルではありません\n");
			return 1;
		}
		
		if (pcmver == -1)
		{
			printf("OPNDRVのPCMバッファが足りません\n");
			printf("%dKB以上のバッファを確保してください\n", (length + 1024) / 1024);
			return 1;
		}
		
		return 0;
	}

	pcmver = pcmplay(dat);

	if (pcmver == 0)
	{
		printf("PCMファイルではありません\n");
		exit(1);
	}
	
	if (pcmver < opnnum)
	{
		printf("PCMデータのバージョンがOPNDRVよりも新しいので、再生できません\n");
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