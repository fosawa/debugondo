	/*OPNDRV Ver2.0
	
		C言語用ライブラリ
		ヘッダファイル
		(Borland C++ Ver3.0)
		
						Programmed by F.Osawa
	*/
	
int opncheck(void);
void mstart (unsigned char far *addr);
int getdivision(unsigned char far *dat);
int getopnver(unsigned char far *dat);
int opnstart(unsigned char far *dat);
int opnstart2(unsigned char far *dat, unsigned long length);
void mend();
void mfade(int fmspeed, int psgspeed);
unsigned char mread(unsigned char ch, unsigned char workno);
int varread(int val);
void mescon();
void mescof();
void minton(unsigned char far *addr, unsigned char ch);
void sinton(unsigned char ch,unsigned char inum,...);
void sstart();
void send();
void tempochange(int tempo, int division);
int regread(int regno, int sel);
int getsemax(unsigned char far *sdata);
int getsever(unsigned char far *sdata);
int sestart(unsigned char far *sdata, int snum);
int sestore(unsigned char far *sdata, unsigned long length);
void sestart2(int snum);
int midiinit (int interface);
int midistart (void huge *addr);
int midistart2 (void huge *addr, unsigned long length);
void midistop(void);
void midicont(void);
void miditempo(int tempo);
void midiclose(void);
void midiext(void);
void midisend(unsigned char data);
int getmidi(void);
void midifade(unsigned char start, unsigned char end, unsigned int speed);
int pcmplay(unsigned char huge *dat);
int pcmplay2(unsigned char huge *dat, unsigned long length);
int getpcmver(unsigned char far *dat);
void pcmstop(void);
int getstatus(void);

