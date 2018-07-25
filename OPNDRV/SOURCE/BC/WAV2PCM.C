	/*
	Wave fileをPCM fileに変換するプログラム
	
								1994/04/27
											F.Osawa
	*/
	
	#define pcmver 200
	
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<alloc.h>
#include	<signal.h>
#include	<dos.h>
#include	<dir.h>
#include	<io.h>
#include	<mem.h>
#include	<jstring.h>


#define getdword(fp) ((unsigned long)fgetc(fp)+((unsigned long)fgetc(fp)<<8L)+((unsigned long)fgetc(fp)<<16L)+((unsigned long)fgetc(fp)<<24L))
#define getword(fp) (fgetc(fp)+(fgetc(fp)<<8))

void putdword(long int m, FILE *fp)
{
	fputc((char)(m & 255L), fp);
	m = m >> 8;
	fputc((char)(m & 255L), fp);
	m = m >> 8;
	fputc((char)(m & 255L), fp);
	m = m >> 8;
	fputc((char)(m & 255L), fp);
}

void putword(int m, FILE *fp)
{
	fputc(m >> 8, fp);
	fputc(m & 255, fp);
}


unsigned int shz = 0;
int play_type = 0;
int info_flag = 0;
int repeat_flag = 0;
int volume = 1;

volatile int stop_flag = 0;
volatile int skip_flag = 0;

int wavetrans(char *filename, char *filename2)
{
	/*waveファイルを解析し、変換するメインルーチン*/
	FILE *fp, *fp2;
	char typechk[5];	/*ファイルチェック文字列領域*/
	unsigned long fmtoffset;	/*次のフォームタイプへのオフセット*/
	unsigned long idoffset;	/*次のIDまでのオフセット*/
	unsigned int filetype;	/*ファイルタイプ*/
	unsigned int ch;	/*チャンネル数*/
	unsigned long hz, p_hz;	/*サンプリング周波数*/
	unsigned long mvbyte;	/*転送バイト数*/
	unsigned int smpsize;	/*サンプルサイズ(=1サンプル当たりのバイト数)*/
	unsigned int pcmbit;	/*量子化ビット数*/
	unsigned long pcmsize, p_pcmsize;	/*pcmデータサイズ*/
	unsigned huge char *pcmdata;	/*pcmデータの格納ポインタ*/
	unsigned long u;
	unsigned int pcm;
	unsigned int dummy;
	unsigned long rest_size;
	int pcmtype;
	
	int all_flag = 0;
	int data;
	fpos_t file_pos;
	unsigned long leftsize;
	int over_samp = 0;
	
	char	fmt_flag = 0, data_flag = 0;
	char buffer[8192];
	
	if ((fp = fopen(filename, "rb")) == NULL)
		{
		fprintf(stderr, " [%s] - not found\n", filename);
		return 1;
		}
	
	setvbuf(fp, buffer, _IOFBF, 8192);
	
	/*ヘッダ「RIFF」のチェック*/
	fgets(typechk, 5, fp);
	if (strcmp(typechk, "RIFF") != 0)
		{
		fprintf(stderr, " [%s] - not RIFF file\n", filename);
		return 1;
		}
	
	/*次のフォームタイプまでのオフセット*/
	fmtoffset = getdword(fp);
	
	/*フォームタイプのチェック*/
	fgets(typechk, 5, fp);
	if (strcmp(typechk, "WAVE") != 0)
		{
		fprintf(stderr, " [%s] - not WAVE file\n", filename);
		return 1;
		}
	
	/*ID「fmt 」のチェック*/
	/*IDのチェック*/
	
	do
	{
	if (fgets(typechk, 5, fp) == NULL) break;
	
	idoffset = getdword(fp);
	idoffset++;
	idoffset &= 0xfffffffe;
	
	if ((strcmp(typechk, "fmt ") == 0) && (fmt_flag == 0))
		{
		/*fmtチャンク*/
		
		/*PCMタイプの取得*/
		filetype = getword(fp);
		if (filetype != 1)
			{
			fprintf(stderr, " [%s] - not PCM file(packed file?) \n", filename);
			return 1;
			}
		
		/*チャンネル数の取得*/
		ch = getword(fp);
	
		/*サンプリング周波数の取得*/
		p_hz = hz = getdword(fp);
		if (shz != 0) hz = shz;
	
		/*転送バイト数の取得*/
		mvbyte = getdword(fp);
	
		/*サンプルサイズの取得*/
		smpsize = getword(fp);
	
		/*量子化ビット数の取得*/
		pcmbit = getword(fp);
		
		/*ゴミの削除*/
		for (u = 0; u < idoffset - 16; u++)
			{
			fgetc(fp);
			}
		
		fmt_flag = 1;
		continue;
		}
	
	if ((strcmp(typechk, "data") == 0) && (data_flag == 0))
		{
		/*data チャンク*/
		if (fmt_flag == 0) 
			{
			fprintf(stderr, " [%s] - not found \"fmt \" CK.\n", filename);
			return 1;
			}
		
		/*ファイル2をオープン*/
		if ((fp2 = fopen(filename2, "rb")) != NULL)
		{
      		int i;
      		fprintf (stderr, "[%s] Overwrite [y / else] ? ", filename2);
			i = getchar();
      		if (!((i == 'Y') || (i == 'y') || (i == 'ﾝ')))
			{
	  		fprintf (stderr, "... not converted");
	  		fclose(fp);
	  		fclose(fp2);
	  		return 1;
			}
		}
		
		if ((fp2 = fopen(filename2, "wb")) == NULL)
		{
		fputs("Cannot write data (Disk full ?)", stderr);
		}
		
		/*PCM ID書き込み*/
		fputs("PCM", fp2);
		
		/*バージョン整数部*/
		fputc(pcmver / 100, fp2);
		
		/*バージョン小数部*/
		fputc(pcmver % 100, fp2);
		
		
		/*PCMモードの書き込み*/
		if (pcmbit <= 8)
		{
			/*8bit*/
			pcmtype = 0x70;
		}
		else
		{
			/*16bit*/
			pcmtype = 0x30;
		}
		
		/*PCMレート*/
		switch (p_hz)
		{
			case 44100:
							pcmtype |= 0;
							break;
			case 33080:
							pcmtype |= 1;
							break;
			case 22050:
							pcmtype |= 2;
							break;
							
			case 16540:
							pcmtype |= 3;
							break;
							
			case 11030:
			case 11025:
							pcmtype |= 4;
							break;
							
			case 8270:
							pcmtype |= 5;
							break;
							
			case 5520:
							pcmtype |= 6;
							break;
							
			case 4130:
							pcmtype |= 7;
							break;
							
			default:
							printf("この周波数はPC9801-86ボードでの再生はできません\n");
							exit(1);
							
		}
		
		fputc(pcmtype, fp2);
		
		/*データサイズ*/
		
		if (ch == 1)
		{
			putdword(idoffset * 2UL, fp2);
		}
		else
		{
			putdword(idoffset, fp2);
		}
		
		if (pcmbit > 8)
		{
			idoffset /= 2UL;
		}
		
		for (rest_size = 0; rest_size < idoffset; rest_size++)
		{
			unsigned char a;
			unsigned int b;
			
			if (ch == 1)
			{
				/*モノラルデータ*/
				if (pcmbit <= 8)
				{
				/*データ8bit*/
				a = fgetc(fp);
				a += 0x80;
				fputc(a, fp2);
				fputc(a, fp2);
				}
				else
				{
				/*データ16bit*/
				b = getword(fp);
				putword(b, fp2);
				putword(b, fp2);
				}
			}
			else
			{
				/*ステレオデータ*/
				if (pcmbit <= 8)
				{
				/*データ8bit*/
				a = fgetc(fp);
				a += 0x80;
				fputc(a, fp2);
				}
				else
				{
				/*データ16bit*/
				b = getword(fp);
				putword(b, fp2);
				}
			}
		}
		}
		
	} while (feof(fp) == 0);
	
	fclose(fp);
	fclose(fp2);
  	fprintf (stderr, "...Converted\n");
	
	return 0;
}

int info(char *filename)
{
	/*waveファイルの詳細を表示する*/
	const char *fftype[] = {" mono ", "stereo"};
	
	FILE *fp;
	char typechk[5];	/*ファイルチェック文字列領域*/
	unsigned long fmtoffset;	/*次のフォームタイプへのオフセット*/
	unsigned long idoffset;	/*次のIDまでのオフセット*/
	unsigned int filetype;	/*ファイルタイプ*/
	unsigned int ch;	/*チャンネル数*/
	unsigned long hz, p_hz;	/*サンプリング周波数*/
	unsigned long mvbyte;	/*転送バイト数*/
	unsigned int smpsize;	/*サンプルサイズ(=1サンプル当たりのバイト数)*/
	unsigned int pcmbit;	/*量子化ビット数*/
	unsigned long pcmsize, p_pcmsize;	/*pcmデータサイズ*/
	unsigned huge char *pcmdata;	/*pcmデータの格納ポインタ*/
	unsigned long u;
	unsigned int pcm;
	unsigned int dummy;
	char	fmt_flag = 0, data_flag = 0;
	
	if ((fp = fopen(filename, "rb")) == NULL)
		{
		fprintf(stderr, " [%s] - not found\n", filename);
		return 1;
		}
	
	/*ヘッダ「RIFF」のチェック*/
	fgets(typechk, 5, fp);
	if (strcmp(typechk, "RIFF") != 0)
		{
		fprintf(stderr, " [%s] - not RIFF file\n", filename);
		return 1;
		}
	
	/*次のフォームタイプまでのオフセット*/
	fmtoffset = getdword(fp);
	
	/*フォームタイプのチェック*/
	fgets(typechk, 5, fp);
	if (strcmp(typechk, "WAVE") != 0)
		{
		fprintf(stderr, " [%s] - not WAVE file\n", filename);
		return 1;
		}
	
	printf("\n\nInfomation [%s]:\n", filename);
	printf("RIFF: WAVE \n");
	
	/*ID「fmt 」のチェック*/
	/*IDのチェック*/
	
	
	do
	{
	if (fgets(typechk, 5, fp) == NULL) break;
	
	/*次のIDまでのオフセット値*/
	idoffset = getdword(fp);
	
	printf("\n  ID  :    CK.size  \n");
	printf("[%4s]: %10lu \n", typechk, idoffset);
	
	idoffset++;
	idoffset &= 0xfffffffe;
	
	if (strcmp(typechk, "fmt ") == 0)
		{
		/*fmtチャンク*/
		
		/*PCMタイプの取得*/
		filetype = getword(fp);
		
		/*チャンネル数の取得*/
		ch = getword(fp);
	
		/*サンプリング周波数の取得*/
		p_hz = hz = getdword(fp);
		if (shz != 0) hz = shz;
	
		/*転送バイト数の取得*/
		mvbyte = getdword(fp);
	
		/*サンプルサイズの取得*/
		smpsize = getword(fp);
	
		/*量子化ビット数の取得*/
		pcmbit = getword(fp);
		
		/*ゴミの削除*/
		for (u = 0; u < idoffset - 16; u++)
			{
			fgetc(fp);
			}
		
		printf("FormatTag:     %10u    Channels:      %10u[%s] \n", filetype, ch, fftype[ch - 1]);
		printf("SamplesPerSec: %10lu    AvgBytePerSec: %10lu \n", p_hz, mvbyte);
		printf("BlockAlign:    %10u    BitsPerSample: %10u \n", smpsize, pcmbit);
		
		fmt_flag = 1;
		
		continue;
		}
	
	fseek(fp, idoffset, SEEK_CUR);
	} while (feof(fp) == 0);
	
	return 0;
}

int main(int argv, char **argc)
{
	char filename[FILENAME_MAX];
	char filename2[FILENAME_MAX];
	int fileflag = 0;
	int rcode;
	int i;
	
	if (argv <= 1)
		{
		msg:
		
		puts ("wav2pcm  Ver 1.00 : WAVE形式ファイルをPCM形式に変換します");
		puts ("                                Programmed by F.Osawa");
		puts ("Usage:");
		puts ("     wav2pcm [Options] wavefilename [pcmfilename]");
		puts ("Options:");
		puts ("      /i:   ファイルの詳細表示をします");
		return 1;
		}
	
	/*オプションチェック*/
	
	for (i = 1; i < argv; i++)
		{
		fileflag = 0;
		stop_flag = 0;
		skip_flag = 0;
		
		if ((*(argc[i]) != '/') && (*(argc[i]) != '-'))
		{
			switch (fileflag)
			{
				case 0:
						/*ファイル名(WAVE)*/
						strcpy(filename, argc[i]);
						fileflag++;
						break;
				case 1:
						/*ファイル名(PCM)*/
						strcpy(filename2, argc[i]);
						fileflag++;
						break;
				default:
						/*ファイル名が多すぎる*/
						printf("指定ファイル名が多すぎます\n");
						return 1;
			}
		
		}
		else
		{
			strlwr(argc[i]); /*大文字->小文字*/
			argc[i]++; /* /と-を飛ばす */
			
			if (strstr(argc[i], "i-") == argc[i])
				{
				info_flag = 0;
				continue;
				}
			
			if (strstr(argc[i], "i+") == argc[i])
				{
				info_flag = 1;
				continue;
				}
			
			if (strstr(argc[i], "i") == argc[i])
				{
				info_flag = 1;
				continue;
				}
			
			if (strstr(argc[i], "h") == argc[i])
				goto msg;
			if (strstr(argc[i], "?") == argc[i])
				goto msg;
			
			err:
			
			fprintf(stderr, "Incorrect option -[%s]", argc[i] - 1);
			return 1;
		}
	}
	
	if (fileflag == 0)
	{
		/*ファイル名が足りない*/
		goto msg;
	}
		
	if (fileflag == 1) 
	{
		char *s;
		/*ファイル名が一つだけの時には、source,destination共に同じファイル名にする*/
		if ((s = strpbrk(filename, ":\\")) == NULL)
		{
			strcpy(filename2, filename);
		}
		else
		{
			strcpy(filename2, &s[1]);
		}
		
	if ((s = strrchr(filename2, '.')) != NULL)
		{
			s[0] = '\0';
		}
	}
		
	/*destinationがドライブ名のみの場合には、ファイル名を加える*/
	if (filename2[strlen(filename2) - 1] == ':')
	{
		char *s;
		if ((s = strpbrk(filename, ":\\")) == NULL)
		{
			strcat(filename2, filename);
		}
		else
		{
			strcat(filename2, &s[1]);
		}
	}

	/*destinationが'\'で終わっている場合には、ファイル名を加える*/
	if (filename2[strlen(filename2) - 1] == '\\')
	{
		char *s;
		if ((s = strpbrk(filename, ":\\")) == NULL)
		{
			strcat(filename2, filename);
		}
		else
		{
			strcat(filename2, &s[1]);
		}
	}
	
	/*拡張子がなければwav,pcmにする*/
	
	if (strchr(filename, '.') == NULL)
		strcat(filename, ".wav");
	
	if (strchr(filename2, '.') == NULL)
		strcat(filename2, ".pcm");
	
	wavetrans(filename, filename2);
	
	return 0;
}
