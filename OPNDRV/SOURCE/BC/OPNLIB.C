	/*OPNDRV Ver2.0
	
		C����p���C�u����
		(Borland C++ Ver3.0)
		
						Programmed by F.Osawa
	*/
	
	#include <stdio.h>
	#include <dos.h>
	#include <io.h>
	#include <stdlib.h>
	#include <string.h>
	#include <stdarg.h>
	
	#define VECT 0x41		/*�n�o�m�c�q�u�̃x�N�^*/
	#define L64 3


	/*���ʕ�*/

int opncheck()
{
	/*OPNDRV Version ��100�{�̒l��Ԃ��܂�*/
	int i, ver;
	const char ID[]="VRDNPO";
	char far *vect;
	
	vect = (char far *)getvect(VECT);
	
	for (i = 0; i <= 5; i++)
	{
		vect--;
		if (vect[0] != ID[i]) return 0;		/*OPNDRV���Ȃ�*/
	}
	
	ver = (vect[-1] - 0x30) + (vect[-2] - 0x30) * 10 + (vect[-4] - 0x30) * 100;
	
	return ver;
}

	/*FM������*/

void mstart (unsigned char far *addr)
{
	/*FM�������t�X�^�[�g*/
	/*addr[0]�`[15]�́A���t�f�[�^�������|�C���^���i�[����Ă���*/
	
	struct REGPACK rg;
	
	rg.r_ax=0;
	rg.r_bx=FP_OFF(addr);
	rg.r_es=FP_SEG(addr);

	intr (VECT,&rg);
}

int getdivision(unsigned char far *dat)
{
	/*OPN�t�@�C����division��Ԃ�*/
	/* 0�ŗ^����ꂽ�f�[�^��OPN�`���łȂ����Ƃ����� */
	
	int rcode;
	
	/*ID�`�F�b�N*/
	if ((dat[0] != 'O') || (dat[1] != 'P') || (dat[2] != 'N'))
	{
		return 0;
	}
	
	/*division�𓾂�*/
	rcode = dat[5] + (dat[6] << 8);
	
	return rcode;
}

int getopnver(unsigned char far *dat)
{
	/*OPN�t�@�C���̃o�[�W������Ԃ�*/
	/* 0�ŗ^����ꂽ�f�[�^��OPN�`���łȂ����Ƃ����� */
	/* �����łȂ���΁A�f�[�^�̃o�[�W������100�{��Ԃ�*/
	int rcode;
	
	/*ID�`�F�b�N*/
	if ((dat[0] != 'O') || (dat[1] != 'P') || (dat[2] != 'N'))
	{
		return 0;
	}
	
	/*�o�[�W�����𓾂�*/
	rcode = dat[3] * 100 + dat[4];
	
	return rcode;
}

int opnstart2(unsigned char far *dat, unsigned long length)
{
	struct REGPACK rg;
	
	/*ID�`�F�b�N*/
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
	/*OPN�`���t�@�C���̉��t���X�^�[�g�����܂�*/
	/* dat�ɂ́AOPN�`���t�@�C����ǂݍ��񂾐擪�|�C���^�����ăR�[�����Ă�������*/
	/* 0�ŗ^����ꂽ�f�[�^��OPN�`���łȂ����Ƃ����� */
	/* �����łȂ���΁A�f�[�^�̃o�[�W������100�{��Ԃ�*/
	
	
	struct REGPACK rg;
	int rcode;
	
	/*ID�`�F�b�N*/
	if ((dat[0] != 'O') || (dat[1] != 'P') || (dat[2] != 'N'))
	{
		return 0;
	}
	
	/*�o�[�W�����𓾂�*/
	rcode = dat[3] * 100 + dat[4];
	
	rg.r_ax = 0;
	rg.r_bx = FP_OFF(&dat[7]);
	rg.r_es = FP_SEG(&dat[7]);

	intr(VECT,&rg);
	
	return rcode;
}

void mend()
{
	/*mstart�֐��ŊJ�n�������t���I������*/
	
	struct REGPACK rg;

	rg.r_ax = 0x100;
	intr(VECT,&rg);
	return;
}

void mfade(int fmspeed, int psgspeed)
{
	/*�t�F�[�h�A�E�g����*/
	/*fmspeed=FM���������RTH�����t�F�[�h�A�E�g�X�s�[�h*/
	/*psgspeed=PSG�����t�F�[�h�A�E�g�X�s�[�h*/
	
	struct REGPACK rg;
	
	rg.r_ax = 0x600;
	rg.r_bx = fmspeed * 256 + psgspeed;
	intr (VECT,&rg);

	return;
}

unsigned char mread (unsigned char ch, unsigned char workno)
{
	/*���[�N�G���A�̓��e�𓾂�*/
	/*			ch=�`���l��  (1-17(16,17�͊��荞�݃`���l��))
				workno=���[�N�ԍ�
	*/
	
	struct REGPACK rg;
	rg.r_ax = 0x800;
	rg.r_dx = workno*256+ch;
	intr (VECT, &rg);
	return (unsigned char)(rg.r_ax / 256);
}

int varread(int val)
{
	/*�����ϐ��l�𓾂�*/
	
	
	struct REGPACK rg;
	rg.r_ax = 50 * 256;
	rg.r_dx = val;
	intr (VECT, &rg);
	return rg.r_ax & 255;
}
	
void mescon()
{
	/*���t����FM���������ꎞ��~���܂�*/
	struct REGPACK rg;

	rg.r_ax = 0x300;
	intr(VECT, &rg);
}

void mescof()
{
	/*mescon�֐��Œ�~�������̂��ĊJ���܂�*/
	struct REGPACK rg;

	rg.r_ax = 0x400;
	intr(VECT, &rg);
}

void minton(unsigned char far *addr, unsigned char ch)
{
	/*���荞�݉��t���J�n���܂�*/
	/*addr = ���荞�݃f�[�^�擪�A�h���X*/
	/*ch = �`���l��*/
	struct REGPACK rg;

	rg.r_ax = 13 * 256;
	rg.r_dx = ch;
	rg.r_cx = FP_OFF(addr);
	rg.r_es = FP_SEG(addr);
	intr(VECT, &rg);

}

void sinton (unsigned char ch,unsigned char inum,...)
{
	/*�G�L�X�p���_����̊��荞�ݐݒ�*/
	/*ch=�`���l��(1=PSG2CH, 2=PSG3CH, 3=PSG2CH&3CH)*/
	/*inum = ���荞�ݗD�揇��*/
	/*���̌�A�f�[�^�擪�A�h���X�������|�C���^���Â�(�ϒ�)*/
	
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
	/*�G�L�X�p���_on*/
	struct REGPACK rg;

	rg.r_ax=0x900;
	intr (VECT,&rg);
}

void send()
{
	/*�G�L�X�p���_off*/
	struct REGPACK rg;

	rg.r_ax=0xa00;
	intr (VECT,&rg);

}

void tempochange(int tempo, int division)
{
	/*FM�������t���̃e���|��ύX����*/
	/*�������A���̃f�[�^���Ńe���|�ݒ�R�}���h�ɂԂ���ƁA�ݒ�͖����ɂȂ�*/
	
	struct REGPACK rg;
	unsigned int tm;
	
	tm = 256-208200/(division*tempo);
	
	rg.r_ax = 15 * 256;
	rg.r_bx = tm;
	
	intr (VECT,&rg);
}

int regread(int regno, int sel)
{
	/*FM�����ɑ��������W�X�^�̓��e�𒲂ׂ�*/
	/*regno = ���W�X�^�ԍ�*/
	/*sel = ���W�X�^�Z���N�^(0 = YM2203�݊������A 1 = YM2608�g������)*/
	struct REGPACK rg;
	
	rg.r_ax = 16 * 256;
	rg.r_dx = sel * 256 + regno;
	
	intr (VECT,&rg);

	return rg.r_ax & 0xff;
}
	
	/*SE��*/
	
int getsemax(unsigned char far *sdata)
{
	/*SE�t�@�C�����ǂ������ׂ�*/
	/*SE�t�@�C���Ȃ�Ίi�[����Ă���SE�i���o�[�ő�l���A�����łȂ����0��Ԃ�*/
	
	int rcode;
	
	if ((sdata[0] != 'S') || (sdata[1] != 'E') || (sdata[2] != ' '))
	{
		return 0;
	}
	
	return sdata[5];
}

int getsever(unsigned char far *sdata)
{
	/*SE�t�@�C���̃o�[�W������Ԃ�*/
	/*SE�f�[�^�łȂ����0���A�����łȂ���΃f�[�^�o�[�W������100�{�̒l��Ԃ�*/
	
	int rcode;
	
	/*ID�`�F�b�N*/
	if ((sdata[0] != 'S') || (sdata[1] != 'E') || (sdata[2] != ' '))
	{
		return 0;
	}
	
	/*�o�[�W�����𓾂�*/
	rcode = sdata[3] * 100 + sdata[4];
	
	
	return rcode;
}

void sestart2(int num)
{
	/*�X�g�A����SE��炷*/
	struct REGPACK rg;
	
	rg.r_ax = 2 * 256;
	rg.r_bx = num;
	
	intr (VECT,&rg);
	
}

int sestore(unsigned char far *sdata, unsigned long length)
{
	struct REGPACK rg;
	
	/*SE���풓������*/
	
	/*ID�`�F�b�N*/
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
	/*���ʉ���炷*/
	/*FM�����������t���ł��邱�Ƃ��K�v*/
	/*sdata = SE�t�@�C����ǂݍ��񂾐擪�|�C���^, snum = �o������SE�ԍ�*/
	/*SE�f�[�^�łȂ����0���A�����łȂ���΃f�[�^�o�[�W������100�{�̒l��Ԃ�*/
	/*snum�������������-1��Ԃ�*/
	
	struct REGPACK rg;
	unsigned int offset;
	unsigned int addr;
	unsigned char bf[4];
	int i;
	int rcode;
	
	offset = FP_OFF(sdata);
	
	
	/*ID�`�F�b�N*/
	if ((sdata[0] != 'S') || (sdata[1] != 'E') || (sdata[2] != ' '))
	{
		return 0;
	}
	
	/*�o�[�W�����𓾂�*/
	rcode = sdata[3] * 100 + sdata[4];
	
	if (sdata[5] <= snum)
	{
		return -1;
	}
	
	/*�I�t�Z�b�g�␳*/
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
	
	/*�I�t�Z�b�g�␳��߂�*/
	
	for (i = 0; i < 4; i++)
	{
		sdata[snum * 5 + i + 2 + 5] = bf[i];
	}

	return rcode;
}
	
	/*MIDI��*/
	
int midiinit (int interface)
{
	/*MIDI�C���^�[�t�F�[�X�̏�����*/
	/*interfase = 1:MPU-401, 2=RS-232C*/
	/*�߂�l: 0:����I��, 0�ȊO:���̃C���^�[�t�F�[�X�͎g���Ȃ�*/
	/*�������ARS-232C�C���^�[�t�F�[�X���w�肵�����ɂ́A���0�̐���I�����߂邱�Ƃɒ���*/
	
	struct REGPACK rg;

	rg.r_ax = 20 * 256;
	rg.r_dx = interface;
	
	intr (VECT, &rg);

	return rg.r_ax;
}

int midistart (void huge *addr)
{
	/*MIDI���t���X�^�[�g������*/
	/*addr = �W��MIDI�t�@�C���f�[�^�̓����Ă���擪�|�C���^*/
	/*�߂�l: 0=����I���A0�ȊO=�W��MIDI�t�@�C���łȂ�*/
	
	struct REGPACK rg;
	
	rg.r_ax = 22 * 256;
	rg.r_bx = FP_OFF(addr);
	rg.r_es = FP_SEG(addr);
	
	intr (VECT, &rg);
	
	return rg.r_ax;
}

int midistart2 (void huge *addr, unsigned long length)
{
	/*MIDI���t���X�^�[�g������*/
	/*addr = �W��MIDI�t�@�C���f�[�^�̓����Ă���擪�|�C���^*/
	/*�߂�l: 0=����I���A1=�W��MIDI�t�@�C���łȂ�*/
	/*        -1=�o�b�t�@������Ȃ�*/
	
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
	/*midistart�ŊJ�n������MIDI���t���~�߂�(�ꎞ��~)*/
	struct REGPACK rg;
	
	rg.r_ax = 23*256;
	intr (VECT, &rg);
	return;
}

void midicont(void)
{
	/*midistop�Œ�~���������t���ēx���t�J�n�ɂ���*/
	struct REGPACK rg;
	
	rg.r_ax = 24 * 256;
	intr (VECT, &rg);
	return;
}

void miditempo(int tempo)
{
	/*MIDI�e���|�l�␳�̐ݒ�*/
	/*�e���|��W���e���|��tempo/256.0�{�ɂ���*/
	
	struct REGPACK rg;
	
	rg.r_ax = 27 * 256;
	rg.r_dx = tempo;
	
	intr (VECT, &rg);
	
}

void midiclose(void)
{
	/*MIDI�@�\���X�g�b�v������*/
	struct REGPACK rg;
	
	rg.r_ax = 21 * 256;
	intr (VECT, &rg);
	
}

void midiext(void)
{
	/*MIDI�g�����^�C�x���g����*/
	/*���̃t�@���N�V�������Ă񂾂��ƁA
	
		ff, 07, 04, 'M', 'a', 'r', 'k'���f�[�^���ɏo�Ă����炻�̈ʒu���}�[�N
		ff, 07, 04, 'J', 'u', 'm', 'p'�ł��̃}�[�N�����ʒu�ɃW�����v
		
	�@�Ƃ����A�W��MIDI�t�@�C���𖳎������C�x���g���������
	*/
	
	struct REGPACK rg;
	
	rg.r_ax = 29 * 256;
	intr (VECT, &rg);
	
}

void midisend(unsigned char data)
{
	/* MIDI��1�o�C�g�f�[�^�𑗂� */
	struct REGPACK rg;
	
	rg.r_ax = 30*256;
	rg.r_dx = data;
	intr (VECT, &rg);
}

int getmidi(void)
{
	/*MIDI�X�e�[�^�X������*/
	/*�߂�l:
		����8bit : 0=���t���A1=�ҋ@��(=��~�R�}���h�őҋ@��)�A2=���t�I��(=�f�[�^�����ׂđ���I�����)
		���8bit : �x���V�e�B�[�␳�l
	*/
	
	struct REGPACK rg;
	
	rg.r_ax = 26 * 256;
	intr (VECT, &rg);
	return rg.r_ax;
}

void midifade(unsigned char start, unsigned char end, unsigned int speed)
{
	/*MIDI �t�F�[�h�A�E�g*/
	/*start = �͂��߂̃x���V�e�B�[�l
	  end = �I���̃x���V�e�B�[�l
	  speed = �ω����x(�P�ʂ�gatetime)
	  
	  �t�@���N�V�����Ăяo����A
	  ����v���Av�~start/128�{��v�~end/128�{�ɁAspeed�̎��ԓ��Œ����I�ɕω�����
	*/
	
	struct REGPACK rg;
	
	rg.r_ax = 25*256;
	rg.r_cx = speed;
	rg.r_dx = end * 256 + start;
	intr (VECT, &rg);
	
}

	/*PCM������*/
	
int getpcmver(unsigned char far *dat)
{
	/*PCM�f�[�^�̃o�[�W������Ԃ�*/
	/*��PCM�`����0�������łȂ���΁APCM�f�[�^�̃o�[�W������Ԃ�*/
	int rcode;
	
	/*ID�`�F�b�N*/
	if ((dat[0] != 'P') || (dat[1] != 'C') || (dat[2] != 'M'))
	{
		return 0;
	}
	
	
	/*�o�[�W�����𓾂�*/
	rcode = dat[3] * 100 + dat[4];
	
	return rcode;
}
	
int pcmplay(unsigned char huge *dat)
{
	/*PCM�f�[�^��炷*/
	/*dat = PCM�t�@�C���`���f�[�^�̓����Ă���擪�|�C���^*/
	/*��PCM�`����0�������łȂ���΁APCM�f�[�^�̃o�[�W������Ԃ�*/
	
	struct REGPACK rg;
	int rcode;
	
	/*ID�`�F�b�N*/
	if ((dat[0] != 'P') || (dat[1] != 'C') || (dat[2] != 'M'))
	{
		return 0;
	}
	
	
	/*�o�[�W�����𓾂�*/
	rcode = dat[3] * 100 + dat[4];
	
	
	rg.r_ax = 40 * 256;
	rg.r_bx = FP_OFF(&dat[5]);
	
	rg.r_es = FP_SEG(&dat[5]);

	intr (VECT,&rg);

	return rcode;
}

int pcmplay2(unsigned char huge *dat, unsigned long length)
{
	/*PCM�f�[�^��炷*/
	/*dat = PCM�t�@�C���`���f�[�^�̓����Ă���擪�|�C���^*/
	/*��PCM�`����-2�������łȂ���΁APCM�f�[�^�̃o�[�W������Ԃ�*/
	/*-1�Ńo�b�t�@������Ȃ����Ƃ�����*/
	
	struct REGPACK rg;
	int rcode;
	
	/*ID�`�F�b�N*/
	if ((dat[0] != 'P') || (dat[1] != 'C') || (dat[2] != 'M'))
	{
		return -2;
	}
	
	
	/*�o�[�W�����𓾂�*/
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
	/*���t����PCM�������~�߂�*/
	
	struct REGPACK rg;
	
	rg.r_ax = 41 * 256;
	
	intr (VECT,&rg);
	
}

int	getstatus()
{
	/*���t�X�e�[�^�X�𓾂�*/
	/*
		�߂�l:
		
		�r�b�g0: FM�����Đ� (0:�Đ���,1:��~��)
		�r�b�g1: PCM�����Đ�(0:�Đ���,1:��~��)
		�r�b�g4: �|�[�g�ԍ���(0:188h, 1:288h)
		�r�b�g5: PCM������(0:���݂��Ȃ�,1:���݂���)
		�r�b�g6: YM2608��(0:���݂��Ȃ�,1:���݂���)
		�r�b�g7: FM�����{�[�h��(0:���݂��Ȃ�,1:���݂���)
	*/
	
	struct REGPACK rg;
	
	rg.r_ax = 17 * 256;
	
	intr (VECT,&rg);
	
	return(rg.r_ax & 0xff);
}

