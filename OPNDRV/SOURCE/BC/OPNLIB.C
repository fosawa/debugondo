	/*OPNDRV Ver2.0
	
		C言語用ライブラリ
		(Borland C++ Ver3.0)
		
						Programmed by F.Osawa
	*/
	
	#include <stdio.h>
	#include <dos.h>
	#include <io.h>
	#include <stdlib.h>
	#include <string.h>
	#include <stdarg.h>
	
	#define VECT 0x41		/*ＯＰＮＤＲＶのベクタ*/
	#define L64 3


	/*共通部*/

int opncheck()
{
	/*OPNDRV Version の100倍の値を返します*/
	int i, ver;
	const char ID[]="VRDNPO";
	char far *vect;
	
	vect = (char far *)getvect(VECT);
	
	for (i = 0; i <= 5; i++)
	{
		vect--;
		if (vect[0] != ID[i]) return 0;		/*OPNDRVがない*/
	}
	
	ver = (vect[-1] - 0x30) + (vect[-2] - 0x30) * 10 + (vect[-4] - 0x30) * 100;
	
	return ver;
}

	/*FM音源部*/

void mstart (unsigned char far *addr)
{
	/*FM音源演奏スタート*/
	/*addr[0]～[15]は、演奏データを差すポインタが格納されている*/
	
	struct REGPACK rg;
	
	rg.r_ax=0;
	rg.r_bx=FP_OFF(addr);
	rg.r_es=FP_SEG(addr);

	intr (VECT,&rg);
}

int getdivision(unsigned char far *dat)
{
	/*OPNファイルのdivisionを返す*/
	/* 0で与えられたデータ列がOPN形式でないことを示す */
	
	int rcode;
	
	/*IDチェック*/
	if ((dat[0] != 'O') || (dat[1] != 'P') || (dat[2] != 'N'))
	{
		return 0;
	}
	
	/*divisionを得る*/
	rcode = dat[5] + (dat[6] << 8);
	
	return rcode;
}

int getopnver(unsigned char far *dat)
{
	/*OPNファイルのバージョンを返す*/
	/* 0で与えられたデータ列がOPN形式でないことを示す */
	/* そうでなければ、データのバージョンの100倍を返す*/
	int rcode;
	
	/*IDチェック*/
	if ((dat[0] != 'O') || (dat[1] != 'P') || (dat[2] != 'N'))
	{
		return 0;
	}
	
	/*バージョンを得る*/
	rcode = dat[3] * 100 + dat[4];
	
	return rcode;
}

int opnstart2(unsigned char far *dat, unsigned long length)
{
	struct REGPACK rg;
	
	/*IDチェック*/
	if ((dat[0] != 'O') || (dat[1] != 'P') || (dat[2] != 'N'))
	{
		return 1;
	}
	
	rg.r_ax = 18 * 256;
	rg.r_bx = FP_OFF(&dat[7]);
	rg.r_es = FP_SEG(&dat[7]);
	rg.r_dx = length >> 16L;
	rg.r_cx = length & 0xffff;

	intr(VECT,&rg);
	
	return rg.r_ax;
}

int opnstart(unsigned char far *dat)
{
	/*OPN形式ファイルの演奏をスタートさせます*/
	/* datには、OPN形式ファイルを読み込んだ先頭ポインタを入れてコールしてください*/
	/* 0で与えられたデータ列がOPN形式でないことを示す */
	/* そうでなければ、データのバージョンの100倍を返す*/
	
	
	struct REGPACK rg;
	int rcode;
	
	/*IDチェック*/
	if ((dat[0] != 'O') || (dat[1] != 'P') || (dat[2] != 'N'))
	{
		return 0;
	}
	
	/*バージョンを得る*/
	rcode = dat[3] * 100 + dat[4];
	
	rg.r_ax = 0;
	rg.r_bx = FP_OFF(&dat[7]);
	rg.r_es = FP_SEG(&dat[7]);

	intr(VECT,&rg);
	
	return rcode;
}

void mend()
{
	/*mstart関数で開始した演奏を終了する*/
	
	struct REGPACK rg;

	rg.r_ax = 0x100;
	intr(VECT,&rg);
	return;
}

void mfade(int fmspeed, int psgspeed)
{
	/*フェードアウトする*/
	/*fmspeed=FM音源およびRTH音源フェードアウトスピード*/
	/*psgspeed=PSG音源フェードアウトスピード*/
	
	struct REGPACK rg;
	
	rg.r_ax = 0x600;
	rg.r_bx = fmspeed * 256 + psgspeed;
	intr (VECT,&rg);

	return;
}

unsigned char mread (unsigned char ch, unsigned char workno)
{
	/*ワークエリアの内容を得る*/
	/*			ch=チャネル  (1-17(16,17は割り込みチャネル))
				workno=ワーク番号
	*/
	
	struct REGPACK rg;
	rg.r_ax = 0x800;
	rg.r_dx = workno*256+ch;
	intr (VECT, &rg);
	return (unsigned char)(rg.r_ax / 256);
}

int varread(int val)
{
	/*内部変数値を得る*/
	
	
	struct REGPACK rg;
	rg.r_ax = 50 * 256;
	rg.r_dx = val;
	intr (VECT, &rg);
	return rg.r_ax & 255;
}
	
void mescon()
{
	/*演奏中のFM音源部を一時停止します*/
	struct REGPACK rg;

	rg.r_ax = 0x300;
	intr(VECT, &rg);
}

void mescof()
{
	/*mescon関数で停止したものを再開します*/
	struct REGPACK rg;

	rg.r_ax = 0x400;
	intr(VECT, &rg);
}

void minton(unsigned char far *addr, unsigned char ch)
{
	/*割り込み演奏を開始します*/
	/*addr = 割り込みデータ先頭アドレス*/
	/*ch = チャネル*/
	struct REGPACK rg;

	rg.r_ax = 13 * 256;
	rg.r_dx = ch;
	rg.r_cx = FP_OFF(addr);
	rg.r_es = FP_SEG(addr);
	intr(VECT, &rg);

}

void sinton (unsigned char ch,unsigned char inum,...)
{
	/*エキスパンダありの割り込み設定*/
	/*ch=チャネル(1=PSG2CH, 2=PSG3CH, 3=PSG2CH&3CH)*/
	/*inum = 割り込み優先順位*/
	/*その後、データ先頭アドレスを差すポインタがつづく(可変長)*/
	
	unsigned char iaddr[5];
	va_list pt;
	unsigned int dummy;
        struct REGPACK rg;

	va_start(pt,inum);

	switch (ch)
	{
		case 1:		/*psg 2ch only*/
				iaddr[0]=inum | 0x80;
				dummy=(unsigned int)va_arg(pt,unsigned char *);
				iaddr[1]=dummy % 256;
				iaddr[2]=dummy / 256;
				break;
		case 2:		/*psg 3ch only*/
				iaddr[0]=inum | 0x40;
				dummy=(unsigned int)va_arg(pt,unsigned char *);
				iaddr[1]=dummy % 256;
				iaddr[2]=dummy / 256;
				break;
		case 3:		/*psg 2ch and 3ch */
				iaddr[0]=inum | 0xc0;
				dummy=(unsigned int)va_arg(pt,unsigned char *);
				iaddr[1]=dummy % 256;
				iaddr[2]=dummy / 256;
				dummy=(unsigned int)va_arg(pt,unsigned char *);
				iaddr[3]=dummy % 256;
				iaddr[4]=dummy / 256;
				break;
	}
	va_end(pt);

	rg.r_ax=0xb00;
	rg.r_bx=(unsigned int)(&iaddr[0]);
	intr (VECT,&rg);
}

void sstart ()
{
	/*エキスパンダon*/
	struct REGPACK rg;

	rg.r_ax=0x900;
	intr (VECT,&rg);
}

void send()
{
	/*エキスパンダoff*/
	struct REGPACK rg;

	rg.r_ax=0xa00;
	intr (VECT,&rg);

}

void tempochange(int tempo, int division)
{
	/*FM音源演奏中のテンポを変更する*/
	/*ただし、元のデータ内でテンポ設定コマンドにぶつかると、設定は無効になる*/
	
	struct REGPACK rg;
	unsigned int tm;
	
	tm = 256-208200/(division*tempo);
	
	rg.r_ax = 15 * 256;
	rg.r_bx = tm;
	
	intr (VECT,&rg);
}

int regread(int regno, int sel)
{
	/*FM音源に送ったレジスタの内容を調べる*/
	/*regno = レジスタ番号*/
	/*sel = レジスタセレクタ(0 = YM2203互換部分、 1 = YM2608拡張部分)*/
	struct REGPACK rg;
	
	rg.r_ax = 16 * 256;
	rg.r_dx = sel * 256 + regno;
	
	intr (VECT,&rg);

	return rg.r_ax & 0xff;
}
	
	/*SE部*/
	
int getsemax(unsigned char far *sdata)
{
	/*SEファイルかどうか調べる*/
	/*SEファイルならば格納されているSEナンバー最大値を、そうでなければ0を返す*/
	
	int rcode;
	
	if ((sdata[0] != 'S') || (sdata[1] != 'E') || (sdata[2] != ' '))
	{
		return 0;
	}
	
	return sdata[5];
}

int getsever(unsigned char far *sdata)
{
	/*SEファイルのバージョンを返す*/
	/*SEデータでなければ0を、そうでなければデータバージョンの100倍の値を返す*/
	
	int rcode;
	
	/*IDチェック*/
	if ((sdata[0] != 'S') || (sdata[1] != 'E') || (sdata[2] != ' '))
	{
		return 0;
	}
	
	/*バージョンを得る*/
	rcode = sdata[3] * 100 + sdata[4];
	
	
	return rcode;
}

void sestart2(int num)
{
	/*ストアしたSEを鳴らす*/
	struct REGPACK rg;
	
	rg.r_ax = 2 * 256;
	rg.r_bx = num;
	
	intr (VECT,&rg);
	
}

int sestore(unsigned char far *sdata, unsigned long length)
{
	struct REGPACK rg;
	
	/*SEを常駐させる*/
	
	/*IDチェック*/
	if ((sdata[0] != 'S') || (sdata[1] != 'E') || (sdata[2] != ' '))
	{
		return 0;
	}
	
	rg.r_ax = 19 * 256;
	rg.r_bx = FP_OFF(sdata);
	rg.r_es = FP_SEG(sdata);
	rg.r_dx = length >> 16L;
	rg.r_cx = length & 0xffff;
	
	intr (VECT,&rg);
	
	if (rg.r_ax == 0xffff)
	{
		return -1;
	}
	
	return (sdata[3] * 100 + sdata[4]);
}

int sestart(unsigned char far *sdata, int snum)
{
	/*効果音を鳴らす*/
	/*FM音源部が演奏中であることが必要*/
	/*sdata = SEファイルを読み込んだ先頭ポインタ, snum = 出したいSE番号*/
	/*SEデータでなければ0を、そうでなければデータバージョンの100倍の値を返す*/
	/*snumが多きすぎれば-1を返す*/
	
	struct REGPACK rg;
	unsigned int offset;
	unsigned int addr;
	unsigned char bf[4];
	int i;
	int rcode;
	
	offset = FP_OFF(sdata);
	
	
	/*IDチェック*/
	if ((sdata[0] != 'S') || (sdata[1] != 'E') || (sdata[2] != ' '))
	{
		return 0;
	}
	
	/*バージョンを得る*/
	rcode = sdata[3] * 100 + sdata[4];
	
	if (sdata[5] <= snum)
	{
		return -1;
	}
	
	/*オフセット補正*/
	addr = sdata[snum * 5 + 2 + 5] + sdata[snum * 5 + 3 + 5] * 256;
	addr += offset;
	
	bf[0] = sdata[snum * 5 + 2 + 5];
	bf[1] = sdata[snum * 5 + 3 + 5];
	
	sdata[snum * 5 + 2 + 5] = addr & 0xff;
	sdata[snum * 5 + 3 + 5] = addr >> 8;
	
	addr = sdata[snum * 5 + 4 + 5] + sdata[snum * 5 + 5 + 5] * 256;
	addr += offset;
	
	bf[2] = sdata[snum * 5 + 4 + 5];
	bf[3] = sdata[snum * 5 + 5 + 5];
	
	sdata[snum * 5 + 4 + 5] = addr & 0xff;
	sdata[snum * 5 + 5 + 5] = addr >> 8;
	
	rg.r_ax = 0x0e00;
	rg.r_bx = FP_OFF(&sdata[snum * 5 + 1 + 5]);
	rg.r_es = FP_SEG(&sdata[snum * 5 + 1 + 5]);
	
	intr (VECT,&rg);
	
	/*オフセット補正を戻す*/
	
	for (i = 0; i < 4; i++)
	{
		sdata[snum * 5 + i + 2 + 5] = bf[i];
	}

	return rcode;
}
	
	/*MIDI部*/
	
int midiinit (int interface)
{
	/*MIDIインターフェースの初期化*/
	/*interfase = 1:MPU-401, 2=RS-232C*/
	/*戻り値: 0:正常終了, 0以外:そのインターフェースは使えない*/
	/*ただし、RS-232Cインターフェースを指定した時には、常に0の正常終了が戻ることに注意*/
	
	struct REGPACK rg;

	rg.r_ax = 20 * 256;
	rg.r_dx = interface;
	
	intr (VECT, &rg);

	return rg.r_ax;
}

int midistart (void huge *addr)
{
	/*MIDI演奏をスタートさせる*/
	/*addr = 標準MIDIファイルデータの入っている先頭ポインタ*/
	/*戻り値: 0=正常終了、0以外=標準MIDIファイルでない*/
	
	struct REGPACK rg;
	
	rg.r_ax = 22 * 256;
	rg.r_bx = FP_OFF(addr);
	rg.r_es = FP_SEG(addr);
	
	intr (VECT, &rg);
	
	return rg.r_ax;
}

int midistart2 (void huge *addr, unsigned long length)
{
	/*MIDI演奏をスタートさせる*/
	/*addr = 標準MIDIファイルデータの入っている先頭ポインタ*/
	/*戻り値: 0=正常終了、1=標準MIDIファイルでない*/
	/*        -1=バッファが足りない*/
	
	struct REGPACK rg;
	
	rg.r_ax = 31 * 256;
	rg.r_bx = FP_OFF(addr);
	rg.r_es = FP_SEG(addr);
	rg.r_dx = length >> 16L;
	rg.r_cx = length & 0xffff;
	
	intr (VECT, &rg);
	
	return rg.r_ax;
}

void midistop(void)
{
	/*midistartで開始させたMIDI演奏を止める(一時停止)*/
	struct REGPACK rg;
	
	rg.r_ax = 23*256;
	intr (VECT, &rg);
	return;
}

void midicont(void)
{
	/*midistopで停止させた演奏を再度演奏開始にする*/
	struct REGPACK rg;
	
	rg.r_ax = 24 * 256;
	intr (VECT, &rg);
	return;
}

void miditempo(int tempo)
{
	/*MIDIテンポ値補正の設定*/
	/*テンポを標準テンポのtempo/256.0倍にする*/
	
	struct REGPACK rg;
	
	rg.r_ax = 27 * 256;
	rg.r_dx = tempo;
	
	intr (VECT, &rg);
	
}

void midiclose(void)
{
	/*MIDI機能をストップさせる*/
	struct REGPACK rg;
	
	rg.r_ax = 21 * 256;
	intr (VECT, &rg);
	
}

void midiext(void)
{
	/*MIDI拡張メタイベント許可*/
	/*このファンクションを呼んだあと、
	
		ff, 07, 04, 'M', 'a', 'r', 'k'がデータ中に出てきたらその位置をマーク
		ff, 07, 04, 'J', 'u', 'm', 'p'でそのマークした位置にジャンプ
		
	　という、標準MIDIファイルを無視したイベントが許可される
	*/
	
	struct REGPACK rg;
	
	rg.r_ax = 29 * 256;
	intr (VECT, &rg);
	
}

void midisend(unsigned char data)
{
	/* MIDIに1バイトデータを送る */
	struct REGPACK rg;
	
	rg.r_ax = 30*256;
	rg.r_dx = data;
	intr (VECT, &rg);
}

int getmidi(void)
{
	/*MIDIステータスを見る*/
	/*戻り値:
		下位8bit : 0=演奏中、1=待機中(=停止コマンドで待機中)、2=演奏終了(=データをすべて送り終わった)
		上位8bit : ベロシティー補正値
	*/
	
	struct REGPACK rg;
	
	rg.r_ax = 26 * 256;
	intr (VECT, &rg);
	return rg.r_ax;
}

void midifade(unsigned char start, unsigned char end, unsigned int speed)
{
	/*MIDI フェードアウト*/
	/*start = はじめのベロシティー値
	  end = 終わりのベロシティー値
	  speed = 変化速度(単位はgatetime)
	  
	  ファンクション呼び出し後、
	  音量vが、v×start/128倍→v×end/128倍に、speedの時間内で直線的に変化する
	*/
	
	struct REGPACK rg;
	
	rg.r_ax = 25*256;
	rg.r_cx = speed;
	rg.r_dx = end * 256 + start;
	intr (VECT, &rg);
	
}

	/*PCM音源部*/
	
int getpcmver(unsigned char far *dat)
{
	/*PCMデータのバージョンを返す*/
	/*非PCM形式で0をそうでなければ、PCMデータのバージョンを返す*/
	int rcode;
	
	/*IDチェック*/
	if ((dat[0] != 'P') || (dat[1] != 'C') || (dat[2] != 'M'))
	{
		return 0;
	}
	
	
	/*バージョンを得る*/
	rcode = dat[3] * 100 + dat[4];
	
	return rcode;
}
	
int pcmplay(unsigned char huge *dat)
{
	/*PCMデータを鳴らす*/
	/*dat = PCMファイル形式データの入っている先頭ポインタ*/
	/*非PCM形式で0をそうでなければ、PCMデータのバージョンを返す*/
	
	struct REGPACK rg;
	int rcode;
	
	/*IDチェック*/
	if ((dat[0] != 'P') || (dat[1] != 'C') || (dat[2] != 'M'))
	{
		return 0;
	}
	
	
	/*バージョンを得る*/
	rcode = dat[3] * 100 + dat[4];
	
	
	rg.r_ax = 40 * 256;
	rg.r_bx = FP_OFF(&dat[5]);
	
	rg.r_es = FP_SEG(&dat[5]);

	intr (VECT,&rg);

	return rcode;
}

int pcmplay2(unsigned char huge *dat, unsigned long length)
{
	/*PCMデータを鳴らす*/
	/*dat = PCMファイル形式データの入っている先頭ポインタ*/
	/*非PCM形式で-2をそうでなければ、PCMデータのバージョンを返す*/
	/*-1でバッファが足りないことを示す*/
	
	struct REGPACK rg;
	int rcode;
	
	/*IDチェック*/
	if ((dat[0] != 'P') || (dat[1] != 'C') || (dat[2] != 'M'))
	{
		return -2;
	}
	
	
	/*バージョンを得る*/
	rcode = dat[3] * 100 + dat[4];
	
	
	rg.r_ax = 42 * 256;
	rg.r_bx = FP_OFF(&dat[5]);
	rg.r_es = FP_SEG(&dat[5]);
	rg.r_dx = length >> 16L;
	rg.r_cx = length & 0xffff;

	intr (VECT,&rg);
	
	if (rg.r_ax != 0)
	{
		return rg.r_ax;
	}

	return rcode;
}

void pcmstop()
{
	/*演奏中のPCM音源を止める*/
	
	struct REGPACK rg;
	
	rg.r_ax = 41 * 256;
	
	intr (VECT,&rg);
	
}

int	getstatus()
{
	/*演奏ステータスを得る*/
	/*
		戻り値:
		
		ビット0: FM音源再生 (0:再生中,1:停止中)
		ビット1: PCM音源再生(0:再生中,1:停止中)
		ビット4: ポート番号が(0:188h, 1:288h)
		ビット5: PCM音源が(0:存在しない,1:存在する)
		ビット6: YM2608が(0:存在しない,1:存在する)
		ビット7: FM音源ボードが(0:存在しない,1:存在する)
	*/
	
	struct REGPACK rg;
	
	rg.r_ax = 17 * 256;
	
	intr (VECT,&rg);
	
	return(rg.r_ax & 0xff);
}

