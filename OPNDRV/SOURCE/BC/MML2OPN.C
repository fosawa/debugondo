	/*
	MML -> OPN
	
				コンバータ
				
				FM6音,SSG3音対応版
				
									Programmed by F.Osawa
	*/
extern unsigned _stklen = 8192;		/* 4KB では不足 */

	#define	opnver 200
	
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <jstring.h>
	
	#define DET 0
	int L64= 3;
	int L4=	48;
	
	int line = 0;
	unsigned int optbuf[15];
	
void putword(unsigned int m, FILE *fp)
{
	fputc(m & 255, fp);
	m = m >> 8;
	fputc(m & 255, fp);
}
	
void puterrno(char *filename)
{
	/*ファイルネームと、行番号を表示する*/
	printf ("\n%s %d:", filename, line);
	return;
}

int gettok(unsigned char *tk, FILE *fp)
{
	/*ファイルから文字を読みだし、トークンを切り、tkに代入する*/
	int pt = 0;
	int moji;
	
	/*空白、TAB、\nの読み捨て*/
	
	retry:
	
	do
	{
	moji = fgetc(fp);
	if (moji == EOF) return(EOF);
	if (moji == '\n') line++;
	} while ((moji == ' ') || (moji == '\t') || (moji == '\n'));
	
	if (moji == ';')
		{
		/*コメントだったら行末まで、読み捨て*/
		do
		{
		moji = fgetc(fp);
		if (moji == EOF) return(EOF);
		} while (moji != '\n');
		
		line++;
		goto retry;
		}
		
	tk[pt++] = moji;
	if ((moji == '{') || (moji == '}'))
		{
		tk[pt++] = '\0';
		return(pt);
		}
	
	moji = fgetc(fp);
	while ((moji != ' ') && (moji != '\t') && (moji != '\n') && (moji != ',') && (moji != '{') && (moji != '}') && (moji != '['))
		{
		tk[pt++] = moji;
		moji = fgetc(fp);
		}
	if (moji == '\n') line++;
	if ((moji == '{') || (moji == '}') || (moji == '['))
		{
		ungetc(moji, fp);
		}
	
	tk[pt++] = '\0';
	/*tkの内容を小文字に変更*/
	jstrlwr(tk);
	
	return(pt);
}

void soundtrans (unsigned char before[37],unsigned char after[29])

{
	/*ａｆｔｅｒには、２９ｂｙｔｅ以上の領域が必要*/
	int i,op,sp;
	for (i=0;i<=3;i++)
		{
		op=i;
		if (i==1) op=2;
		if (i==2) op=1;
		sp=op*9;
		after[i]=(before[sp+8] & 7)*16+before[sp+7];
		after[i+4]=before[sp+5];
		after[i+8]=before[sp+6]*64+before[sp];
		after[i+12]=before[sp+1];
		after[i+16]=before[sp+2];
		after[i+20]=before[sp+4]*16+before[sp+3];
		after[i+24]=0;
		}
		after[28]=before[36];
}
	
unsigned int trans (unsigned char *dat,unsigned char *rdat,int psg, int ch)
{
/*datをＯＰＮＤＲＶで使えるような形式に変換する*/
    static unsigned char *fname[2]={
				"toftonsstsvlsmlsolkonsposvbnopsdtsfsendtmpot1ot2jmpletincdecif1if2nopnopnopnopnopnopnopnopnoplonlofsapstrpcmpedpstifeifp",
				"toftonsstssvsvlsspsnssposvbmslsdtsfsendtmpot1ot2jmpletincdecif1if2iedictshvsesesteoneofstaipssldnopnopnoppcmpedpstifeifp"};

	int l=4;
	int oct=4;
	int p=0;	/*pointer*/
	int ll;
	int se;
	int neg;
	
	unsigned char m1;
	unsigned char oto;
	unsigned int rp=0;
	float ff,fff;
	static int slab[52][9];	/*ラベル（＠＠ａ－＠＠ｚ）*/
	static char slabf[52][9];
	
	static int stkpt[9] =
	{
	0, 0, 0, 0, 0, 0, 0, 0, 0};	/*ループ用スタック*/
	static int stk[16];
	static int kaisu[16];
	
	static int lpflag[16]=
	{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
	
	static int lptbl[16];
	
	unsigned char *clab;
	int i, ii;
	unsigned int k;
	unsigned char ffname[4];
	unsigned char dmy;
	int tieflag=0;
	static unsigned int clabmax;
	int rflag = 0;
	int zettai = 0;
	
	if (psg == 2)
	{
		psg = 1;
		se = 1;
	}
	else
	{
		se = 0;
	}
	
	if (rdat == NULL) rflag = 1;
	
	if (rflag == 0)
	{
		if ((clab=(unsigned char  *)malloc(clabmax))==NULL) 
		{
			/*ラベル用エリアが確保できない*/
			printf("ラベル用エリアが確保できませんでした\n");
			exit(1);
		}
		setmem (clab,clabmax,' ');
	}
	


	ff=1;
	ffname[3]='\0';

	for (p=0;dat[p]!=0;p++)
		{
		m1=dat[p];
		if ((m1>='a') && (m1<='z')) m1-=('a'-'A');
		ll=l;
		fff=ff;

		switch (m1) {
		 /*音*/
		 case 'C':	oto=1+DET;goto mwork;
		 case 'D':	oto=3+DET;goto mwork;
		 case 'E':	oto=5+DET;goto mwork;
		 case 'F':	oto=6+DET;goto mwork;
		 case 'G':	oto=8+DET;goto mwork;
		 case 'A':	oto=0xa+DET;goto mwork;
		 case 'B':	oto=0xc+DET;goto mwork;
		 case 'R':	oto=0;goto mwork;
		 case 'P':	oto=0;goto mwork;

		 /*繰り返し記号*/
		 case '|':	
		 			p++; m1 = dat[p];
		 			if (m1 != ':') goto sterr;
		 			/*後ろに回数が書いてあるか?*/
		 			m1 = dat[p + 1];
					if ((m1<'0') || (m1>'9'))
					{
						i = 2;
					}
					else
					{
						i=m1-'0';
						p++;m1=dat[p+1];
						while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
		 			}
		 			
		 			if (rflag == 0)
		 			{
		 				kaisu[stkpt[ch]] = i;
		 				stk[stkpt[ch]] = rp + optbuf[ch];
		 				stkpt[ch]++;
		 				if (stkpt[ch] > 15) 
		 				{
		 						printf("繰り返し記号は16回までしかネストできません\n");
		 						exit(1);
		 				}
		 			}
		 			
		 			continue;
		 
		 case '/':
		 			if (lpflag[ch] != 0) 
		 			{
		 				printf("一つの繰り返し記号ループ内に2つ以上のループ分岐記号(/)があります\n");
		 				exit(1);
		 			}
		 			i = stkpt[ch] - 1;
		 			
		 			if (rflag == 0) rdat[rp] = 0xac;
		 			rp++;
		 			if (rflag == 0) rdat[rp] = kaisu[i];
		 			rp++;
		 			if (rflag == 0) rdat[rp] = kaisu[i];
		 			rp++;
		 			
		 			if (rflag == 0)	lpflag[ch] = 1;
		 			lptbl[ch] = rp + optbuf[ch];
		 			
		 			
		 			/*飛び先2バイト*/
		 			if (rflag == 0) rdat[rp] = 0;
		 			rp++;
		 			if (rflag == 0) rdat[rp] = 0;
		 			rp++;
		 			
		 			continue;
		 
		 case ':':
		 			p++; m1 = dat[p];
		 			if (m1 != '|') goto sterr;
		   			
		   			if (rflag == 0)	stkpt[ch]--;
		   			if (stkpt[ch] < 0) 
		   			{
		 						printf("繰り返し記号の数が合っていません\n");
		 						exit(1);
		 			}
		   			
		   			/*スタックから1つ取り出して書き込む*/
		   			if (rflag == 0) rdat[rp] = 0xab;
		   			rp++;
		   			if (rflag == 0) rdat[rp] = kaisu[stkpt[ch]];
		   			rp++;
		   			if (rflag == 0) rdat[rp] = kaisu[stkpt[ch]];
		   			rp++;
		   			if (rflag == 0) rdat[rp] = (-(rp + optbuf[ch]) + (stk[stkpt[ch]])) & 0xff;
		   			rp++;
		   			if (rflag == 0) rdat[rp] = (-(rp + optbuf[ch]) + (stk[stkpt[ch]])) >> 8;
		   			rp++;
		   			
		   			if (lpflag[ch] != 0)
		   			{
		   				if (rflag == 0)
		   				{
		   					rdat[lptbl[ch] - optbuf[ch]] = ((rp - 5 + optbuf[ch]) - lptbl[ch]) & 0xff;
		   					rdat[lptbl[ch] - optbuf[ch] + 1] = ((rp - 5 + optbuf[ch]) - lptbl[ch]) >> 8;		
		   				lpflag[ch] = 0;
		   				}
		   			}
		   			
		   			continue;
		   			
		 /*オクターブ*/
		 case 'O':      p++;m1=dat[p];
				if ((m1<'1') || (m1>'8')) goto sterr;
				oct=m1-'0';
				continue;
		 case '<':	oct--;
				if (oct<1) goto sterr;
				continue;
		 case '>':	oct++;
				if (oct>8) goto sterr;
				continue;
		 /*長さ*/
		 case 'L':	goto leng;
				continue;
				
		 /*ラベル*/
		 case '@':	p++;m1=dat[p];
		 		if ((m1 >= '0') && (m1 <= '9')) goto soundset;
				if ((m1 == 'V') || (m1 == 'v')) goto vol;
				if ((m1 == 'M') || (m1 == 'm')) goto slr;
				if ((m1 == 'R') || (m1 == 'r')) goto sr;
				if ((m1 == 'L') || (m1 == 'l')) goto sl;
				
				if (m1!='@') goto sterr;
				p++;m1=dat[p];
                if ((m1>='a') && (m1<='z')) m1-=('a'-'A');
				if ((m1<'A') || (m1>'Z')) goto sterr;
				slab[m1-'A'][ch]=rp + optbuf[ch];
				slabf[m1 - 'A'][ch] = 1;
				continue;

				vol:
				if ((m1!='V') && (m1!='v')) goto sterr;
				if (psg==1) goto sterr;

				m1=dat[p+1];
				if ((m1<'0') || (m1>'9')) goto sterr;
				i=m1-'0';
				p++;m1=dat[p+1];
				  while ((m1>='0') && (m1<='9'))
					{i=i*10+m1-'0';p++;m1=dat[p+1];}
				if(rflag == 0)rdat[rp]=0x83;
				rp++;
				
				if (i > 127) goto sterr;
				
				if(rflag == 0)rdat[rp]=127-i ;
				rp++;
				continue;
				
				slr:
				/*LR*/
				if (rflag == 0) rdat[rp] = 0xa9;
				rp++;
				if (rflag == 0) rdat[rp] = 3;
				rp++;
				continue;
				
				sr:
				/*R*/
				if (rflag == 0) rdat[rp] = 0xa9;
				rp++;
				if (rflag == 0) rdat[rp] = 1;
				rp++;
				continue;
				
				sl:
				/*L*/
				if (rflag == 0) rdat[rp] = 0xa9;
				rp++;
				if (rflag == 0) rdat[rp] = 2;
				rp++;
				continue;
				
		 /*tempo*/

		 case 'T':      m1=dat[p+1];
				if ((m1<'0') || (m1>'9')) goto sterr;
				i=m1-'0';
				p++;m1=dat[p+1];
				 while ((m1>='0') && (m1<='9'))
					{i=i*10+m1-'0';p++;m1=dat[p+1];}
				if(rflag == 0)
				{
					if (psg==0)	rdat[rp]=0x8d; else rdat[rp] = 0xa8;
				}
				
				rp++;
				if(rflag == 0)rdat[rp]=256-208200/(L4*i);
				rp++;

				continue;
		 case ' ':	continue;
		 case ',':	continue;
		 case '\n':	continue;
		 case 'M':	goto ssp;
		 case 'S':	goto senv;
		 case 'Y':	goto ycom;

		 case 'V':	if (psg==1) goto vset;
				     else {
					  m1=dat[p+1];
					  if ((m1<'0') || (m1>'9')) goto sterr;
					  i=m1-'0';
					  p++;m1=dat[p+1];
					  while ((m1>='0') && (m1<='9'))
						{i=i*10+m1-'0';p++;m1=dat[p+1];}
					  if(rflag == 0)rdat[rp]=0x83;
					  rp++;
					  if (i > 15) goto sterr;
					  if(rflag == 0)rdat[rp]=2+(15-i)*3;
					  rp++;
					  continue;
					  }

		 /*コマンド*/
		 case '*':      /*機能名を探す*/

				for (i=0;i<=2;i++)
				{
				ffname[i]=dat[p+1+i];
				if ((ffname[i]>='A') && (ffname[i]<='Z')) ffname[i]+='a'-'A';
				}
			
				for (i=0;i<=strlen(fname[psg]);i+=3)
				{
					if ((ffname[0]==fname[psg][i]) && (ffname[1]==fname[psg][i+1]) && (ffname[2]==fname[psg][i+2]))
					{
						goto found;
					}
				}
				
				printf("コマンド[%c%c%c]が見つかりません\n", ffname[0], ffname[1], ffname[2]);
				goto sterr;	/*コマンド名が見つからない*/

				found:
				p+=3;
				i/=3;
				switch (i) {
				  case 0:	/*タイオフ*/
						if(rflag == 0)rdat[rp]=0x80;
						rp++;tieflag=0;continue;
				  case 1:	/*タイオン*/
						if(rflag == 0)rdat[rp]=0x81;
						rp++;tieflag=1;continue;
				  case 2:	/*音色設定*/
						m1=dat[p+1];
						if (m1!='@') goto sterr;
						p++;
						m1=dat[p+1];
						p++;
					soundset:
						
						i = m1 - '0';
						m1 = dat[p + 1];
						while ((m1 >= '0') && (m1 <= '9'))
						{
							i = i * 10 + m1 - '0'; p++;
							m1 = dat[p + 1];
						}
						
						p++;
						if (psg == 1)
						{
							if (rflag == 0) rdat[rp] = 0x83;
							rp++;
							if (rflag == 0) rdat[rp] = 1;
							rp++;
						}
						
						if(rflag == 0)rdat[rp]=0x82;
						rp++;
						
						if ((i < 0) || (i > 127))
						{
							printf("音色番号が大きすぎます\n");
							goto sterr;
						}
						
						if(rflag == 0)rdat[rp]=((unsigned int)i) % 256;
						rp++;
						if(rflag == 0)rdat[rp]=((unsigned int)i) / 256;
						rp++;
						continue;
				  case 3:	/*ボリューム／ＳＶＥ　オンオフ*/
						m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
						i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}

						if(rflag == 0)rdat[rp]=0x83;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
						continue;
				  case 4:
						vset:

						/*モジュレーターｌｖ／ｖｏｌ*/
						m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}

						if(psg == 1)
						{
							if (rflag == 0) rdat[rp] = 0x83;
							rp++;
							if (rflag == 0) rdat[rp] = 0;
							rp++;
						}
						
						if(rflag == 0)rdat[rp]=0x84;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
						continue;
				  case 5:	/*モジューレータｌｖ／ｓｖｅ　ｓｐｅｅｄ*/
						m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=0x85;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
						if (psg==1) continue;
						if ((dat[p+1]!=',') && (dat[p+1]!=' ')) goto sterr;
						p++;
						m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if (psg != 1)
						{
							i--;
						}
						if(rflag == 0)rdat[rp]=i;
						rp++;
						continue;
				  case 6:       m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=0x86;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
						continue;
				  case 7:	m1=dat[p+1];
						if (m1 == '-')
						{
							neg = -1;
							p++;
							m1 = dat[p + 1];
						}
						else
						{
							neg = 1;
						}
						if (psg == 1) neg = -neg;
						
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=0x87;
						rp++;
						if(rflag == 0)rdat[rp]=i * neg;
						rp++;
						continue;
				  case 8:	m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=0x88;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
                        if ((dat[p+1]!=',') && (dat[p+1]!=' ')) goto sterr;
						p++;
						m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=i;
						rp++;
						if ((dat[p+1]!=',') && (dat[p+1]!=' ')) goto sterr;
						p++;
						m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=i;
						rp++;
                        
                        if ((dat[p+1]!=',') && (dat[p+1]!=' ')) goto sterr;
						p++;
						m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=i;
						rp++;
						continue;
				  case 9:       m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=0x89;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
						continue;
				  case 10:	m1=dat[p+1];
				  		if (m1 == '-')
						{
							neg = -1;
							p++;
							m1 = dat[p + 1];
						}
						else
						{
							neg = 1;
						}
						if (psg == 1) neg = -neg;
						
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=0x8a;
						rp++;
						if(rflag == 0)rdat[rp]=i * neg;
						rp++;
						continue;
				  case 11:	m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=0x8b;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
						continue;
				  case 12:	if(rflag == 0)rdat[rp]=0x8c;
				  		rp++;
						continue;
				  case 13:	
				  		m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)
						{
							if (psg == 0) rdat[rp]=0x8d; else rdat[rp] = 0xa8;
						}
						
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
						continue;
				  case 14:      ycom:

						m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=0x8e;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
						
						p++;
						m1=dat[p+1];
						if ((m1!=',') && (m1!=' ')) goto sterr;
						p++;
						m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=i;
						rp++;
						
						continue;
				  case 15:	m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=0x8f;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
						
						p++;
						m1=dat[p+1];
						if ((m1!=',') && (m1!=' ')) goto sterr;
						p++;
						m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=i;
						rp++;
						
						
						continue;
				  case 16:	m1=dat[p+1];
						if (m1!='@')	goto sterr;
						p++;
						m1=dat[p+1];
						if (m1!='@') 	goto sterr;
						p++;
						m1=dat[p+1];
                        if ((m1>='a') && (m1<='z')) m1-=('a'-'A');
						if ((m1<'A') || (m1>'Z')) goto sterr;
						p++;
						if(rflag == 0)rdat[rp]=0xa0;
						rp++;
						if (rflag == 0)
						{
							clab[rp]='@';rp++;
							clab[rp]=m1;rp++;
						}
						else
						{
						rp += 2;
						}
						
						continue;
				  case 17:	m1=dat[p+1];
						if (m1!='@')	goto sterr;
						p++;
						m1=dat[p+1];
						if (m1!='@')	goto sterr;
						p++;
						m1=dat[p+1];
						if ((dat[p+2]<'0') || (dat[p+2]>'9')) i=m1-'0'; else {i=(m1-'0')*10+(dat[p+2]-'0');p++;}
						if(rflag == 0)rdat[rp]=0xa1;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
						p++;
						m1=dat[p+1];
						if ((m1!=',') && (m1!=' ')) goto sterr;
						p++;
						m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=i;
						rp++;
						continue;
				  case 18:      m1=dat[p+1];
						if (m1!='@')	goto sterr;
						p++;
						m1=dat[p+1];
						if (m1!='@')	goto sterr;
						p++;
						m1=dat[p+1];
						if ((dat[p+2]<'0') || (dat[p+2]>'9')) i=m1-'0'; else {i=(m1-'0')*10+(dat[p+2]-'0');p++;}
						p++;
						if(rflag == 0)rdat[rp]=0xa2;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
						continue;
				  case 19:      m1=dat[p+1];
						if (m1!='@')	goto sterr;
						p++;
						m1=dat[p+1];
						if (m1!='@')	goto sterr;
						p++;
						m1=dat[p+1];
						if ((dat[p+2]<'0') || (dat[p+2]>'9')) i=m1-'0'; else {i=(m1-'0')*10+(dat[p+2]-'0');p++;}
						p++;
						if(rflag == 0)rdat[rp]=0xa3;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
						continue;

				  case 20:      m1=dat[p+1];
						if (m1!='@')	goto sterr;
						p++;
						m1=dat[p+1];
						if (m1!='@')	goto sterr;
						p++;
						m1=dat[p+1];
						if ((dat[p+2]<'0') || (dat[p+2]>'9')) i=m1-'0'; else {i=(m1-'0')*10+(dat[p+2]-'0');p++;}
						if(rflag == 0)rdat[rp]=0xa4;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
						p++;
                                                if ((dat[p+1]!=',') && (dat[p+1]!=' ')) goto sterr;
						p++;


						m1=dat[p+1];
						if (m1!='@')	goto sterr;
						p++;
						m1=dat[p+1];
						if (m1!='@') 	goto sterr;
						p++;
						m1=dat[p+1];
                                                if ((m1>='a') && (m1<='z')) m1-=('a'-'A');
						if ((m1<'A') || (m1>'Z')) goto sterr;
						p++;
						if (rflag == 0)
						{
							clab[rp]='@';rp++;
							clab[rp]=m1;rp++;
						}
						else
						{
						rp += 2;
						}
						
						continue;
				  case 21:      m1=dat[p+1];
						if (m1!='@')	goto sterr;
						p++;
						m1=dat[p+1];
						if (m1!='@')	goto sterr;
						p++;
						m1=dat[p+1];
						if ((dat[p+2]<'0') || (dat[p+2]>'9')) i=m1-'0'; else {i=(m1-'0')*10+(dat[p+2]-'0');p++;}
						if(rflag == 0)rdat[rp]=0xa5;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
						p++;

						if ((dat[p+1]!=',') && (dat[p+1]!=' ')) goto sterr;
						p++;
						m1=dat[p+1];
						if (m1!='@')	goto sterr;
						p++;
						m1=dat[p+1];
						if (m1!='@') 	goto sterr;
						p++;
						m1=dat[p+1];
                                                if ((m1>='a') && (m1<='z')) m1-=('a'-'A');
						if ((m1<'A') || (m1>'Z')) goto sterr;
						p++;
						if (rflag == 0)
						{
							clab[rp]='@';rp++;
							clab[rp]=m1;rp++;
						}
						else
						{
						rp+=2;
						}
						continue;
				  case 22:      if(rflag == 0)rdat[rp]=0x96;
						rp++;
						continue;

				  case 23:      m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=0x97;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
						continue;
				  case 24:	m1=dat[p+1];
						if ((m1!='0') && (m1!='1')) goto sterr;
						p++;
						if(rflag == 0)rdat[rp]=0x98;
						rp++;
						if(rflag == 0)rdat[rp]=m1-'0';
						rp++;
						continue;
				  case 25:      ssp:

						m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=0x99;
						rp++;
						if(rflag == 0)rdat[rp]=i % 256;
						rp++;
						if(rflag == 0)rdat[rp]=i / 256;
						rp++;
						continue;
				  case 26:      senv:

						m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=0x9a;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
						continue;
				  case 27:	if(rflag == 0)rdat[rp]=0x9b;
				  		rp++;
						continue;
				  case 28:	if(rflag == 0)rdat[rp]=0x9c;
				  		rp++;
						continue;
				  case 29:	
						m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=0x9d;
						rp++;
						if(rflag == 0)rdat[rp]=i % 256;
						rp++;
						if(rflag == 0)rdat[rp]=i / 256;
						rp++;
						continue;
				  case 30:

						m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=0x9e;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
						continue;
				  
				  case 31:
				  		if (psg == 0)
				  		{
				  		m1 = dat[p + 1];
				  		if ((m1 <'0') || (m1 >'9')) goto sterr;
				  		i=m1 - '0';
				  		p++;m1 = dat[p + 1];
				  		while ((m1 >= '0') && (m1 <= '9'))
				  			{
				  			i = i * 10 + m1 - '0'; p++;
				  			m1 = dat[p + 1];
				  			}
				  		if (rflag == 0) rdat[rp] = 0xa8;
				  		rp++;
				  		if (rflag == 0) rdat[rp] = (i & 7) + 8;
				  		rp++;
				  		continue;
				  		}
				  		else
				  		{
				  		m1 = dat[p + 1];
				  		
				  		if ((m1 <'0') || (m1 >'9')) goto sterr;
				  		i=m1 - '0';
				  		p++;m1 = dat[p + 1];
				  		while ((m1 >= '0') && (m1 <= '9'))
				  			{
				  			i = i * 10 + m1 - '0'; p++;
				  			m1 = dat[p + 1];
				  			}
				  		if (rflag == 0) rdat[rp] = 0x8d;
				  		rp++;
				  		if (rflag == 0) rdat[rp] = i;
				  		rp++;
				  		
						if ((dat[p+1]!=',') && (dat[p+1]!=' ')) goto sterr;
						p++;
				  		
				  		m1 = dat[p + 1];
				  		
				  		if ((m1 <'0') || (m1 >'9')) goto sterr;
				  		i=m1 - '0';
				  		p++;m1 = dat[p + 1];
				  		while ((m1 >= '0') && (m1 <= '9'))
				  			{
				  			i = i * 10 + m1 - '0'; p++;
				  			m1 = dat[p + 1];
				  			}
				  		
				  		if (rflag == 0) rdat[rp] = i;
				  		rp++;
				  		continue;
				  		}
				  		
				  case 32:
				  		if (rflag == 0) rdat[rp] = 0xa8;
				  		rp++;
				  		if (rflag == 0) rdat[rp] = 0;
				  		rp++;
				  		continue;
				  case 33:
				  		/*AMS*/
				  		m1 = dat[p + 1];
				  		if ((m1 <'0') || (m1 >'9')) goto sterr;
				  		i = m1 - '0';
				  		p++; m1 = dat[p + 1];
				  		while ((m1 >= '0') && (m1 <= '9'))
				  		{
				  			i = i * 10 + m1 - '0'; p++;
				  			m1 = dat[p + 1];
				  		}
				  		
						if ((dat[p+1]!=',') && (dat[p+1]!=' ')) goto sterr;
						p++;
				  		
				  		/*PMS*/
				  		m1 = dat[p + 1];
				  		if ((m1 <'0') || (m1 >'9')) goto sterr;
				  		ii = m1 - '0';
				  		p++; m1 = dat[p + 1];
				  		while ((m1 >= '0') && (m1 <= '9'))
				  		{
				  			ii = ii * 10 + m1 - '0'; p++;
				  			m1 = dat[p + 1];
				  		}
				  		
				  		i = ((i & 3) << 4) + (ii & 7);
				  		
				  		if (rflag == 0) rdat[rp] = 0xaa;
				  		rp++;
				  		if (rflag == 0) rdat[rp] = i;
				  		rp++;
				  		
						if ((dat[p+1]!=',') && (dat[p+1]!=' ')) goto sterr;
						p++;
				  		
				  		/*AMON*/
				  		m1 = dat[p + 1];
				  		if ((m1 <'0') || (m1 >'9')) goto sterr;
				  		i = m1 - '0';
				  		p++; m1 = dat[p + 1];
				  		while ((m1 >= '0') && (m1 <= '9'))
				  		{
				  			i = i * 10 + m1 - '0'; p++;
				  			m1 = dat[p + 1];
				  		}
				  		
				  		/*OP2とOP3を交換*/
				  		i = (i & 9) + ((i & 2) << 1) + ((i & 4) >> 1);
				  		if (rflag == 0) rdat[rp] = i & 15;
				  		rp++;
				  		continue;
				 
				 case 34:
				 		/*トレモロ*/
				 		m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=0xad;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
                        if ((dat[p+1]!=',') && (dat[p+1]!=' ')) goto sterr;
						p++;
						m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                        i=m1-'0';
						p++;m1=dat[p+1];
						while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=i;
						rp++;
						if ((dat[p+1]!=',') && (dat[p+1]!=' ')) goto sterr;
						p++;
						m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=i;
						rp++;
                        
                        if ((dat[p+1]!=',') && (dat[p+1]!=' ')) goto sterr;
						p++;
						m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=i;
						rp++;
						
                        if ((dat[p+1]!=',') && (dat[p+1]!=' ')) goto sterr;
						p++;
						m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=i;
						rp++;
						
						
						continue;
				 
				 case 35:
				 		/*PCMスタート*/
				 		m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
				 		
				 		if ((i < 0) || (i > 99))
				 		{
				 			/*番号が大きすぎる*/
				 			goto sterr;
				 		}
				 		
				 		if (rflag == 0) rdat[rp] = 0xae;
				 		rp++;
				 		if (rflag == 0) rdat[rp] = i;
				 		rp++;
				 		if (rflag == 0) rdat[rp] = i;
				 		rp++;
				 		if (rflag == 0) rdat[rp] = i;
				 		rp++;
				 		if (rflag == 0) rdat[rp] = i;
				 		rp++;
				 		
				 		continue;
				  case 36:
				  		/*PCMストップ*/
				  		if (rflag == 0) rdat[rp] = 0xaf;
				  		rp++;
						continue;
						
				  case 37:
				  		/*PCMストップ*/
				  		if (rflag == 0) rdat[rp] = 0xb0;
				  		rp++;
				  		continue;
				  case 38:
				  		/*YM2608 ONでジャンプ*/
				  		m1=dat[p+1];
						if (m1!='@')	goto sterr;
						p++;
						m1=dat[p+1];
						if (m1!='@') 	goto sterr;
						p++;
						m1=dat[p+1];
                        if ((m1>='a') && (m1<='z')) m1-=('a'-'A');
						if ((m1<'A') || (m1>'Z')) goto sterr;
						p++;
						if(rflag == 0)rdat[rp]=0xb1;
						rp++;
						if (rflag == 0)
						{
							clab[rp]='@';rp++;
							clab[rp]=m1;rp++;
						}
						else
						{
						rp += 2;
						}
						
						continue;
		
				  case 39:
				  		/*PCM機能 ONでジャンプ*/
				  		m1=dat[p+1];
						if (m1!='@')	goto sterr;
						p++;
						m1=dat[p+1];
						if (m1!='@') 	goto sterr;
						p++;
						m1=dat[p+1];
                        if ((m1>='a') && (m1<='z')) m1-=('a'-'A');
						if ((m1<'A') || (m1>'Z')) goto sterr;
						p++;
						if(rflag == 0)rdat[rp]=0xb2;
						rp++;
						if (rflag == 0)
						{
							clab[rp]='@';rp++;
							clab[rp]=m1;rp++;
						}
						else
						{
						rp += 2;
						}
						
						continue;
	
				  		
				  }
			default:	continue;

				}

		mwork:
		 /*次の一文字を読む*/
		 m1=dat[p+1];
		 if ((m1=='+') || (m1=='#')) {p++;m1=dat[p+1];oto++;}
		 if (m1=='-') {p++;m1=dat[p+1];oto--;}

		 if ((oto & 0xf)> 12) oto=(oto & 0xf)-12+ (oto & 0xf0)+0x10;
		 
		 if (m1 == '*')
		 {
		 	/*絶対音長*/
		 	zettai = 1;
		 	p++;
		 	m1 = dat[p + 1];
		 }
		 else
		 {
		 	zettai = 0;
		 }
		 
		 if ((m1>='0') && (m1<='9'))
			{
			ll=m1-'0';p++;
			m1=dat[p+1];
			while ((m1>='0') && (m1<='9'))
				{
				ll=ll*10+m1-'0';p++;m1=dat[p+1];
				}
			}
		 if (m1=='.') {fff=1.5;p++;}

		 /*書き込み*/
		 if (zettai == 0)
		 {
		 	i=(unsigned int)((4*L4*fff)/ll);
		 }
		 else
		 {
		 	i = ll * fff;
		 }
		 
		 if (i<=127)

		 {
		 if(rflag == 0)rdat[rp]=(unsigned char)(i);
		 rp++;
		 if (oto!=0) 
		 	{
		 	if(rflag == 0)rdat[rp]=(unsigned char)((oct-1)*16+oto);
			}
			else 
			{
			if(rflag == 0)rdat[rp]=0;
		 	}
		 rp++;
		 }

		 else
		 {
		 /*音長が１２７を超えているためタイの処理にする*/
		 if (tieflag==0) 
		 	{if(rflag == 0)rdat[rp]=0x81;
		 	rp++;}

		 do
		 {
		 if(rflag == 0)rdat[rp]=127;
		 rp++;
		 if (oto!=0) 
		 	{
		 	if(rflag == 0)rdat[rp]=(unsigned char)((oct-1)*16+oto);
			}
			else 
			{
			if(rflag == 0)rdat[rp]=0;
		 	}
		 rp++;
		 i-=127;
		 }while (i>127);

		 if(rflag == 0)rdat[rp]=(unsigned char)(i);
		 rp++;
		 if (oto!=0) 
		 	{
		 	if(rflag == 0)rdat[rp]=(unsigned char)((oct-1)*16+oto);
			}
			else 
			{
			if(rflag == 0)rdat[rp]=0;
		 	}
		 
		 rp++;

		 if (tieflag==0) 
		 	{if(rflag == 0)rdat[rp]=0x80;
		  	rp++;}

		 }

		 continue;

		 leng:

		 m1 = dat[p + 1];
		 if ((m1>='0') && (m1<='9'))
			{
			l=m1-'0';p++;
			m1=dat[p+1];
			while ((m1>='0') && (m1<='9'))
				{
				l=l*10+m1-'0';p++;m1=dat[p+1];
				}
			}
		 if (m1=='.') {ff=1.5;p++;}

		 continue;

		 }


	if(rflag == 0)
	{
		if (se == 0) rdat[rp]=0x8c;
			else rdat[rp] = 0x96;
	}
	rp++;
	if(rflag == 0)rdat[rp]=0;
	rp++;
	
	/*ラベルの記入*/

	if (rflag == 0)
	{
	for (i=0;i<=rp-1;i++)
		{
		if (clab[i]!='@') continue;
		if (slabf[clab[i + 1] - 'A'][ch] == 0)
		{
			puterrno(NULL);
			printf("ラベル[@@%c]が定義されていません\n", clab[i + 1]);
			exit(1);
		}
		k=slab[clab[i+1]-'A'][ch]-(i + optbuf[ch]);
		if(rflag == 0)rdat[i]=k & 255;
		if(rflag == 0)rdat[i+1]=k >> 8;
		}
	
	free (clab);
	}
	
	clabmax = rp;
	return(rp);


	sterr:
	
	for (i = p - 20; i < p + 20; i++)
	{
		if (i == p-1) printf("\x1b[32m");
		if (i >= 0) putchar(dat[i]);
		if ((dat[i] == '\0') && (i > p)) break;
		if (i == p) printf("\x1b[0m");
	}
	
	printf("MML文法ミスです\n");
	
	exit(1);
	return(p);

}

unsigned int rth_trans (unsigned char *dat,unsigned char *rdat,int psg, int ch)
{
/*datをＯＰＮＤＲＶで使えるような形式に変換する(リズム音源)*/
    static unsigned char *fname[2]={
				"toftonsstsvlsmlsolkonsposvbnopsdtsfsendtmpot1ot2jmpletincdecif1if2nopnopnopnopnopnopnopnopnoplonlofsapstrpcmpedpstifeifp",
				"toftonsstssvsvlsspsnssposvbmslsdtsfsendnopot1ot2jmpletincdecif1if2iedictshvsesesteoneofstaips"};

	int l=4;
	int oct=4;
	int p=0;	/*pointer*/
	int ll;
	unsigned char m1;
	unsigned char oto;
	unsigned int rp=0;
	float ff,fff;
	static int slab[52][15];	/*ラベル（＠＠ａ－＠＠ｚ）*/
	static char slabf[52][15];	/*ラベル（＠＠ａ－＠＠ｚ）*/
	unsigned char *clab;
	int i;
	unsigned int k;
	unsigned char ffname[4];
	unsigned char dmy;
	int tieflag=0;
	static unsigned int clabmax;
	int rflag = 0;
	int vv;
	int v = 31;
	int swc;
	int LR = 3;
	static int stkpt[15] =
	{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};/*ループ用スタック*/
	static int stk[16];
	static int kaisu[16];

	static int lpflag[16] =
	{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static int lptbl[16];
	int zettai = 0;
	
	if (rdat == NULL) rflag = 1;
	
	
	if (rflag == 0)
	{
		if ((clab=(unsigned char  *)malloc(clabmax))==NULL) {return (-1);}
	
		setmem (clab,clabmax,' ');
	}
	


	ff=1;
	ffname[3]='\0';

	for (p=0;dat[p]!=0;p++)
		{
		m1=dat[p];
		if ((m1>='a') && (m1<='z')) m1-=('a'-'A');
		ll=l;
		fff=ff;
		vv = v;
		
		switch (m1) {
		 /*音*/
		 case 'A':	vv = 31; swc = 0; goto mwork;
		 case 'B':	vv = 28; swc = 0; goto mwork;
		 case 'C':	vv = 20; swc = 0; goto mwork;
		 case 'D':	vv = 16; swc = 0; goto mwork;
		 case 'E':	vv = 12; swc = 0; goto mwork;
		 case 'F':	vv =  8; swc = 0; goto mwork;
		 case 'G':	vv =  4; swc = 0; goto mwork;
		 case 'H':	vv =  1; swc = 0; goto mwork;
		 case 'X':	swc = 0; goto mwork;
		 case 'R':	vv =  0; swc = 1; goto mwork;
		 case 'P':	vv =  0; swc = 0; goto mwork;

		 /*繰り返し記号*/
		 case '|':	
		 			p++; m1 = dat[p];
		 			if (m1 != ':') goto sterr;
		 			/*後ろに回数が書いてあるか?*/
		 			m1 = dat[p + 1];
					if ((m1<'0') || (m1>'9'))
					{
						i = 2;
					}
					else
					{
						i=m1-'0';
						p++;m1=dat[p+1];
						while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
		 			}
		 			
		 			if (rflag == 0)
		 			{
		 				kaisu[stkpt[ch]] = i;
		 				stk[stkpt[ch]] = rp + optbuf[ch];
		 				stkpt[ch]++;
		 				if (stkpt[ch] > 15) 
		 				{
		 						printf("繰り返し記号は16回までしかネストできません\n");
		 						exit(1);
		 				}
		 			}
		 			
					continue;

		 case '/':
		 			if (lpflag[ch] != 0) 
		 			{
		 				printf("一つの繰り返し記号ループ内に2つ以上のループ分岐記号(/)があります\n");
		 				exit(1);
		 			}
		 			i = stkpt[ch] - 1;
		 			
		 			if (rflag == 0) rdat[rp] = 0xac;
		 			rp++;
		 			if (rflag == 0) rdat[rp] = kaisu[i];
		 			rp++;
		 			if (rflag == 0) rdat[rp] = kaisu[i];
		 			rp++;
		 			
		 			if (rflag == 0)
		 			{
		 				lpflag[ch] = 1;
		 				lptbl[ch] = rp + optbuf[ch];
		 			}
		 			
		 			
		 			/*飛び先2バイト*/
		 			rp++;
		 			rp++;
		 			
		 			continue;


		 case ':':
		 			p++; m1 = dat[p];
		 			if (m1 != '|') goto sterr;
		   			
		   			
		   			if (rflag == 0) stkpt[ch]--;
		   			if (stkpt[ch] < 0) 
		   			{
		 						printf("繰り返し記号の数が合っていません\n");
		 						exit(1);
		 			}
		   				
		   			/*スタックから1つ取り出して書き込む*/
		   			if (rflag == 0) rdat[rp] = 0xab;
		   			rp++;
		   			if (rflag == 0) rdat[rp] = kaisu[stkpt[ch]];
		   			rp++;
		   			if (rflag == 0) rdat[rp] = kaisu[stkpt[ch]];
		   			rp++;
		   			if (rflag == 0) rdat[rp] = (-(rp + optbuf[ch]) + (stk[stkpt[ch]])) & 0xff;
		   			rp++;
		   			if (rflag == 0) rdat[rp] = (-(rp + optbuf[ch]) + (stk[stkpt[ch]])) >> 8;
		   			rp++;
		   			
		   			if (lpflag[ch] != 0)
		   			{
		   				if (rflag == 0)
		   				{
		   					rdat[lptbl[ch] - optbuf[ch]] = ((rp - 5 + optbuf[ch]) - lptbl[ch]) & 0xff;
		   					rdat[lptbl[ch] - optbuf[ch] + 1] = ((rp - 5 + optbuf[ch]) - lptbl[ch]) >> 8;		
		   					lpflag[ch] = 0;
		   				}
		   			}
		   			
		   			
		   			continue;

		 /*長さ*/
		 case 'L':	goto leng;

		 /*ラベル*/
		 case '@':	p++;m1=dat[p];
				if ((m1 == 'V') || (m1 == 'v')) goto vol;
				if ((m1 == 'M') || (m1 == 'm')) goto slr;
				if ((m1 == 'R') || (m1 == 'r')) goto sr;
				if ((m1 == 'L') || (m1 == 'l')) goto sl;
				
				if (m1 != '@') goto sterr;
				
				p++;m1=dat[p];
                if ((m1>='a') && (m1<='z')) m1-=('a'-'A');
				if ((m1<'A') || (m1>'Z')) goto sterr;
				slab[m1-'A'][ch]=rp + optbuf[ch];
				slabf[m1 - 'A'][ch] = 1;
				continue;

				
				
				vol:
				if ((m1!='V') && (m1!='v')) goto sterr;
				if (psg==1) goto sterr;

				m1=dat[p+1];
				if ((m1<'0') || (m1>'9')) goto sterr;
				i=m1-'0';
				p++;m1=dat[p+1];
				  while ((m1>='0') && (m1<='9'))
					{i=i*10+m1-'0';p++;m1=dat[p+1];}
				
				v = i;
				if ((v < 0) || (v > 31)) goto sterr;
				
				/*
				if(rflag == 0)rdat[rp]=0x83;
				rp++;
				if(rflag == 0)rdat[rp]=127-i ;
				rp++;
				*/
				
				continue;

				slr:
					/*L+R*/
					LR = 3;
					continue;
				sr:
					/*R*/
					LR = 1;
					continue;
				
				sl:
					/*L*/
					LR = 2;
					continue;
					
		 /*tempo*/

		 case 'T':      m1=dat[p+1];
				if ((m1<'0') || (m1>'9')) goto sterr;
				i=m1-'0';
				p++;m1=dat[p+1];
				 while ((m1>='0') && (m1<='9'))
					{i=i*10+m1-'0';p++;m1=dat[p+1];}
				if(rflag == 0)rdat[rp]=0x8d;
				rp++;
				if(rflag == 0)rdat[rp]=256-208200/(L4*i);
				rp++;

				continue;
		 case ' ':	continue;
		 case ',':	continue;
		 case '\n':	continue;
		 case 'Y':	goto ycom;

		 case 'V':	
		 			m1=dat[p+1];
					if ((m1<'0') || (m1>'9')) goto sterr;
					i=m1-'0';
					p++;m1=dat[p+1];
					while ((m1>='0') && (m1<='9'))
						{i=i*10+m1-'0';p++;m1=dat[p+1];}
					
					if (i != 0)
					{
						i = (i + 1)* 2 - 1;
					}
					v = i;
					if ((v < 0) || (v > 31)) goto sterr;
					
					continue;
					
		 /*コマンド*/
		 case '*':      /*機能名を探す*/

				for (i=0;i<=2;i++)
				{
				ffname[i]=dat[p+1+i];
				if ((ffname[i]>='A') && (ffname[i]<='Z')) ffname[i]+='a'-'A';
				}

				for (i=0;i<=strlen(fname[psg]);i+=3)
					{
					if ((ffname[0]==fname[psg][i]) && (ffname[1]==fname[psg][i+1]) && (ffname[2]==fname[psg][i+2]))
						goto found;
	
				}
	
				printf("コマンド[%c%c%c]が見つかりません\n", ffname[0], ffname[1], ffname[2]);
				goto sterr;	/*コマンド名が見つからない*/

				found:
				p+=3;
				i/=3;
				switch (i) {
				  case 0:	/*タイオフ*/
						if(rflag == 0)rdat[rp]=0x80;
						rp++;tieflag=0;continue;
				  case 1:	/*タイオン*/
						if(rflag == 0)rdat[rp]=0x81;
						rp++;tieflag=1;continue;
				  case 2:	/*音色設定*/
						m1=dat[p+1];
						if (m1!='@') goto sterr;
						p++;
						m1=dat[p+1];
						if ((dat[p+2]<'0') || (dat[p+2]>'9')) i=m1-'0'; else {i=(m1-'0')*10+(dat[p+2]-'0');p++;}
						p++;
						if(rflag == 0)rdat[rp]=0x82;
						rp++;
						if(rflag == 0)rdat[rp]=((unsigned int)i) % 256;
						rp++;
						if(rflag == 0)rdat[rp]=((unsigned int)i) / 256;
						rp++;
						continue;
				  case 3:	/*ボリューム／ＳＶＥ　オンオフ*/
						m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
						i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}

						if(rflag == 0)rdat[rp]=0x83;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
						continue;
				  case 4:
						vset:

						/*モジュレーターｌｖ／ｖｏｌ*/
						m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}

						if(rflag == 0)rdat[rp]=0x84;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
						continue;
				  case 5:	/*モジューレータｌｖ／ｓｖｅ　ｓｐｅｅｄ*/
						m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=0x85;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
						if (psg==1) continue;
						if ((dat[p+1]!=',') && (dat[p+1]!=' ')) goto sterr;
						p++;
						m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=i;
						rp++;
						continue;
				  case 6:       m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=0x86;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
						continue;
				  case 7:	m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=0x87;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
						continue;
				  case 8:	m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=0x88;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
                        if ((dat[p+1]!=',') && (dat[p+1]!=' ')) goto sterr;
						p++;
						m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=i;
						rp++;
						if ((dat[p+1]!=',') && (dat[p+1]!=' ')) goto sterr;
						p++;
						m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=i;
						rp++;
                        
                        if ((dat[p+1]!=',') && (dat[p+1]!=' ')) goto sterr;
						p++;
						m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=i;
						rp++;
						continue;
				  case 9:       m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=0x89;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
						continue;
				  case 10:	m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=0x8a;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
						continue;
				  case 11:	m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=0x8b;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
						continue;
				  case 12:	if(rflag == 0)rdat[rp]=0x8c;
				  		rp++;
						continue;
				  case 13:	m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=0x8d;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
						continue;

				  case 14:      ycom:

						m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=0x8e;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
						continue;
				  case 15:	m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=0x8f;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
						continue;
				  case 16:	m1=dat[p+1];
						if (m1!='@')	goto sterr;
						p++;
						m1=dat[p+1];
						if (m1!='@') 	goto sterr;
						p++;
						m1=dat[p+1];
                        if ((m1>='a') && (m1<='z')) m1-=('a'-'A');
						if ((m1<'A') || (m1>'Z')) goto sterr;
						p++;
						if(rflag == 0)rdat[rp]=0xa0;
						rp++;
						if (rflag == 0)
						{
							clab[rp]='@';rp++;
							clab[rp]=m1;rp++;
						}
						else
						{
						rp += 2;
						}
						
						continue;
				  case 17:	m1=dat[p+1];
						if (m1!='@')	goto sterr;
						p++;
						m1=dat[p+1];
						if (m1!='@')	goto sterr;
						p++;
						m1=dat[p+1];
						if ((dat[p+2]<'0') || (dat[p+2]>'9')) i=m1-'0'; else {i=(m1-'0')*10+(dat[p+2]-'0');p++;}
						if(rflag == 0)rdat[rp]=0xa1;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
						p++;
						m1=dat[p+1];
						if ((m1!=',') && (m1!=' ')) goto sterr;
						p++;
						m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=i;
						rp++;
						continue;
				  case 18:      m1=dat[p+1];
						if (m1!='@')	goto sterr;
						p++;
						m1=dat[p+1];
						if (m1!='@')	goto sterr;
						p++;
						m1=dat[p+1];
						if ((dat[p+2]<'0') || (dat[p+2]>'9')) i=m1-'0'; else {i=(m1-'0')*10+(dat[p+2]-'0');p++;}
						p++;
						if(rflag == 0)rdat[rp]=0xa2;
						rp++;
						if (rflag == 0)rdat[rp]=i;
						rp++;
						continue;
				  case 19:      m1=dat[p+1];
						if (m1!='@')	goto sterr;
						p++;
						m1=dat[p+1];
						if (m1!='@')	goto sterr;
						p++;
						m1=dat[p+1];
						if ((dat[p+2]<'0') || (dat[p+2]>'9')) i=m1-'0'; else {i=(m1-'0')*10+(dat[p+2]-'0');p++;}
						p++;
						if(rflag == 0)rdat[rp]=0xa3;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
						continue;

				  case 20:      m1=dat[p+1];
						if (m1!='@')	goto sterr;
						p++;
						m1=dat[p+1];
						if (m1!='@')	goto sterr;
						p++;
						m1=dat[p+1];
						if ((dat[p+2]<'0') || (dat[p+2]>'9')) i=m1-'0'; else {i=(m1-'0')*10+(dat[p+2]-'0');p++;}
						if(rflag == 0)rdat[rp]=0xa4;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
						p++;
                                                if ((dat[p+1]!=',') && (dat[p+1]!=' ')) goto sterr;
						p++;


						m1=dat[p+1];
						if (m1!='@')	goto sterr;
						p++;
						m1=dat[p+1];
						if (m1!='@') 	goto sterr;
						p++;
						m1=dat[p+1];
                                                if ((m1>='a') && (m1<='z')) m1-=('a'-'A');
						if ((m1<'A') || (m1>'Z')) goto sterr;
						p++;
						if (rflag == 0)
						{
							clab[rp]='@';rp++;
							clab[rp]=m1;rp++;
						}
						else
						{
						rp += 2;
						}
						
						continue;
				  case 21:      m1=dat[p+1];
						if (m1!='@')	goto sterr;
						p++;
						m1=dat[p+1];
						if (m1!='@')	goto sterr;
						p++;
						m1=dat[p+1];
						if ((dat[p+2]<'0') || (dat[p+2]>'9')) i=m1-'0'; else {i=(m1-'0')*10+(dat[p+2]-'0');p++;}
						if(rflag == 0)rdat[rp]=0xa5;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
						p++;

						if ((dat[p+1]!=',') && (dat[p+1]!=' ')) goto sterr;
						p++;
						m1=dat[p+1];
						if (m1!='@')	goto sterr;
						p++;
						m1=dat[p+1];
						if (m1!='@') 	goto sterr;
						p++;
						m1=dat[p+1];
                                                if ((m1>='a') && (m1<='z')) m1-=('a'-'A');
						if ((m1<'A') || (m1>'Z')) goto sterr;
						p++;
						if (rflag == 0)
						{
							clab[rp]='@';rp++;
							clab[rp]=m1;rp++;
						}
						else
						{
						rp += 2;
						}
						
						continue;
				  case 22:      if(rflag == 0)rdat[rp]=0x96;
						rp++;
						continue;

				  case 23:      m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=0x97;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
						continue;
				  case 24:	m1=dat[p+1];
						if ((m1!='0') && (m1!='1')) goto sterr;
						p++;
						if(rflag == 0)rdat[rp]=0x98;
						rp++;
						if(rflag == 0)rdat[rp]=m1-'0';
						rp++;
						continue;
				  case 25:      ssp:

						m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=0x99;
						rp++;
						if(rflag == 0)rdat[rp]=i % 256;
						rp++;
						if(rflag == 0)rdat[rp]=i / 256;
						rp++;
						continue;
				  case 26:      senv:

						m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=0x9a;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
						continue;
				  case 27:	if(rflag == 0)rdat[rp]=0x9b;
				  		rp++;
						continue;
				  case 28:	if(rflag == 0)rdat[rp]=0x9c;
				  		rp++;
						continue;
				  case 29:	
						m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=0x9d;
						rp++;
						if(rflag == 0)rdat[rp]=i % 256;
						rp++;
						if(rflag == 0)rdat[rp]=i / 256;
						rp++;
						continue;
				  case 30:

						m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
						if(rflag == 0)rdat[rp]=0x9e;
						rp++;
						if(rflag == 0)rdat[rp]=i;
						rp++;
						continue;

				 case 35:
				 		/*PCMスタート*/
				 		m1=dat[p+1];
						if ((m1<'0') || (m1>'9')) goto sterr;
                                                i=m1-'0';
						p++;m1=dat[p+1];
						 while ((m1>='0') && (m1<='9'))
							{i=i*10+m1-'0';p++;m1=dat[p+1];}
				 		
				 		if ((i < 0) || (i > 99))
				 		{
				 			/*番号が大きすぎる*/
				 			goto sterr;
				 		}
				 		
				 		if (rflag == 0) rdat[rp] = 0xae;
				 		rp++;
				 		if (rflag == 0) rdat[rp] = i;
				 		rp++;
				 		if (rflag == 0) rdat[rp] = i;
				 		rp++;
				 		if (rflag == 0) rdat[rp] = i;
				 		rp++;
				 		if (rflag == 0) rdat[rp] = i;
				 		rp++;
				 		
				 		continue;
				  case 36:
				  		/*PCMストップ*/
				  		if (rflag == 0) rdat[rp] = 0xaf;
				  		rp++;
						continue;
						
				  case 37:
				  		/*PCMストップ*/
				  		if (rflag == 0) rdat[rp] = 0xb0;
				  		rp++;
				  		continue;
				  case 38:
				  		/*YM2608 ONでジャンプ*/
				  		m1=dat[p+1];
						if (m1!='@')	goto sterr;
						p++;
						m1=dat[p+1];
						if (m1!='@') 	goto sterr;
						p++;
						m1=dat[p+1];
                        if ((m1>='a') && (m1<='z')) m1-=('a'-'A');
						if ((m1<'A') || (m1>'Z')) goto sterr;
						p++;
						if(rflag == 0)rdat[rp]=0xb1;
						rp++;
						if (rflag == 0)
						{
							clab[rp]='@';rp++;
							clab[rp]=m1;rp++;
						}
						else
						{
						rp += 2;
						}
						
						continue;
			
				  case 39:
				  		/*PCM機能 ONでジャンプ*/
				  		m1=dat[p+1];
						if (m1!='@')	goto sterr;
						p++;
						m1=dat[p+1];
						if (m1!='@') 	goto sterr;
						p++;
						m1=dat[p+1];
                        if ((m1>='a') && (m1<='z')) m1-=('a'-'A');
						if ((m1<'A') || (m1>'Z')) goto sterr;
						p++;
						if(rflag == 0)rdat[rp]=0xb2;
						rp++;
						if (rflag == 0)
						{
							clab[rp]='@';rp++;
							clab[rp]=m1;rp++;
						}
						else
						{
						rp += 2;
						}
						
						continue;
			  
				  }
			default:	continue;

				}

		mwork:
		 /*次の一文字を読む*/
		 m1=dat[p+1];
		 
		 if (m1 == '*')
		 {
		 	/*絶対音長*/
		 	zettai = 1;
		 	p++;
		 	m1 = dat[p + 1];
		 }
		 else
		 {
		 	zettai = 0;
		 }
		 
		 if ((m1>='0') && (m1<='9'))
			{
			ll=m1-'0';p++;
			m1=dat[p+1];
			while ((m1>='0') && (m1<='9'))
				{
				ll=ll*10+m1-'0';p++;m1=dat[p+1];
				}
			}
		
		 if (m1=='.') {fff=1.5;p++;}

		 /*書き込み*/
		 if (zettai == 0)
		 {
		 	i=(unsigned int)((4*L4*fff)/ll);
		 }
		 else
		 {
		 	i = ll * fff;
		 }

		 if (i<=127)

		 {
		 if(rflag == 0)rdat[rp]=(unsigned char)(i);
		 rp++;
		
		 if(rflag == 0)rdat[rp]=(unsigned char)((vv & 0x1f) + (swc << 5) + (LR << 6));
		 
		 rp++;
		 }

		 else
		 {
		 /*音長が１２７を超えているためタイの処理にする*/
		 if (tieflag==0) 
		 	{if(rflag == 0)rdat[rp]=0x81;
		 	rp++;}

		 do
		 {
		 if(rflag == 0)rdat[rp]=127;
		 rp++;
		 if(rflag == 0)rdat[rp]=(unsigned char)((vv & 0x1f) + (swc << 5) + (LR << 6));
		 
		 rp++;
		 i-=127;
		 }while (i>127);

		 if(rflag == 0)rdat[rp]=(unsigned char)(i);
		 rp++;
		 if(rflag == 0)rdat[rp]=(unsigned char)((vv & 0x1f) + (swc << 5) + (LR << 6));
		 
		 rp++;

		 if (tieflag==0) 
		 	{if(rflag == 0)rdat[rp]=0x80;
		  	rp++;}

		 }

		 continue;

		 leng:
		 /*次の一文字を読む*/
		 m1=dat[p+1];

		 if ((m1>='0') && (m1<='9'))
			{
			l=m1-'0';p++;
			m1=dat[p+1];
			while ((m1>='0') && (m1<='9'))
				{
				l=l*10+m1-'0';p++;m1=dat[p+1];
				}
			}
		 if (m1=='.') {ff=1.5;p++;}

		 continue;

		 }


	if(rflag == 0)rdat[rp]=0x8c;
	rp++;
	if(rflag == 0)rdat[rp]=0;
	rp++;
	
	/*ラベルの記入*/

	if (rflag == 0)
	{
	for (i=0;i<=rp-1;i++)
		{
		if (clab[i]!='@') continue;
		if (slabf[clab[i + 1] - 'A'][ch] == 0)
		{
			puterrno(NULL);
			printf("ラベル[@@%c]が定義されていません\n", clab[i + 1]);
			exit(1);
		}
	
		k=slab[clab[i+1]-'A'][ch]-(i + optbuf[ch]);
		if(rflag == 0)rdat[i]=k & 255;
		if(rflag == 0)rdat[i+1]=k >> 8;
		}
	free (clab);
	}
	
	clabmax = rp;
	
	return(rp);


	sterr:

	for (i = p - 20; i < p + 20; i++)
	{
		if (i == p - 1) printf("\x1b[32m");
		if (i >= 0) putchar(dat[i]);
		if ((dat[i] == '\0') && (i > p)) break;
		if (i == p) printf("\x1b[0m");
	}
	
	printf("MML文法ミスです\n");
	
	exit(1);
	return(p);
}

int	main(int argv, char ** argc)
{
	const unsigned int comnum[] =
		{
		0, 0, 2, 1, 1, 2, 1, 1, 4, 1, 1, 1, 0, 1, 2, 2,
		2, 2, 2, 2, 4, 4, 0, 1, 1, 2, 1, 0, 0, 2, 1, 6,
		2, 2, 1, 1, 3, 3, 1, 0, 3, 0, 0, 4, 4, 5, 4, 0,
		0, 2
		};
	const unsigned int comnum2[] =
		{
		0, 0, 2, 1, 1, 1, 1, 1, 4, 1, 1, 1, 0, 2, 2, 2,
		2, 2, 2, 2, 4, 4, 0, 1, 1, 2, 1, 0, 0, 2, 1, 6,
		2, 2, 1, 1, 3, 3, 2, 0, 1, 0, 0, 4, 4, 0, 4, 0,
		0, 2
		};
	unsigned char r4end[] =	
		{
		0x8c,0
		};
	
	
	FILE *fp, *fp2;
	char filename[FILENAME_MAX], filename2[FILENAME_MAX];
	unsigned char  *chk;
	int psg;
	int moji;
	int nestlevel = 1;
	int status;
	int eqn = -1;
	int opt = -1;
	unsigned int pt = 0;
	unsigned int buf_size;
	unsigned char data[128];
	
	char tok[512];	/*文字列トークン列*/
	unsigned char  *r_buf[15];
	unsigned char  *se_buf_1st[100];
	unsigned char  *se_buf_2nd[100];
	unsigned char  *s_data[128];
	unsigned int file_offset = 15 * 2;
	unsigned int s_offset[128];
	unsigned int d_offset[15];
	unsigned int se_offset_1st[100];
	unsigned int se_offset_2nd[100];
	unsigned char tr[29];
	unsigned long pcm_offset[100];
	unsigned long now_point;
	
	char  *pcm_filename[100];
	
	int i, j;
	char opnflag[15];
	int optflag = 0;
	int type = -1;
	int num = -1;
	int senum = -1;
	int se_exist = 0;
	int ch_exist = 0;
	int endflag = 0;
	int types[100];
	int nums[100];
	int filenum = 0;
	int f_type = 0;
	int optoff = 0;
	unsigned int pcm_ch;
	unsigned int  *pcm_sf;
	
	
	/*オプションチェック etc... */
	if (argv <= 1)
		{
		msg:
		
		puts ("MML2OPN  Ver 1.11 : MML形式ファイルをOPN/SE形式に変換します");
		puts ("                                    Programmed by F.Osawa");
		puts ("Usage:");
		puts ("     MML2OPN Sourcefilename [Destinationfilename] [Options]");
		puts ("Options:");
		puts ("     /o: OPN形式ファイルに変換します(Default)\n");
		puts ("     /s: SE形式ファイルに変換します\n");
		return(1);
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
		
		if (strstr (argc[i], "s") == argc[i])
			{
			f_type = 1;
			continue;
			}
		
		if (strstr (argc[i], "o") == argc[i])
			{
			f_type = 0;
			continue;
			}
		if ((strstr (argc[i], "h") == argc[i]) || (strstr (argc[i], "?") == argc[i])) goto msg;
	
		fprintf (stderr, "Incorrect option -[%s]", argc[i] - 1);
		return(1);
		}
	
	if (filenum == 1) 
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
	
	/*拡張子がなければMMLにする*/
	
	if (strchr(filename, '.') == NULL)
		strcat(filename, ".MML");
	
	if (strchr(filename2, '.') == NULL)
		{
		if (f_type == 0)
			strcat(filename2, ".OPN");
			else
			strcat(filename2, ".SE");
		}
	
	if ((fp = fopen(filename, "rt")) == NULL)
	{
		printf("ファイル[%s]が見つかりません\n", filename);
		exit(1);
	}
	
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
		printf("ファイル[%s]が開けません\n", filename2);
		exit(1);
	}
	
	for (i = 0; i < 15; i++)
	{
		optbuf[i] = 0;
	}
	
	for (i = 0; i < 15 * 2 + 7; i++)
		{
		fputc(0, fp2);	/*データ格納先のデータ分予約*/
		}
	
	for (i = 0; i < 100; i++)
		{
		se_buf_1st[i] = NULL;
		se_buf_2nd[i] = NULL;
		pcm_offset[i] = 0xffffffff;
		}
	
	for (i = 0; i < 128; i++)
	{
		s_offset[i] = 0xffff;
	}
	
	for (i = 0; i < 15; i++)
		{
		opnflag[i] = 0;
		}
		
	
	printf("[%s] → [%s] ... Converting", filename, filename2);
	
	for (;;)
	{
		status = gettok(tok, fp);
		if (status == -1) break;
		switch (tok[0])
			{
			case '\\':
					/*コマンド*/
					printf("\n Command [%3s]:", &tok[1]);
					
					if (strcmp(&tok[1], "division") == 0)
					{
						/*division*/
						opt = -1;
						eqn = 10;
						pt = 0;
						optflag = 0;
						break;
					}
					
					if (strcmp(&tok[1], "ch") == 0) 
					{
						/*チャンネル演奏データ*/
						optoff = 0;
					patch:
						ch_exist = 1;
						eqn = 1;
						opt = -1;
						if ((chk = malloc(2)) == NULL)
							{
							puterrno(filename);
							printf("キャラクタ用バッファが足りません\n");
							exit(1);
							}
						strcpy(chk, "");
						pt = 0;
						optflag = 0;
						break;
					}
					
					if (strcmp(&tok[1], "fm") == 0)
					{
						/*FM音源チャネル*/
						optoff = -1;
						goto patch;
					}
					
					if (strcmp(&tok[1], "psg") == 0)
					{
						/*PSG音源チャネル*/
						optoff = 5;
						goto patch;
					}
					
					if (strcmp(&tok[1], "rth") == 0)
					{
						/*RTH音源チャネル*/
						optoff = 8;
						goto patch;
					}
					
					if (strcmp(&tok[1], "soundopnfm") == 0)
					{
						/*opn形式音色データ*/
						eqn = 2;
						opt = -1;
						pt = 0;
						optflag = 0;
						break;
					}
					if (strcmp(&tok[1], "soundfm") == 0)
					{
						/*fm形式音色データ*/
						eqn = 3;
						opt = -1;
						pt = 0;
						optflag = 0;
						break;
					}
					if (strcmp(&tok[1], "soundpsg") == 0)
					{
						/*ssg形式音色データ*/
						eqn = 4;
						opt = -1;
						pt = 0;
						optflag = 0;
						break;
					}
					if (strcmp(&tok[1], "pcm") == 0)
					{
						/*PCMデータ*/
						eqn = 6;
						opt = -1;
						pt = 0;
						optflag = 0;
					
						if ((chk = malloc(2)) == NULL)
							{
							puterrno(filename);
							printf("キャラクタ用バッファが足りません\n");
							exit(1);
							}
						strcpy(chk, "");
						break;
					}
					
					if (strcmp(&tok[1], "se") == 0)
					{
						/*se演奏データ*/
						se_exist = 1;
						eqn = 5;
						opt = -1;
						pt = 0;
						type = num = -1;
						if ((chk = malloc(2)) == NULL)
							{
							puterrno(filename);
							printf("キャラクタ用バッファが足りません\n");
							exit(1);
							}
						strcpy(chk, "");
						optflag = 0;
						endflag = 0;
						senum ++;
						break;
					}
					/*Unknown Command error*/
					puterrno(filename);
					printf("コマンド[%s]が理解できません\n", tok);
					exit(1);
					break;
			case '[':
					/*オプション*/
					opt = atoi(&tok[1]);
					printf ("[%d]", opt);
					if (eqn == 10)
					{
						L4 = opt;
						optflag++;
						break;
					}
					
					if (eqn == 5)
						{
						switch (optflag)
							{
							case 0:	type = opt;
									types[senum] = type;
									break;
							case 1: num = opt;
									nums[senum] = num;
									break;
							default:
									/*エラー*/
									puterrno(filename);
									printf("オプション引数が多すぎます\n");
									exit(1);
							}
						}
						else
						{
						if (optflag != 0)
							{
							puterrno(filename);
							printf("オプション引数が多すぎます\n");
							exit(1);
							}
						}
					optflag++;
					break;
			case '{':	
					/*データスタート*/
					if ((eqn == 5) && (endflag != 0))
						{
						pt = 0;
						type = num = -1;
						if ((chk = malloc(2)) == NULL)
							{
							puterrno(filename);
							printf("キャラクタ用バッファが足りません\n");
							exit(1);
							}
						strcpy(chk, "");
						}
					break;
			case '}':
					/*データエンド*/
					switch (eqn)
						{
						case -1:		/*error*/
						case  0:		/*error*/
						 			exit(1);
						case  6:
										/*PCMファイル名*/
									if ((opt < 0) || (opt > 99))
									{
										puterrno(filename);
										printf("PCM番号が大きすぎます\n");
										exit(1);
									}
									
									if (pcm_offset[opt] == 0xffff)
									{
										puterrno(filename);
										printf("PCM番号[%d]はすでに定義されています\n", opt);
										exit(1);
									}
									if ((pcm_filename[opt] = malloc(strlen(chk) + 1)) == NULL)
									{
										puterrno(filename);
										printf("変換用テンポラリバッファが確保できませんでした\n");
										exit(1);
									}
									
									strcpy(pcm_filename[opt], chk);
									
									pcm_offset[opt] = 0;
									
									break;
									
						case  1:		/*CHコマンド*/
						 		opt += optoff;
						 		if ((opt >= 0) && (opt <= 5))
						 			{
									
									buf_size = trans(chk, NULL, 0, opt);
									if (opnflag[opt] == 0)
									{
										if ((r_buf[opt] = malloc(buf_size)) == NULL)
										{
										puterrno(filename);
										printf("変換用テンポラリバッファが確保できませんでした\n");
										exit(1);
										}
									trans(chk, r_buf[opt], 0, opt);
									optbuf[opt] = buf_size - 2;
									}
									else
									{
										if ((r_buf[opt] = realloc(r_buf[opt], optbuf[opt] + buf_size)) == NULL)
										{
										puterrno(filename);
										printf("変換用テンポラリバッファが確保できませんでした\n");
										exit(1);
										}
									trans(chk, &r_buf[opt][optbuf[opt]], 0, opt);
									optbuf[opt] += buf_size - 2;
									}
									
									opnflag[opt] = 1;
									free(chk);
									break;
									}

						 		if ((opt >= 9) && (opt <= 14))
						 			{
									buf_size = rth_trans(chk, NULL, 0, opt);
									if (opnflag[opt] == 0)
									{
										if ((r_buf[opt] = malloc(buf_size)) == NULL)
										{
										puterrno(filename);
										printf("変換用テンポラリバッファが確保できませんでした\n");
										exit(1);
										}
									rth_trans(chk, r_buf[opt], 0, opt);
									optbuf[opt] = buf_size - 2;
									}
									else
									{
										if ((r_buf[opt] = realloc(r_buf[opt], optbuf[opt] + buf_size)) == NULL)
										{
										puterrno(filename);
										printf("変換用テンポラリバッファが確保できませんでした\n");
										exit(1);
										}
									rth_trans(chk, &r_buf[opt][optbuf[opt]], 0, opt);
									optbuf[opt] += buf_size - 2;
									}
									
									opnflag[opt] = 1;
									free(chk);
									break;
									}

						 		if (((opt >= 6) && (opt <= 8)))
						 			{
									buf_size = trans(chk, NULL, 1, opt);
									
									if (opnflag[opt] == 0)
									{
										if ((r_buf[opt] = malloc(buf_size)) == NULL)
										{
										puterrno(filename);
										printf("変換用テンポラリバッファが確保できませんでした\n");
										exit(1);
										}
									trans(chk, r_buf[opt], 1, opt);
									optbuf[opt] = buf_size - 2;
									}
									else
									{
										if ((r_buf[opt] = realloc(r_buf[opt], optbuf[opt] + buf_size)) == NULL)
										{
										puterrno(filename);
										printf("変換用テンポラリバッファが確保できませんでした\n");
										exit(1);
										}
									trans(chk, &r_buf[opt][optbuf[opt]], 1, opt);
									optbuf[opt] += buf_size - 2;
									}
									
									opnflag[opt] = 1;
									free(chk);
									break;
									}
									
								
								if ((opt < 0) || (opt > 14))
									{
									puterrno(filename);
									printf("チャンネル番号が大きすぎます\n");
									exit(1);
									}
								break;
						case 5:	/*SE形式*/
								if (endflag == 0)
									{
									if (types[senum] & 1)
									{
										buf_size = trans(chk, NULL, 2, 0);
										if ((se_buf_1st[senum] = malloc(buf_size)) == NULL)
											{
											puterrno(filename);
											printf("変換用テンポラリバッファが確保できませんでした\n");
											exit(1);
											}
										trans(chk, se_buf_1st[senum], 2, 0);
										free(chk);
									}
									else
									{
										buf_size = trans(chk, NULL, 2, 0);
										if ((se_buf_2nd[senum] = malloc(buf_size)) == NULL)
											{
											puterrno(filename);
											printf("変換用テンポラリバッファが確保できませんでした\n");
											exit(1);
											}
										trans(chk, se_buf_2nd[senum], 2, 0);
										free(chk);
									}
									}
									else
									{
									buf_size = trans(chk, NULL, 2, 0);
									if ((se_buf_2nd[senum] = malloc(buf_size)) == NULL)
										{
										puterrno(filename);
										printf("変換用テンポラリバッファが確保できませんでした\n");
										exit(1);
										}
									trans(chk, se_buf_2nd[senum], 2, 0);
									free(chk);
									}
								endflag ++;
								break;
								
						case 3:		/*FM形式の場合コンバートが必要*/
								if (pt != 37)
									{
									puterrno(filename);
									printf("音色データの数が一致しません(37個にしてください)\n");
									exit(1);
									}
								soundtrans(data, tr);
								memcpy(data, tr, 29);
								pt = 29;
								
						case 2:		/*OPN形式*/
								if (pt != 29)
									{
									puterrno(filename);
									printf("音色データの数が一致しません(29個にしてください)\n");
									exit(1);
									}
						case 4:	/*SSG形式*/
								if ((opt < 0) || (opt > 127))
									{
									puterrno(filename);
									printf("音色番号[%d]が大きすぎます\n",opt);
									exit(1);
									}
								
								if (s_offset[opt] != 0xffff)
									{
									puterrno(filename);
									printf("音色番号[%d]が2重定義されています\n", opt);
									exit(1);
									}
								s_offset[opt] = pt;
								if ((s_data[opt] = malloc(pt)) == NULL)
									{
									puterrno(filename);
									printf("音色データ定義用バッファが確保できませんでした\n");
									exit(1);
									}
								memcpy(s_data[opt], data, pt);
								file_offset += pt;
								/*PSG音色の場合は、ホールド状態にする*/
								if (eqn == 4)
								{
									s_data[opt][pt - 1] |= 0x80;
								}
								break;
						}
					if (eqn != 5) eqn = -1;
					opt = -1;
					break;
			default:
					/*データ解析*/
					printf (".");
					switch (eqn)
						{
						case -1:		/*error*/
						case  0:
								puterrno(filename);
								printf("データの始まりが不明です({と}の対応が合っているか確認してください\n");
								exit(1);
								break;
						 
						case 5:		/*SE形式データ*/
						case 1:		/*CHコマンド*/
						 		if ((chk = realloc(chk, strlen(chk) + strlen(tok) + 2)) == NULL)
						 			{
									puterrno(filename);
									printf("変換用テンポラリバッファが確保できませんでした\n");
									exit(1);
									}
								strcat(chk, " ");
								strcat(chk, tok);
								break;
						case 6:
								/*PCM形式データ*/
								if ((chk = realloc(chk, strlen(chk) + strlen(tok) + 1)) == NULL)
									{
									puterrno(filename);
									printf("変換用テンポラリバッファが確保できませんでした\n");
									exit(1);
									}
								strcat(chk, tok);
								break;
						case 2:		/*OPN形式データ*/
						case 3:		/*FM形式データ*/
						case 4:		/*SSG形式データ*/
						 		if (tok[0] == ',')
						 			break;
						 		data[pt++] = atoi(tok);
						 		break;
						}
					break;
			}
	}
	
	/*データの書き込み*/
	
	if ((ch_exist == 1) && (se_exist == 1))
		{
		puterrno(filename);
		printf("演奏データと効果音データは、同居できません\n");
		exit(1);
		}
	
	if (ch_exist == 1)
		{
		/*OPNファイルの作成*/
		/*音色データ書き込み*/
		if (f_type != 0)
			{
			puterrno(filename);
			printf("このファイルは、演奏データのみを含むので、OPN形式にしか変換できません(-oオプションを付けてもう一度実行してみてください)\n");
			exit(1);
			}
		printf("\n\nOPN Type File Writing ...\n");
		
		
		file_offset = 15 * 2 + 7;
		for (i = 0; i < 128; i++)
			{
			if (s_offset[i] != 0xffff)
				{
				pt = s_offset[i];
				fwrite(s_data[i], 1, pt, fp2);
				s_offset[i] = file_offset;
				file_offset += pt;
				printf(" Sound[%d]: offset %u\n", i, file_offset);
				}
			}
		
		for (i = 0; i < 15; i++)
			{
			if (opnflag[i] == 0)
				{
				if (i < 6)
				{
					printf("演奏データ\\fm[%d]が定義されていないので、休符に変換します\n", i + 1);
				}
				else
				{
				if (i < 9)
				{
					printf("演奏データ\\psg[%d]が定義されていないので、休符に変換します\n", i + 1 - 6);
				}
				else
				{
					printf("演奏データ\\rth[%d]が定義されていないので、休符に変換します\n", i + 1 - 9);
				}
				}
				r_buf[i] = &r4end[0];
				}
			}
		
		/*音色データ部のオフセット調節*/
		pcm_ch = 0;
		pcm_sf = NULL;
		
		for (i = 0; i < 15; i++)
			{
			
			d_offset[i] = file_offset;
			
			#if 0
			printf(" Data[%d]:  offset %u\n", i, file_offset);
			#endif 
			
			pt = 0;
			
			while (r_buf[i][pt] != 0)
				{
				if (r_buf[i][pt] == 0xae)
				{
					/*PCM音色データオフセットの保存*/
					if ((pcm_sf = realloc(pcm_sf, (pcm_ch + 1) * 2 * 2)) == NULL)
					{
						printf("PCM音色データ保存バッファがオーバーしました\n");
						exit(1);
					}
					
					pcm_sf[pcm_ch * 2] = file_offset + pt + 1;
					pcm_sf[pcm_ch * 2 + 1] = r_buf[i][pt + 1];
					pcm_ch++;
				}
				
					
				if (r_buf[i][pt] == 0x82)
					{
					unsigned int dw;
					/*音色データオフセットの調整*/
					pt++;
					dw = r_buf[i][pt] + r_buf[i][pt + 1]*256;
					
					if (s_offset[dw] == 0xffff)
						{
						/*音色データが定義されていない*/
						puterrno(filename);
						printf("音色データ[%d]が定義されていません", dw);
						exit(1);
						}
					
					r_buf[i][pt - 1] = 0xa7;	/*新コマンド (相対アドレス音色設定)*/
					dw = s_offset[dw];
					dw -= (file_offset + pt);
					r_buf[i][pt] = dw & 0xff;
					r_buf[i][pt + 1] = dw >> 8;
					pt += 2;
					continue;
					}
				if (r_buf[i][pt] < 0x80)
					pt += 2;
					else
					{
					if (i < 6)
						{
						pt += comnum[r_buf[i][pt] - 0x80] + 1;
						}
						else
						{
						pt += comnum2[r_buf[i][pt] - 0x80] + 1;
						}
					}
				}
			fwrite(r_buf[i], 1, pt, fp2);
			file_offset += pt;
			}
		
		
		/*現在のファイルポインタを保存*/
		now_point=ftell(fp2);
		if (now_point >= 65535L)
		{
			printf("曲データが64Kバイトを越えてしまいました\n");
			exit(1);
		}
		
		/*PCMデータの書き出し*/
		for (i = 0; i < 100; i++)
		{
			if (pcm_offset[i] != 0xffffffff)
			{
				/*書き出し*/
				FILE *rfp;
				int pcmver;
				
				if ((rfp = fopen(pcm_filename[i], "rb"))== NULL)
				{
					puterrno(filename);
					printf("PCMファイル[%s]が見つかりません\n", pcm_filename[i]);
					exit(1);
				}
				
				pcm_offset[i] = ftell(fp2);
				
				if ((fgetc(rfp) != 'P') || (fgetc(rfp) != 'C') || (fgetc(rfp) != 'M'))
				{
					puterrno(filename);
					printf("ファイル[%s]はPCMファイルではありません\n", pcm_filename[i]);
					exit(1);
				}
				
				pcmver = fgetc(rfp) * 100 + fgetc(rfp);
				if (pcmver < opnver)
				{
				
					puterrno(filename);
					printf("ファイル[%s]のバージョンが古すぎます\n", pcm_filename[i]);
					exit(1);
				}
				
				while (!feof(rfp))
				{
					fputc(fgetc(rfp), fp2);
				}
				fclose(rfp);
			}
		}
		
		/*PCMデータアドレス設定*/
		for (i = 0; i < pcm_ch; i++)
		{
			int pcmno;
			unsigned long jmp;
			unsigned int seg, off;
			
			fseek(fp2, pcm_sf[i * 2], SEEK_SET);
			
			pcmno = pcm_sf[i * 2 + 1];
			
			jmp = pcm_offset[pcmno];
			jmp = jmp - pcm_sf[i * 2];
			
			/*jmpをセグメントと、オフセットに分ける*/
			seg = jmp >> 4;
			off = jmp & 0xf;
			
			fseek(fp2, pcm_sf[i * 2], SEEK_SET);
			
			putword(off, fp2);
			putword(seg, fp2);
		}
		
		
		fseek(fp2, 0, SEEK_SET);
		
		/*OPN ID書き込み*/
		fputs("OPN", fp2);
		/*バージョン整数部*/
		fputc(opnver / 100, fp2);
		/*バージョン小数部*/
		fputc(opnver % 100, fp2);
		
		/*division*/
		fputc(L4 & 0xff, fp2);
		fputc(L4 >> 8, fp2);
		
		for (i = 0; i < 3; i++)
			{
			fputc((d_offset[i] - 7) % 256, fp2);
			fputc((d_offset[i] - 7) / 256, fp2);
			}
		/*PSG部とFM部のオフセットを変更したいため、変な処理になっている*/
		for (i = 6; i < 9; i++)
		{
			fputc((d_offset[i] - 7) % 256, fp2);
			fputc((d_offset[i] - 7) / 256, fp2);
		}
		for (i = 3; i < 6; i++)
		{
			fputc((d_offset[i] - 7) % 256, fp2);
			fputc((d_offset[i] - 7) / 256, fp2);
		}
		
		for (i = 9; i < 15; i++)
		{
			fputc((d_offset[i] - 7) % 256, fp2);
			fputc((d_offset[i] - 7) / 256, fp2);
		}
		
		}
		else
		{
		/*SE形式ファイル*/
		/*全部で、SEデータは、senumだけ存在する*/
		/*よってその分 5[byte/1data] * senum[data] [byte]だけ確保する*/
		
		if (f_type == 0)
			{
			puterrno(filename);
			printf("このファイルは、効果音データのみを含むので、SE形式にしか変換できません(-sオプションを付けてもう一度実行してみてください)\n");
			exit(1);
			}
		senum ++;
		printf("\n\nOPN Type File Writing ...\n");
		
		
		file_offset = 5U * senum + 1 + 5;	/*@@@*/
		
		fseek(fp2, file_offset, SEEK_SET);
		
		/*音色データを書き出す*/
		
		for (i = 0; i < 128; i++)
			{
			if (s_offset[i] != 0xffff)
				{
				pt = s_offset[i];
				fwrite(s_data[i], 1, pt, fp2);
				s_offset[i] = file_offset;
				file_offset += pt;
				printf(" Sound[%d]: offset %u\n", i, s_offset[i]);
				}
			}
		
		/*SEデータを書き出す*/
		pcm_ch = 0;
		pcm_sf = NULL;
		
		for (i = 0; i < senum; i++)
			{
			unsigned char *rb;
			
			for (j = 0; j < 2; j++)
				{
				if (j == 0) 
					{
					se_offset_1st[i] = file_offset; 
					printf(" Data[%d,%d]:  offset %u\n", i, j, se_offset_1st[i]);
					if (se_buf_1st[i] == NULL) continue;
					rb = se_buf_1st[i];
					}
					else 
					{
					se_offset_2nd[i] = file_offset;
					printf(" Data[%d,%d]:  offset %u\n", i, j, se_offset_2nd[i]);
					if (se_buf_2nd[i] == NULL) continue;
					rb = se_buf_2nd[i];
					}
				pt = 0;
			
				while (rb[pt] != 0)
					{
					if (rb[pt] == 0xae)
					{
						/*PCM音色データオフセットの保存*/
						if ((pcm_sf = realloc(pcm_sf, (pcm_ch + 1) * 2 * 2)) == NULL)
						{
							printf("PCM音色データ保存バッファがオーバーしました\n");	
							exit(1);
						}
					
						pcm_sf[pcm_ch * 2] = file_offset + pt + 1;
						pcm_sf[pcm_ch * 2 + 1] = rb[pt + 1];
						pcm_ch++;
					}
			
					if (rb[pt] == 0x82)
						{
						unsigned int dw;
						/*音色データオフセットの調整*/
						pt++;
						dw = rb[pt] + rb[pt + 1]*256;
						
						if (s_offset[dw] == 0xffff)
							{
							/*音色データが定義されていない*/
							puterrno(filename);
							printf("音色データ[%d]が定義されていません", dw);
							exit(1);
							}
					
						rb[pt - 1] = 0xa7;	/*新コマンド (相対アドレス音色設定)*/
						dw = s_offset[dw];
						dw -= (file_offset + pt);
						rb[pt] = dw & 0xff;
						rb[pt + 1] = dw >> 8;
						pt += 2;
						continue;
						}
					if (rb[pt] < 0x80)
						pt += 2;
						else
						{
						pt += comnum2[rb[pt] - 0x80] + 1;
						}
					}
				fwrite(rb, 1, pt, fp2);
				file_offset += pt;
				}
			}
		
		/*PCMデータの書き出し*/
		for (i = 0; i < 100; i++)
		{
			if (pcm_offset[i] != 0xffffffff)
			{
				/*書き出し*/
				FILE *rfp;
				int pcmver;
				
				if ((rfp = fopen(pcm_filename[i], "rb"))== NULL)
				{
					puterrno(filename);
					printf("PCMファイル[%s]が見つかりません\n", pcm_filename[i]);
					exit(1);
				}
				
				pcm_offset[i] = ftell(fp2);
				
				if ((fgetc(rfp) != 'P') || (fgetc(rfp) != 'C') || (fgetc(rfp) != 'M'))
				{
					puterrno(filename);
					printf("ファイル[%s]はPCMファイルではありません\n", pcm_filename[i]);
					exit(1);
				}
				
				pcmver = fgetc(rfp) * 100 + fgetc(rfp);
				if (pcmver < opnver)
				{
				
					puterrno(filename);
					printf("ファイル[%s]のバージョンが古すぎます\n", pcm_filename[i]);
					exit(1);
				}
				
				while (!feof(rfp))
				{
					fputc(fgetc(rfp), fp2);
				}
				fclose(rfp);
			}
		}
		
		/*PCMデータアドレス設定*/
		for (i = 0; i < pcm_ch; i++)
		{
			int pcmno;
			unsigned long jmp;
			unsigned int seg, off;
			
			fseek(fp2, pcm_sf[i * 2], SEEK_SET);
			
			pcmno = pcm_sf[i * 2 + 1];
			
			jmp = pcm_offset[pcmno];
			jmp = jmp - pcm_sf[i * 2];
			
			/*jmpをセグメントと、オフセットに分ける*/
			seg = jmp >> 4;
			off = jmp & 0xf;
			
			fseek(fp2, pcm_sf[i * 2], SEEK_SET);
			
			putword(off, fp2);
			putword(seg, fp2);
		}
		
		
		/*ヘッダの書き込み*/
		fseek(fp2, 0, SEEK_SET);
		
		/*SE ID書き込み*/
		fputs("SE ", fp2);
		/*バージョン整数部*/
		fputc(opnver / 100, fp2);
		/*バージョン小数部*/
		fputc(opnver % 100, fp2);
		
		fputc(senum, fp2);
		
		for (i = 0; i < senum; i++)
			{
			int d;
			if ((types[i] < 1) || (types[i] > 3))
				{
				puterrno(filename);
				printf("SE[%d]のタイプが不正です\n", i);
				exit(1);
				}
			if ((nums[i] < 1) || (nums[i] > 63))
				{
				puterrno(filename);
				printf("SE[%d]の優先順位が不正です\n", i);
				exit(1);
				}
			switch (types[i])
				{
				case 1:
						d = 0x80;
						break;
				case 2:
						d = 0x40;
						break;
				case 3:
						d = 0xc0;
						break;
				}
			d += nums[i];
			fputc(d, fp2);
			
			fputc(se_offset_1st[i] % 256, fp2);
			fputc(se_offset_1st[i] / 256, fp2);
			
			fputc(se_offset_2nd[i] % 256, fp2);
			fputc(se_offset_2nd[i] / 256, fp2);
			}
		
		
		}
	
	fclose(fp);
	fclose(fp2);
	
	printf("\n...Converted\n");
	return(0);
}
