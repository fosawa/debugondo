	/*
	opnファイル演奏テストプログラム
	
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
	FILE *fp;
	char filename[FILENAME_MAX];
	unsigned char huge *dat;
	unsigned char huge *dat2;
	int handle;
	unsigned long length;
	int i;
	int opnnum;
	unsigned int offset;
	unsigned long j;
	char buf[8192];
	int dataver;
	int filenum = 0;
	int optflag = 0;
	
	if (argv <= 1)
		{
		msg:
		
		puts ("OPNTEST  Ver 1.00 : OPN形式ファイルを演奏します");
		puts ("                                    Programmed by F.Osawa");
		puts ("Usage:");
		puts ("     OPNTEST [Options] OPNfilename");
		puts ("Options:");
		puts ("     /z 演奏データをメモリに常駐させます");
		puts ("     /e 常駐している音楽を止めます");
		return(1);
		}
	
	
	/*OPNDRVのチェック*/
	opnnum = opncheck();
	if (opnnum == 0)
		{
		printf("OPNDRVが登録されていません\n");
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
		
		if ((strstr (argc[i], "h") == argc[i]) || (strstr (argc[i], "?") == argc[i])) goto msg;
		
		if ((strstr (argc[i], "z") == argc[i]))
		{
			optflag = 1;
			continue;
		}
		
		if ((strstr (argc[i], "e") == argc[i]))
		{
			optflag = 2;
			mend();
			return 0;
		}
		
		fprintf (stderr, "Incorrect option -[%s]", argc[i] - 1);
		return(1);
		
		}
	
	if (filenum == 0)
	{
		printf("ファイル名がありません\n");
		return 1;
	}
	
	if (strchr(filename, '.') == NULL)
		{
		strcat(filename, ".OPN");
		}
	
	if ((fp = fopen(filename, "rb")) == NULL)
	{
		printf("ファイル[%s]が見つかりません\n", filename);
		exit(1);
	}
	
	setvbuf(fp, buf, _IOFBF, 8192);
	
	/*ファイルサイズの取得*/
	
	handle = fileno(fp);
	length = filelength(handle);
	
	if ((dat = farmalloc(length)) == NULL)
		{
		fprintf(stderr, "演奏用バッファが確保できませんでした");
		exit(1);
		}
	
	dat2 = dat;
	for (j = 0; j < length; j++)
	{
		*dat2 = fgetc(fp);
		dat2++;
	}
	
	fclose(fp);
	
	if (optflag == 1)
	{
		dataver = opnstart2(dat, length);
	
		if (dataver == 1)
		{
			printf("OPNファイルではありません\n");
			return 1;
		}
	
		if (dataver == -1)
		{
			printf("OPNDRVのバッファが足りません\n");
			printf("%dKB以上のバッファを確保してください\n", (length + 1024) / 1024);
			return 1;
		}
		
		farfree(dat);
		return 0;
	}
	
	if (optflag == 0)
	{
		dataver = opnstart(dat);
		if (dataver == 0)
		{
			printf("OPNファイルではありません\n");
			return 1;
		}
	
		if (dataver < opnnum)
		{
			printf("OPNDRVのバージョンがデータのバージョンよりも古いため演奏できません\n");
			return 1;
		}
		
		printf("Now playing [%s]\n", filename);
		printf("Hit any key to exit\n");
	
		while (kbhit() == 0)
		{
			int status;
			status = getstatus();
			if ((status & 1) != 0) break;
		}
		mend();
		farfree(dat);
		
		return 0;
	}
	
	if (optflag == 2)
	{
		mend();
	}
	
	return 0;
}

	