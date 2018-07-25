	/*
	デバッグ音頭歌詞表示用プログラム
	
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

	unsigned char karaoke[]="今\0宵\0も\0ど\0こ\0か\0ら\0現\0れ\0る  \0"
							"お\0ら\0の\0回\0り\0に\0現\0れ\0る\n\0"
							"小\0さ\0な\0小\0さ\0な\0虫\0達\0が  \0"
							"い\0る\0の\0は\0わ\0か\0る\0が\0見\0つ\0か\0ら\0ない\n\0"
							"今\0日\0も\0明\0日\0も\0寝\0ら\0れ\0な\0い\n\0"
							
  							"バ\0グ\0を\0探\0し\0て\0一\0週\0間\n\0"
  							"バ\0グ\0を\0潰\0し\0て\0二\0週\0間\n\0"
  							"風\0呂\0に\0も\0入\0れ\0ず\0三\0週\0間\n\0"
  							"そ\0ん\0な\0こ\0ん\0な\0で\0一\0ヶ\0月\n\0"
  							
  							"窓\0の\0外\0に\0は\0日\0が\0昇\0り  \0"
  							"今\0朝\0の\0朝\0食\0ス\0パ\0ゲ\0ティ\0ー\n\0"
  							"か\0ら\0ん\0で\0な\0か\0な\0か\0巻\0き\0取\0れ\0ない  \0"
  							"そ\0ろ\0そ\0ろ\0寝\0よ\0う\0か\0い\0や\0だ\0め\0だ\n\0"
							"今\0日\0も\0明\0日\0も\0寝\0ら\0れ\0な\0い\n\0"
							
  							"バ\0グ\0を\0探\0し\0て\0一\0週\0間\n\0"
  							"バ\0グ\0を\0潰\0し\0て\0二\0週\0間\n\0"
  							"風\0呂\0に\0も\0入\0れ\0ず\0三\0週\0間\n\0"
  							"そ\0ん\0な\0こ\0ん\0な\0で\0一\0ヶ\0月\n\0"
  							
  							"バ\0グ\0バ\0グ\0バ\0グ\0バ\0グ\0バ\0グ\0バイ\0バイ\n\0"
  							"バ\0グ\0バ\0グ\0バ\0グ\0バ\0グ\0バ\0グ\0バイ\0バイ\n\0"
  							"ど\0う\0か\0お\0ね\0が\0い\0消\0え\0て\0く\0れ\n\0"
  							
  							"バ\0グ\0バ\0グ\0バ\0グ\0バ\0グ\0バ\0グ\0バイ\0バイ\n\0"
  							"バ\0グ\0バ\0グ\0バ\0グ\0バ\0グ\0バ\0グ\0バイ\0バイ\n\0"
  							"ど\0う\0か\0お\0ね\0が\0い\0消\0え\0て\0く\0れ\0"
  							
							"*";
	
	
	opnnum = opncheck();
	if (opnnum == 0)
	{
		printf("OPNDRVが登録されていません\n");
		exit(1);
	}
	
	/*ファイルロード*/
	if ((fp = fopen("ondo.opn", "rb")) == NULL)
	{
		printf("ondo.opnが見つかりません\n");
		return 1;
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
	
	
	printf ("\x1b[>5h\x1b[2Jデバッグ音頭\n");
	printf ("                                         作詞・作曲：大澤文孝\n");
	printf ("                                         声：伊藤夏世\n");
	printf ("\n",0);
	
	/*歌詞を緑で表示*/
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
		printf("OPNファイルではありません\n");
		return 1;
	}
	
	if (dataver < opnnum)
	{
		printf("OPNDRVのバージョンがデータのバージョンよりも古いため演奏できません\n");
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