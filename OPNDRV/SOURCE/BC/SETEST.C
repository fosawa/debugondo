	/*
	seファイル演奏テストプログラム
	
							Programmed by F.Osawa
	*/
	#include <stdio.h>
	#include <dos.h>
	#include <io.h>
	#include <stdlib.h>
	#include <string.h>
	#include <alloc.h>
	#include "opnlib.h"

	#define	L64 3
	
int main(int argv, char **argc)
{
	FILE *fp;
	char filename[FILENAME_MAX], filename2[FILENAME_MAX];
	unsigned char far *dat;
	unsigned char far *d2;
	
	unsigned char far *dat3;

	int handle;
	unsigned long length;
	int i;
	unsigned int offset;
	int semax = 0;
	int senum = 1;
	int t = 120;
	unsigned char tb;
	int filenum = 0;
	int opnnum;
	int c;
	int sever;
	char buf[8192];
	unsigned long length2;
	int optflag = 0;
	
	if (argv <= 1)
		{
		msg:
		
		puts ("SETEST  Ver 1.00 : SE形式ファイルを演奏します");
		puts ("                                    Programmed by F.Osawa");
		puts ("Usage:");
		puts ("     SETEST [Options] SEfilename [OPNfilename]");
		puts ("Options:");
		puts ("     /z     SE/OPNファイルを常駐させます");
		puts ("     /e     常駐中の演奏を停止します");
		puts ("     /sxx   /zで常駐中のSEのxx番を鳴らします");
		
		return 1;
		}

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
	

	/*オプションチェック*/
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
				case 1:
							strcpy(filename2, argc[i]);
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
			/*データ常駐*/
			optflag = 1;
			continue;
		}
		
		if (strstr (argc[i], "e") == argc[i])
		{
			/*終了*/
			mend();
			return 0;
		}
		
		if (strstr (argc[i], "s") == argc[i])
		{
			/*SEを鳴らす*/
			sestart2(atoi(argc[i] + 1));
			return 0;
		}
		
		if ((strstr (argc[i], "h") == argc[i]) || (strstr (argc[i], "?") == argc[i])) goto msg;
	
		fprintf (stderr, "Incorrect option -[%s]", argc[i] - 1);
		return(1);
		}
	
	if (strchr(filename, '.') == NULL)
		strcat(filename, ".SE");
	
	if (filenum == 2)
		{
		if (strchr(filename2, '.') == NULL)
			strcat(filename2, ".OPN");
		}
	
	if ((fp = fopen(filename, "rb")) == NULL)
	{
		printf("ファイル[%s]が見つかりません\n", filename);
		exit(1);
	}
	
	setvbuf(fp, buf, _IOFBF, 8192);
	
	/*ファイルサイズの取得*/
	
	handle = fileno(fp);
	length2 = length = filelength(handle);
	
	if ((dat = farmalloc(length)) == NULL)
		{
		fprintf(stderr, "演奏用バッファが確保できませんでした");
		exit(1);
		}
	
	d2 = dat;
	while (!feof(fp))
	{
		*d2 = fgetc(fp);
		d2++;
	}
	
	fclose(fp);
	
	sever = getsever(dat);
	
	if (sever == 0)
	{
		printf("SEファイルではありません\n");
		exit(1);
	}
	
	if (getsever(dat) < opnnum)
	{
		printf("SEファイルのバージョンよりもOPNDRVのバージョンの方が古いため、演奏できません\n");
		exit(1);
	}
	
	/*SEデータ数を読む*/
	semax = getsemax(dat);
	
	/*OPNファイルがあればロード*/
	if (filenum == 2)
		{
		unsigned char huge *dat4;
		int opnver;
		
		if ((fp = fopen(filename2, "rb")) == NULL)
			{
			printf("ファイル[%s]が見つかりません\n", filename2);
			exit(1);
			}
		/*ファイルサイズの取得*/
	
		setvbuf(fp, buf, _IOFBF, 8192);

		handle = fileno(fp);
		length = filelength(handle);
	
		if ((dat3 = farmalloc(length)) == NULL)
			{
			fprintf(stderr, "演奏用バッファが確保できませんでした");
			exit(1);
			}
		
		dat4 = dat3;
		while (!feof(fp))
		{
			*dat4 = fgetc(fp);
			dat4++;
		}
	
		fclose(fp);
		
		if (optflag == 0)
		{
			opnver = opnstart(dat3);
			if (opnver == 0)
			{
				printf("OPNファイルではありません\n");
				exit(1);
			}
			if (opnver < opnnum)
			{
				printf("SEファイルのバージョンよりもOPNDRVのバージョンの方が古いため、演奏できません\n");
				exit(1);
			}
		}
		else
		{
			opnver = opnstart2(dat, length);
	
			if (opnver == 1)
			{
				printf("OPNファイルではありません\n");
				return 1;
			}
	
			if (opnver == -1)
			{
				printf("OPNDRVのバッファが足りません\n");
				printf("%dKB以上のバッファを確保してください\n", (length + 1024) / 1024);
				return 1;
			}
		
			farfree(dat);
		}
		
		}
	
	if (optflag == 1)
	{
		int sever;
		
		sever = sestore(dat, length2);
		
		if (sever == 0)
		{
			printf("SEファイルではありません\n");
			return 1;
		}
		
		if (sever == -1)
		{
			printf("OPNDRVのSEバッファが足りません\n");
			printf("%dKB以上のバッファを確保してください\n", (length2 + 1024) / 1024);
			return 1;
		}
	
		printf("SEデータが常駐しました\n");
		return 0;
	}
	
	printf("[%s] ... SE Max: %d \n", filename, semax - 1);
	printf(" [↑]: SE number up [↓]: SE number down [SPACE]: SE Play \n");
	printf("[ESC]: Quit\n\n");
	
	senum = 0;
	
	printf("\x1b[>5h");
	
	do
	{
	const char *ch[] =
		{
		"2ch    ",
		"    3ch",
		"2ch&3ch"};
	int cc, cd;
	
	switch (dat[senum * 5 + 1 + 5] & 0xc0)
		{
		case	0x80:	cc = 0; break;
		case	0x40:	cc = 1; break;
		case	0xc0:	cc = 2; break;
		}
	
	cd = dat[senum * 5 + 1 + 5] & 0x3f;	/*@@@*/
	
	printf("\x1b[s SE:[%3d] ch:[%7s] priority[%3d] \x1b[u", senum, ch[cc], cd);
	
	while (kbhit() == 0);
	c = getch();
	
	if (c == 0x0b) 
		{
		senum--;
		if (senum < 0) senum = 0;
		}
	
	if (c == 0x0a)
		{
		senum++;
		if (senum >= semax) senum = semax-1;
		}
	
	if (c == ' ')
		{
		int sever;
		
		sever = sestart(dat, senum);
	
		if (sever == 0)
		{
			printf("SEファイルではありません\n");
			exit(1);
		}
		if (sever < opnnum)
		{
			printf("SEファイルのバージョンよりもOPNDRVのバージョンの方が古いため、演奏できません\n");
			exit(1);
		}
		
		}
	
	} while ((c != 'Q') && (c != 'q') && (c != 'ﾀ') && (c != 0x1b));
	
	mend();
	
	printf("\x1b[>5l");
	
	return 0;
}
