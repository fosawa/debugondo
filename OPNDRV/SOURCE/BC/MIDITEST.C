	/*
	OPNDRV MIDI+
	
	  で、標準MIDIファイルを演奏させるサンプル
	  
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

	/*OPNDRVのチェック*/
	drv = opncheck();
	if (drv == 0) 
		{printf ("OPNDRVが組み込まれていません\n"); return(1);}
		
	if (drv < 200)
		{printf ("OPNDRVのバージョンが古すぎます\n");
		 return(1);
		 }
	

	if (argv <= 1)
		{
		msg:
		
		puts ("MIDITEST  Ver 1.00 : 標準MIDI形式ファイルを演奏します");
		puts ("                                    Programmed by F.Osawa");
		puts ("Usage:");
		puts ("     MIDITEST [/m] [/r] MIDIfilename");
		puts ("Options:");
		puts ("     /m MPU-401インターフェースを使用(Default)");
		puts ("     /r RS-232Cインターフェースを使用");
		puts ("     /z MIDIデータを常駐演奏させる");
		puts ("     /e zオプションで常駐演奏中のMIDIを停止する");
		puts ("     /c eオプションで止めたものを再開する");
		return(1);
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
		printf("ファイル名がありません\n");
		return 1;
	}
	
	if (strchr(filename, '.') == NULL)
		{
		strcat(filename, ".MID");
		}

	
	if ((fp = fopen(filename, "rb")) == NULL)
		{
		printf ("ファイル[%s]が見つかりません\n", filename);
		return(1);
		}
	
	/*ファイルの長さを取得*/
	
	/*その分のメモリを確保*/
	
	fsize = filelength(fileno(fp));

	if ((dat = farmalloc(fsize)) == NULL)
		{
		printf ("メモリが確保できませんでした\n");
		fclose(fp);
		return(1);
		}
	
	/*ロード*/
	/*本当は、fread関数を使いたいのだけど、farだからねぇ*/
	
	
	for (i = 0; i < fsize; i++)
		{
		dat[i] = fgetc(fp);
		}
	
	
	fclose(fp);
	
	/*イニシャライズ*/
	
	if (midiinit(intf) != 0) 
	{
		char *intfstr[] ={"MPU-401", "RS-232C"};
		printf("指定したMIDIインターフェース[%s]は使用できません\n", intfstr[intf - 1]);
		return 1;
	}
	
	/*演奏開始*/
	midiext();
	
	/*リセットオールコントローラーを送る*/
	
	for (i = 0; i < 16; i++)
		{
		midisend(0xb0 + i);
		midisend(0x79);
		midisend(0x00);
		}
	
	if (optflag == 1)
	{
		/*データ常駐*/
		midistop();
		rc = midistart2(dat, fsize);
		
		farfree(dat);
		
		if (rc == -1)
		{
			printf("OPNDRVのMIDIバッファが足りません\n");
			printf("%dKB以上のバッファを確保してください\n", (fsize + 1024) / 1024);
			midiclose();
			return 1;
		}
		
		if (rc != 0)
		{
			printf("ファイルがMIDIファイルとはみなせません\n");
			midiclose();
			return 1;
		}
		
		return 0;
	}
	
	if ((rc = midistart(dat))!= 0) 
		{
		printf ("ファイルがMIDIファイルとはみなせません\n");
		midiclose();
		farfree(dat);
		return(1);
		}
	
	printf ("演奏中!\n");
	printf ("キー操作:\n");
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
		printf ("演奏終了\n");
		goto	end;
		}
	}

	key = getch();
	if ((key == 'd') || (key == 'D'))
		{
		/*テンポダウン*/
		tempo++;
		if (tempo > 512) tempo = 512;
		printf ("tempo = [%f] 倍\n", tempo / 256.0);
		miditempo(tempo);
		}
		
	if ((key == 'u') || (key == 'U'))
		{
		/*テンポアップ*/
		tempo--;
		if (tempo < 128) tempo = 128;
		printf ("tempo = [%f] 倍\n", tempo / 256.0);
		miditempo(tempo);
		}
	
	if ((key == 'p') || (key == 'P'))
		{
		/*一時停止*/
		printf ("[pause]\n");
		midistop();
		}
	
	if ((key == 'c') || (key == 'C'))
		{
		/*再開*/
		printf ("[continue]\n");
		midicont();
		}
	
	} while ((key != 'e') && (key != 'E'));
	
end:	
	/*演奏終了*/
	
	midistop();
	
	/*MIDI使用終了*/
	
	midiclose();
	
	
	farfree(dat);
	
	return(0);
}
