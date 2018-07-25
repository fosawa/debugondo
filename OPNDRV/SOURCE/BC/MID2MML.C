	/*
	�W��MIDI�t�@�C�� �� MML �t�@�C���R���o�[�^
	
						Programmed by F.Osawa
	*/
	
	#define FILENAME_MAX 128
	
	#include <stdio.h>
	#include <string.h>
	#include <mem.h>
	
unsigned long getdword(FILE *fp)
{
	/*�_�u�����[�h��Ԃ��B�������A�C���e���`���ł͂Ȃ�*/
	
	unsigned long ul;
	int i;
	
	ul = 0;
	
	for (i = 0; i < 4; i++)
	{
		ul = ul << 8UL;
		ul += getc(fp);
	}
	
	return ul;
}

unsigned int getword(FILE *fp)
{
	/*���[�h�`����Ԃ��B�������A�C���e���`���ł͂Ȃ�*/
	
	unsigned long ul;
	int i;
	
	ul = 0;
	
	for (i = 0; i < 2; i++)
	{
		ul = ul << 8UL;
		ul += getc(fp);
	}
	
	return ul;
}

unsigned long getvarlen(FILE *fp)
{
	unsigned long val;
	int dat;
	
	if ((val = getc(fp)) & 0x80)
	{
		val &= 0x7f;
		do
		{
			val = (val << 7) + ((dat = getc(fp)) & 0x7f);
		} while (dat & 0x80);
	}
	
	return val;
}

void midchk(FILE *fp, FILE *fp2, unsigned int division, unsigned long trklength, int fm, int v_ch, unsigned char *rth)
{
	unsigned long tempo = 500000;	/*tempo = 120*/
	unsigned long deltatime;
	unsigned long count_deltatime = 0;
	int command, before_command, com, ch;
	int dat1, dat2;
	int dd;
	unsigned long len;
	int mml_tempo;
	int before_key = 0xff;	/*�O�̃L�[�ԍ�(ff�ŋx��)*/
	int mml_length = 0;
	int before_velo = 128;
	int now_velo = 0xff;
	int now_oct = 0xff;
	unsigned long before_deltatime = 0;
	int setf = 1;
	int lk = 0;
	int m_ch = 0xff;
	
	while (trklength-- > 0)
	{
		/*�f���^�^�C��*/
		deltatime = getvarlen(fp);
		count_deltatime += deltatime;
		
		/*�R�}���h*/
		command = getc(fp);
		
		/*�����j���O�X�e�[�^�X��?*/
		if (!(command & 0x80))
		{
			ungetc(command, fp);
			command = before_command;
		}
		
		/*�R�}���h���`���l���ƁA�R�}���h�ɕ�����*/
		com = command & 0xf0;
		ch = command & 0xf;
		
		switch (com)
		{
			case 0x80:
					/*�m�[�g�I�t*/
					dat1 = getc(fp);
					dat2 = getc(fp);
					
					if (ch == v_ch)
					{
					
					if (fm == 2)
					{
						int j, flag;
						/*�w��h�������ǂ���*/
						j = 0;
						flag = 0;
						while (rth[j] != 0)
						{
							if (dat1 == rth[j]) flag = 1;
							j++;
						}
						if (flag == 0) break;
					}
						
					if ((setf == 1) && (count_deltatime != 0))
					{
						/*�����鉹������������*/
						
						if ((now_velo != before_velo) && (before_key != 0xff))
						{
							/*�{�����[����������*/
							switch (fm)
							{
								case 0:
										/*PSG*/
										fprintf(fp2, "v%d", (before_velo >> 3));
										break;
								case 1:
										/*FM*/
										fprintf(fp2, "@v%d", before_velo);
										break;
								case 2:
										/*RTH*/
										fprintf(fp2, "@v%d", (before_velo >> 2));
										break;
							}
							now_velo = before_velo;
						}
			
						/*�x����?*/
						if (before_key == 0xff)
						{
							if (fm != 2) fprintf(fp2, "r");
								else fprintf(fp2, "p");
						}
						else
						{
						int oct, keyno;
						char *mml[]=
						{
						"c", "c+", "d", "d+", "e", "f", "f+", "g", "g+", "a", "a+", "b"};
						/*�I�N�^�[�u�l�̎擾*/
						oct = before_key / 12 - 1;
				
						if ((oct != now_oct) && (fm != 2))
						{
							fprintf(fp2, "o%d", oct);
							now_oct = oct;
						}
				
						/*�L�[�i���o�[�擾*/
						keyno = (before_key % 12);
				
						if (fm != 2)
						{
							fprintf(fp2, "%s", mml[keyno]);
						}
						else
						{
							fprintf(fp2, "x");
						}
						
						}
			
						/*�f���^�^�C��(����)*/
						fprintf(fp2, "*%d ", count_deltatime);
						if (lk > 7)
						{
							fprintf(fp2, "\n");
							lk = 0;
						}
						else
						{
						lk++;
						}
						setf = 0;
						
					}
					
						before_key = 0xff;
						before_velo = dat2;
						setf = 1;
						m_ch = ch;
						count_deltatime = 0;
					}
					break;
			
			case 0x90:
					/*�m�[�g�I��*/
					dat1 = getc(fp);
					dat2 = getc(fp);
			
					
					if (ch == v_ch)
					{
		
					if (fm == 2)
					{
						int j, flag;
						/*�w��h�������ǂ���*/
						j = 0;
						flag = 0;
						
						while (rth[j] != 0)
						{
							if (dat1 == rth[j]) flag = 1;
							j++;
						}
						if (flag == 0) break;
					}
		
					if ((setf == 1) && (count_deltatime != 0))
					{
						/*�����鉹������������*/
						
						if ((now_velo != before_velo) && (before_key != 0xff))
						{
							/*�{�����[����������*/
							switch (fm)
							{
								case 0:
										/*PSG*/
										fprintf(fp2, "v%d", (before_velo >> 3));
										break;
								case 1:
										/*FM*/
										fprintf(fp2, "@v%d", before_velo);
										break;
								case 2:
										/*RTH*/
										fprintf(fp2, "@v%d", (before_velo >> 2));
										break;
							}
							now_velo = before_velo;
						}
			
						/*�x����?*/
						if (before_key == 0xff)
						{
							if (fm != 2) fprintf(fp2, "r");
								else fprintf(fp2, "p");
						}
						else
						{
						int oct, keyno;
						char *mml[]=
						{
						"c", "c+", "d", "d+", "e", "f", "f+", "g", "g+", "a", "a+", "b"};
						/*�I�N�^�[�u�l�̎擾*/
						oct = before_key / 12 - 1;
				
						if ((oct != now_oct) && (fm != 2))
						{
							fprintf(fp2, "o%d", oct);
							now_oct = oct;
						}
				
						/*�L�[�i���o�[�擾*/
						keyno = (before_key % 12);
				
						if (fm != 2)
						{
							fprintf(fp2, "%s", mml[keyno]);
						}
						else
						{
							fprintf(fp2, "x");
						}
						}
			
						/*�f���^�^�C��(����)*/
						fprintf(fp2, "*%d ", count_deltatime);
						if (lk > 7)
						{
							fprintf(fp2, "\n");
							lk = 0;
						}
						else
						{
						lk++;
						}
						setf = 0;
						
					}
					
						before_key = dat1;
						before_velo = dat2;
						if (before_velo == 0)
						{
							before_key = 0xff;
						}
					
						setf = 1;
						m_ch = ch;
						count_deltatime = 0;
					}
					break;
					
			case 0xa0:
					/*�|���t�H�j�b�N�L�[�v���b�V���[*/
					dat1 = getc(fp);
					dat2 = getc(fp);
					if (ch == v_ch)
					{
						fprintf(fp2, "\n;PolyKeyPress [Note%d][Press%d]\n", dat1, dat2);
					
						before_key = 0xff;
						
						setf = 1;
						m_ch = ch;
						count_deltatime = 0;
					}
					break;
			
			case 0xb0:
					/*�R���g���[���`�F���W*/
					dat1 = getc(fp);
					dat2 = getc(fp);
					if (dat1 <= 121)
					{
						/*�R���g���[���i���o�[*/
					
					if ((setf == 1) && (count_deltatime != 0))
					{
						/*�����鉹������������*/
						
						if ((now_velo != before_velo) && (before_key != 0xff))
						{
							/*�{�����[����������*/
							switch (fm)
							{
								case 0:
										/*PSG*/
										fprintf(fp2, "v%d", (before_velo >> 3));
										break;
								case 1:
										/*FM*/
										fprintf(fp2, "@v%d", before_velo);
										break;
								case 2:
										/*RTH*/
										fprintf(fp2, "@v%d", (before_velo >> 2));
										break;
							}
							now_velo = before_velo;
						}
			
						/*�x����?*/
						if (before_key == 0xff)
						{
							if (fm != 2) fprintf(fp2, "r");
								else fprintf(fp2, "p");
						}
						else
						{
						int oct, keyno;
						char *mml[]=
						{
						"c", "c+", "d", "d+", "e", "f", "f+", "g", "g+", "a", "a+", "b"};
						/*�I�N�^�[�u�l�̎擾*/
						oct = before_key / 12 - 1;
				
						if ((oct != now_oct) && (fm != 2))
						{
							fprintf(fp2, "o%d", oct);
							now_oct = oct;
						}
				
						/*�L�[�i���o�[�擾*/
						keyno = (before_key % 12);
				
						if (fm != 2)
						{
							fprintf(fp2, "%s", mml[keyno]);
						}
						else
						{
							fprintf(fp2, "x");
						}
						
						}
			
						/*�f���^�^�C��(����)*/
						fprintf(fp2, "*%d ", count_deltatime);
						if (lk > 7)
						{
							fprintf(fp2, "\n");
							lk = 0;
						}
						else
						{
						lk++;
						}
						setf = 0;
						
					}
						
						
						
						fprintf(fp2, "\n;Control [%d][%d]\n", dat1, dat2);
						
						before_key = 0xff;
						
						setf = 1;
						m_ch = ch;
						count_deltatime = 0;
						
					}
					else
					{
					
					
					if ((setf == 1) && (count_deltatime != 0))
					{
						/*�����鉹������������*/
						
						if ((now_velo != before_velo) && (before_key != 0xff))
						{
							/*�{�����[����������*/
							switch (fm)
							{
								case 0:
										/*PSG*/
										fprintf(fp2, "v%d", (before_velo >> 3));
										break;
								case 1:
										/*FM*/
										fprintf(fp2, "@v%d", before_velo);
										break;
								case 2:
										/*RTH*/
										fprintf(fp2, "@v%d", (before_velo >> 2));
										break;
							}
							now_velo = before_velo;
						}
			
						/*�x����?*/
						if (before_key == 0xff)
						{
							if (fm != 2) fprintf(fp2, "r");
								else fprintf(fp2, "p");
						}
						else
						{
						int oct, keyno;
						char *mml[]=
						{
						"c", "c+", "d", "d+", "e", "f", "f+", "g", "g+", "a", "a+", "b"};
						/*�I�N�^�[�u�l�̎擾*/
						oct = before_key / 12 - 1;
				
						if ((oct != now_oct) && (fm != 2))
						{
							fprintf(fp2, "o%d", oct);
							now_oct = oct;
						}
				
						/*�L�[�i���o�[�擾*/
						keyno = (before_key % 12);
				
						if (fm != 2)
						{
							fprintf(fp2, "%s", mml[keyno]);
						}
						else
						{
							fprintf(fp2, "x");
						}
						
						}
			
						/*�f���^�^�C��(����)*/
						fprintf(fp2, "*%d ", count_deltatime);
						if (lk > 7)
						{
							fprintf(fp2, "\n");
							lk = 0;
						}
						else
						{
						lk++;
						}
						setf = 0;
						
					}
					
						switch (dat1)
						{
							case 122:
								/*Local control*/
								if (ch == v_ch)
								{
									fprintf(fp2, "\n;LocalControl");
									before_key = 0xff;
									
									setf = 1;
									m_ch = ch;
									count_deltatime = 0;
						
								}
								if (dat2 == 0)
								{
									if (ch == v_ch) fprintf(fp2, " Off\n");
								}
								else
								{
									if (ch == v_ch) fprintf(fp2, " On\n");
								}
								break;
								
							case 123:
								/*All note off*/
								if (ch == v_ch)
								{
									fprintf(fp2, "\n;All Note Off\n");
									before_key = 0xff;
									
									setf = 1;
									m_ch = ch;
									count_deltatime = 0;
								
								}
								break;
								
							case 124:
								/*Omni Off*/
								if (ch == v_ch)
								{
									fprintf(fp2, "\n;Omni Off\n");
									before_key = 0xff;
									
									setf = 1;
									m_ch = ch;
									count_deltatime = 0;
								
								}
								break;
								
							case 125:
								/*Omni On*/
								if (ch == v_ch)
								{
									fprintf(fp2, "\n;Omni On\n");
									before_key = 0xff;
									
									setf = 1;
									m_ch = ch;
									count_deltatime = 0;
								
								}
								break;
								
							case 126:
								/*Mono Mode*/
								if (ch == v_ch)
								{
									fprintf(fp2, "\n;Mono Mode [%d]\n", dat2);
									before_key = 0xff;
									
									setf = 1;
									m_ch = ch;
									count_deltatime = 0;
								
								}
								break;
								
							case 127:
								/*Poly Mode*/
								if (ch == v_ch)
								{
									fprintf(fp2, "\n;Poly Mode\n");
									before_key = 0xff;
									
									setf = 1;
									m_ch = ch;
									count_deltatime = 0;
								
								}
								break;
							}
						}
					
					break;
					
			case 0xc0:
					/*�v���O�����`�F���W*/
					dat1 = getc(fp);
					if (ch == v_ch)
					{
					
					if ((setf == 1) && (count_deltatime != 0))
					{
						/*�����鉹������������*/
						
						if ((now_velo != before_velo) && (before_key != 0xff))
						{
							/*�{�����[����������*/
							switch (fm)
							{
								case 0:
										/*PSG*/
										fprintf(fp2, "v%d", (before_velo >> 3));
										break;
								case 1:
										/*FM*/
										fprintf(fp2, "@v%d", before_velo);
										break;
								case 2:
										/*RTH*/
										fprintf(fp2, "@v%d", (before_velo >> 2));
										break;
							}
							now_velo = before_velo;
						}
			
						/*�x����?*/
						if (before_key == 0xff)
						{
							if (fm != 2) fprintf(fp2, "r");
								else fprintf(fp2, "p");
						}
						else
						{
						int oct, keyno;
						char *mml[]=
						{
						"c", "c+", "d", "d+", "e", "f", "f+", "g", "g+", "a", "a+", "b"};
						/*�I�N�^�[�u�l�̎擾*/
						oct = before_key / 12 - 1;
				
						if ((oct != now_oct) && (fm != 2))
						{
							fprintf(fp2, "o%d", oct);
							now_oct = oct;
						}
				
						/*�L�[�i���o�[�擾*/
						keyno = (before_key % 12);
				
						if (fm != 2)
						{
							fprintf(fp2, "%s", mml[keyno]);
						}
						else
						{
							fprintf(fp2, "x");
						}
						
						}
			
						/*�f���^�^�C��(����)*/
						fprintf(fp2, "*%d ", count_deltatime);
						if (lk > 7)
						{
							fprintf(fp2, "\n");
							lk = 0;
						}
						else
						{
						lk++;
						}
						setf = 0;
						
					}
					

						fprintf(fp2, "\n;Program Change [No.%d]\n", dat1);
						fprintf(fp2, "*sst@%d\n", dat1);
					
						before_key = 0xff;
						
						setf = 1;
						m_ch = ch;
						count_deltatime = 0;
					
					}
					break;
					
			case 0xd0:
					/*�`���l���v���b�V���[*/
					dat1 = getc(fp);
					if (ch == v_ch)
					{
					
					if ((setf == 1) && (count_deltatime != 0))
					{
						/*�����鉹������������*/
						
						if ((now_velo != before_velo) && (before_key != 0xff))
						{
							/*�{�����[����������*/
							switch (fm)
							{
								case 0:
										/*PSG*/
										fprintf(fp2, "v%d", (before_velo >> 3));
										break;
								case 1:
										/*FM*/
										fprintf(fp2, "@v%d", before_velo);
										break;
								case 2:
										/*RTH*/
										fprintf(fp2, "@v%d", (before_velo >> 2));
										break;
							}
							now_velo = before_velo;
						}
			
						/*�x����?*/
						if (before_key == 0xff)
						{
							if (fm != 2) fprintf(fp2, "r");
								else fprintf(fp2, "p");
						}
						else
						{
						int oct, keyno;
						char *mml[]=
						{
						"c", "c+", "d", "d+", "e", "f", "f+", "g", "g+", "a", "a+", "b"};
						/*�I�N�^�[�u�l�̎擾*/
						oct = before_key / 12 - 1;
				
						if ((oct != now_oct) && (fm != 2))
						{
							fprintf(fp2, "o%d", oct);
							now_oct = oct;
						}
				
						/*�L�[�i���o�[�擾*/
						keyno = (before_key % 12);
				
						if (fm != 2)
						{
							fprintf(fp2, "%s", mml[keyno]);
						}
						else
						{
							fprintf(fp2, "x");
						}
						
						}
			
						/*�f���^�^�C��(����)*/
						fprintf(fp2, "*%d ", count_deltatime);
						if (lk > 7)
						{
							fprintf(fp2, "\n");
							lk = 0;
						}
						else
						{
						lk++;
						}
						setf = 0;
						
					}
					
						fprintf(fp2, "\n;Chanel Press [%d]\n", dat1);
						before_key = 0xff;
						
						setf = 1;
						m_ch = ch;
						count_deltatime = 0;
			
					}
					break;
					
			case 0xe0:
					/*�s�b�`�z�C�[���x���h*/
					dat1 = getc(fp);
					dat2 = getc(fp);
					if (ch == v_ch)
					{
					
					if ((setf == 1) && (count_deltatime != 0))
					{
						/*�����鉹������������*/
						
						if ((now_velo != before_velo) && (before_key != 0xff))
						{
							/*�{�����[����������*/
							switch (fm)
							{
								case 0:
										/*PSG*/
										fprintf(fp2, "v%d", (before_velo >> 3));
										break;
								case 1:
										/*FM*/
										fprintf(fp2, "@v%d", before_velo);
										break;
								case 2:
										/*RTH*/
										fprintf(fp2, "@v%d", (before_velo >> 2));
										break;
							}
							now_velo = before_velo;
						}
			
						/*�x����?*/
						if (before_key == 0xff)
						{
							if (fm != 2) fprintf(fp2, "r");
								else fprintf(fp2, "p");
						}
						else
						{
						int oct, keyno;
						char *mml[]=
						{
						"c", "c+", "d", "d+", "e", "f", "f+", "g", "g+", "a", "a+", "b"};
						/*�I�N�^�[�u�l�̎擾*/
						oct = before_key / 12 - 1;
				
						if ((oct != now_oct) && (fm != 2))
						{
							fprintf(fp2, "o%d", oct);
							now_oct = oct;
						}
				
						/*�L�[�i���o�[�擾*/
						keyno = (before_key % 12);
				
						if (fm != 2)
						{
							fprintf(fp2, "%s", mml[keyno]);
						}
						else
						{
							fprintf(fp2, "x");
						}
						
						}
			
						/*�f���^�^�C��(����)*/
						fprintf(fp2, "*%d ", count_deltatime);
						if (lk > 7)
						{
							fprintf(fp2, "\n");
							lk = 0;
						}
						else
						{
						lk++;
						}
						setf = 0;
						
					}
					
						fprintf(fp2, "\n;Pitch Bend [%d][%d]\n", dat1, dat2);
						before_key = 0xff;
						
						setf = 1;
						m_ch = ch;
						count_deltatime = 0;
			

					}
					break;
					
			case 0xf0:
					/*���̑�*/
					
					if ((setf == 1) && (count_deltatime != 0))
					{
						/*�����鉹������������*/
						
						if ((now_velo != before_velo) && (before_key != 0xff))
						{
							/*�{�����[����������*/
							switch (fm)
							{
								case 0:
										/*PSG*/
										fprintf(fp2, "v%d", (before_velo >> 3));
										break;
								case 1:
										/*FM*/
										fprintf(fp2, "@v%d", before_velo);
										break;
								case 2:
										/*RTH*/
										fprintf(fp2, "@v%d", (before_velo >> 2));
										break;
							}
							now_velo = before_velo;
						}
			
						/*�x����?*/
						if (before_key == 0xff)
						{
							if (fm != 2) fprintf(fp2, "r");
								else fprintf(fp2, "p");
						}
						else
						{
						int oct, keyno;
						char *mml[]=
						{
						"c", "c+", "d", "d+", "e", "f", "f+", "g", "g+", "a", "a+", "b"};
						/*�I�N�^�[�u�l�̎擾*/
						oct = before_key / 12 - 1;
				
						if ((oct != now_oct) && (fm != 2))
						{
							fprintf(fp2, "o%d", oct);
							now_oct = oct;
						}
				
						/*�L�[�i���o�[�擾*/
						keyno = (before_key % 12);
				
						if (fm != 2)
						{
							fprintf(fp2, "%s", mml[keyno]);
						}
						else
						{
							fprintf(fp2, "x");
						}
						
						}
			
						/*�f���^�^�C��(����)*/
						fprintf(fp2, "*%d ", count_deltatime);
						if (lk > 7)
						{
							fprintf(fp2, "\n");
							lk = 0;
						}
						else
						{
						lk++;
						}
						setf = 0;
						
					}
					
					before_key = 0xff;
					
					setf = 1;
					m_ch = ch;
					count_deltatime = 0;
					
					switch (ch)
					{
						case 0:
								/*�G�N�X�N���[�V�u���b�Z�[�W*/
								fprintf(fp2, "\n;Exclusive Msg ");
								len = getvarlen(fp);
								while (len-- > 0)
								{
									dat1 = getc(fp);
									fprintf(fp2, "[%d]", dat1);
								}
								fprintf(fp2, "\n");
								ungetc(dat1, fp);
								break;
						case 1:
								/*�N�I�[�^�[�t���[�����b�Z�[�W*/
								fprintf(fp2, "\n;Quota Msg ");
								dat1 = getc(fp);
								fprintf(fp2, "[%d]", dat1);
								fprintf(fp2, "\n");
								break;
						case 2:
								/*�\���O�|�W�V�����Z���N�^*/
								fprintf(fp2, "\n;Sond Position Msg ");
								dat1 = getc(fp);
								dat2 = getc(fp);
								dat2 = (dat1 << 7) + dat1;
								fprintf(fp2, "[%d]", dat2);
								fprintf(fp2, "\n");
								break;
						
						case 3:
								/*�\���O�Z���N�g*/
								fprintf(fp2, "\n;Song Select Msg ");
								dat1 = getc(fp);
								fprintf(fp2, "[%d]", dat1);
								fprintf(fp2, "\n");
								break;
						case 4:
						case 5:
								/*����`*/
								fprintf(fp2, "\n;Unknown ");
								while (!((dat1 = getc(fp)) & 0x80))
								{
									fprintf(fp2, "[%d]", dat1);
								}
								fprintf(fp2, "\n");
								ungetc(dat1, fp);
								break;
								
						case 6:
								/*�`���[�����N�G�X�g*/
								fprintf(fp2, "\n;Tune Request \n");
								break;
								
						case 7:
								/*�G�N�X�N���[�V�u���b�Z�[�W*/
								fprintf(fp2, "\n;Exclusive Msg ");
								len = getvarlen(fp);
								while (len-- > 0)
								{
									dat1 = getc(fp);
									fprintf(fp2, "[%d]", dat1);
								}
								fprintf(fp2, "\n");
								ungetc(dat1, fp);
								break;
						case 8:
								/*MIDI�N���b�N*/
								fprintf(fp2, "\n;MIDI Clk\n");
								break;
								
						case 9:
								/*����`*/
								fprintf(fp2, "\n;Unknown ");
								while (!((dat1 = getc(fp)) & 0x80))
								{
									fprintf(fp2, "[%d]", dat1);
								}
								fprintf(fp2, "\n");
								ungetc(dat1, fp);
								break;
								
						case 10:
								/*�X�^�[�g*/
								fprintf(fp2, "\n;Start\n");
								break;
								
						case 11:
								/*�R���e�B�j���[*/
								fprintf(fp2, "\n;Continue\n");
								break;
						case 12:
								/*�X�g�b�v*/
								fprintf(fp2, "\n;Stop\n");
								break;
								
						case 13:
								/*����`*/
								fprintf(fp2, "\n;Unknown ");
								while (!((dat1 = getc(fp)) & 0x80))
								{
									fprintf(fp2, "[%d]", dat1);
								}
								fprintf(fp2, "\n");
								ungetc(dat1, fp);
								break;
								
						case 14:
								/*�A�N�e�B�u�Z���V���O*/
								fprintf(fp2, "\n;Active Sensing\n");
								break;
								
						case 15:
								/*�V�X�e�����Z�b�g*/
								/*�����A�W��MIDI�t�@�C���ł̓��^�C�x���g*/
								fprintf(fp2, "\n;Meta Event: ");
								
								/*���^�ԍ��擾*/
								dd = getc(fp);
								len = getvarlen(fp);
								switch (dd)
								{
									case 0:
											/*�V�[�P���X�i���o�[*/
											fprintf(fp2, "Sequebce Number ");
											while (len-- > 0)
											{
												dat1 = getc(fp);
												fprintf(fp2, "[%d]", dat1);
											}
											fprintf(fp2, "\n");
											break;
											
									case 1:
											/*�e�L�X�g�C�x���g*/
											fprintf(fp2, "Text Event ");
											while (len-- > 0)
											{
												dat1 = getc(fp);
												fprintf(fp2, "%c", dat1);
											}
											fprintf(fp2, "\n");
											break;
											
									case 2:
											/*���쌠*/
											fprintf(fp2, "Copyright ");
											while (len-- > 0)
											{
												dat1 = getc(fp);
												fprintf(fp2, "%c", dat1);
											}
											fprintf(fp2, "\n");
											break;
											
									case 3:
											/*�g���b�N��*/
											fprintf(fp2, "Trk Name ");
											while (len-- > 0)
											{
												dat1 = getc(fp);
												fprintf(fp2, "%c", dat1);
											}
											fprintf(fp2, "\n");
											break;
											
									case 4:
											/*�y�햼*/
											fprintf(fp2, "Sound Name ");
											while (len-- > 0)
											{
												dat1 = getc(fp);
												fprintf(fp2, "%c", dat1);
											}
											fprintf(fp2, "\n");
											break;
											
									case 5:
											/*�̎�*/
											fprintf(fp2, "Song ");
											while (len-- > 0)
											{
												dat1 = getc(fp);
												fprintf(fp2, "%c", dat1);
											}
											fprintf(fp2, "\n");
											break;
											
									case 6:
											/*�}�[�J�[*/
											fprintf(fp2, "Marker ");
											while (len-- > 0)
											{
												dat1 = getc(fp);
												fprintf(fp2, "%c", dat1);
											}
											fprintf(fp2, "\n");
											break;
											
									case 7:
											/*�L���[�|�C���g*/
											fprintf(fp2, "Que Point ");
											while (len-- > 0)
											{
												dat1 = getc(fp);
												fprintf(fp2, "%c", dat1);
											}
											fprintf(fp2, "\n");
											break;
											
									case 0x2f:
											/*�g���b�N�I��*/
											fprintf(fp2, "Trk End ");
											while (len-- > 0)
											{
												dat1 = getc(fp);
												fprintf(fp2, "%c", dat1);
											}
											fprintf(fp2, "\n");
											return;
											break;
											
									case 0x51:
											/*�e���|*/
											fprintf(fp2, "Tempo ");
											tempo = 0;
											while (len-- > 0)
											{
												dat1 = getc(fp);
												tempo = (tempo << 8) + dat1;
											}
											fprintf(fp2, "%ld ��s\n", tempo);
											mml_tempo = 1000L * 1000L * 60L / tempo;
											fprintf(fp2, "T%d\n", mml_tempo);
											break;
									
									case 0x54:
											/*SMPTE offset*/
											fprintf(fp2, "SMPTE Offset ");
											while (len-- > 0)
											{
												dat1 = getc(fp);
												fprintf(fp2, "[%d]", dat1);
											}
											fprintf(fp2, "\n");
											break;
											
									case 0x58:
											/*���q�L��*/
											fprintf(fp2, "���q�L�� ");
											
											/*����*/
											dat1 = getc(fp);
											fprintf(fp2, "%d ", dat1);
											
											/*���q*/
											dat1 = getc(fp);
											dat1 = 2 << dat1;
											fprintf(fp2, "/ %d ", dat1);
											
											/*���g���m�[���N���b�N/MIDI�N���b�N*/
											dat1 = getc(fp);
											fprintf(fp2, " %d[metoro Clk/ MIDI Clk] ", dat1);
											
											/*MIDI4����������32�������̐�*/
											dat1 = getc(fp);
											fprintf(fp2, " %d[32������/MIDI4������]", dat1);
											
											fprintf(fp2, "\n");
											break;
											
									case 0x59:
											/*����*/
											fprintf(fp2, "���� ");
											
											/*�V���[�v/�t���b�g�̐�*/
											
											dat1 = getc(fp);
											if (dat1 < 128)
											{
												fprintf(fp2, "# %d ", dat1);
											}
											else
											{
												fprintf(fp2, "b %d ", 256 - dat1);
											}
											
											/*���� or �Z��*/
											
											dat1 = getc(fp);
											if (dat1 == 0)
											{
												fprintf(fp2, "����");
											}
											else
											{
												fprintf(fp2, "�Z��");
											}
											
											fprintf(fp2, "\n");
											break;
											
									case 0x7f:
											/*�V�[�P���T���L���^�C�x���g*/
											fprintf(fp2, "Special Meta Event ");
											while (len-- > 0)
											{
												dat1 = getc(fp);
												fprintf(fp2, "[%d]", dat1);
											}
											fprintf(fp2, "\n");
											break;
											
									default:
											/*�s��*/
											fprintf(fp2, "Unknown ");
											while (len-- > 0)
											{
												dat1 = getc(fp);
												fprintf(fp2, "[%d]", dat1);
											}
											fprintf(fp2, "\n");
											break;
											
								}
					}
		}
			
		before_command = command;
		before_deltatime = deltatime;
	}
}
							

void mid2mml(char *filename, char *filename2)
{
	FILE *fp, *fp2;
	char id[4];
	unsigned long trklength;
	unsigned int trkblock, kk_trkblock, division;
	unsigned long now_f;
	int fm;
	int format;
	int i;
	
	if ((fp = fopen(filename, "rb")) == NULL)
	{
		printf("�t�@�C��[%s]��������܂���\n", filename);
		exit(1);
	}
	
	if ((fp2 = fopen(filename2, "rb")) != NULL)
	{
    	int i;
    	fprintf (stderr, "[%s] Overwrite [y / else] ? ", filename2);
		i = getchar();
      	if (!((i == 'Y') || (i == 'y') || (i == '�')))
		{
	  		fprintf (stderr, "... not converted");
	  		fclose(fp);
	  		fclose(fp2);
	  		return 1;
		}
	}
	
	if ((fp2 = fopen(filename2, "wb")) == NULL)
	{
		printf("�t�@�C��[%s]���J���܂���\n", filename2);
		exit(1);
	}
	
	/*�W��MIDI�t�@�C�����*/
	
	/*�w�b�_��*/
	
	/*MThd ID�m�F*/
	fread(id, 4, 1, fp);
	if (memcmp(id, "MThd", 4) != 0)
	{
		printf("�t�@�C��[%s]�͕W��MIDI�t�@�C���ł͂���܂���\n", filename);
		exit(1);
	}
	
	/*Mthd���̒����𓾂�*/
	trklength = getdword(fp);
	
	/*format�𓾂�*/
	format = getword(fp);
	if (format != 0)
	{
		printf("�t�H�[�}�b�g0�̂ݑΉ��ł�\n");
		exit(1);
	}
	
	/*�g���b�N�u���b�N��*/
	trkblock = getword(fp);
	
	/*division*/
	division = getword(fp);
	fprintf(fp2, "\\division[%d]\n", division);
	
	/*MTrk����ǂ�*/
	
	now_f = ftell(fp);
	
	for (i = 0; i < 9; i++)
	{
		if (i < 6)
		{
			fprintf(fp2, "\\fm[%d]{\n", i + 1);
			fm = 1;
		}
		else
		{
			fprintf(fp2, "\\psg[%d]{\n", i - 5);
			fm = 0;
		}
	
		fseek(fp, now_f, SEEK_SET);
	
		kk_trkblock = trkblock;
		while (kk_trkblock-- > 0)
		{
			/*MTrk ID�m�F*/
			fread(id, 4, 1, fp);
			if (memcmp(id, "MTrk", 4) != 0)
			{
				printf("MTrk�u���b�N��������܂���\n", filename);
				exit(1);
			}
		
			/*�g���b�N�̒����𓾂�*/
			trklength = getdword(fp);
		
			/*MIDI�R�}���h���*/
			midchk(fp, fp2, division, trklength, fm, i, NULL);
		}
		fprintf(fp2, "}\n");
	}

	for (i = 0; i < 6; i++)
	{
		unsigned char bass[] = {35, 36, 0};
		unsigned char snare[] = {38, 40, 0};
		unsigned char sym[] = {49, 51, 52, 55, 57, 59, 0};
		unsigned char hi[] = {42, 44, 46, 0};
		unsigned char tam[] = {41, 43, 45, 47, 48, 50, 0};
		unsigned char rim[] = {37, 0};
		
		unsigned char *rthconv[6];
		
		rthconv[0] = bass;
		rthconv[1] = snare;
		rthconv[2] = sym;
		rthconv[3] = hi;
		rthconv[4] = tam;
		rthconv[5] = rim;
		
		
		fprintf(fp2, "\\rth[%d]{\n", i + 1);
		fseek(fp, now_f, SEEK_SET);
	
		kk_trkblock = trkblock;
		while (kk_trkblock-- > 0)
		{
			/*MTrk ID�m�F*/
			fread(id, 4, 1, fp);
			if (memcmp(id, "MTrk", 4) != 0)
			{
				printf("MTrk�u���b�N��������܂���\n", filename);
				exit(1);
			}
		
			/*�g���b�N�̒����𓾂�*/
			trklength = getdword(fp);
		
			/*MIDI�R�}���h���*/
			midchk(fp, fp2, division, trklength, 2, 10 - 1, rthconv[i]);
		}
		fprintf(fp2, "}\n");
	}

	fclose(fp2);
	fclose(fp);
}

int main(int argv, char **argc)
{
	char filename[FILENAME_MAX], filename2[FILENAME_MAX];
	int i;
	int filenum;
	
	
	if (argv <= 1)
		{
		msg:
		
		puts ("mid2mml  Ver 1.00 : �W��MIDI�`���t�@�C����MML�t�@�C���ɕϊ����܂�");
		puts ("                                    Programmed by F.Osawa");
		puts ("Usage:");
		puts ("     mid2mml MIDIfilename [MMLfilename] [Options]");
		puts ("Options:");
		return(1);
		}
	
	/*�I�v�V�����`�F�b�N*/
	filenum = 0;
	
	for (i = 1; i < argv; i++)
	{
		if ((*(argc[i]) != '/') && (*(argc[i]) != '-'))
		{
			/*�t�@�C����*/
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
		
		/*�I�v�V����*/
		
		strlwr(argc[i]); /*�啶��->������*/
		argc[i]++; /* /��-���΂� */
		
		if (strstr(argc[i], "h") == argc[i])
			goto msg;
		if (strstr(argc[i], "?") == argc[i])
			goto msg;
		
		err:
		
		fprintf (stderr, "Incorrect option -[%s]", argc[i] - 1);
		return(1);
	
	}
	
	if (filenum == 1) 
	{
		/*�t�@�C������������̎��ɂ́Asource,destination���ɓ����t�@�C�����ɂ���*/
		char *s;
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
	
	/*destination���h���C�u���݂̂̏ꍇ�ɂ́A�t�@�C������������*/
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

	/*destination��'\'�ŏI����Ă���ꍇ�ɂ́A�t�@�C������������*/
	
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
	
	/*�g���q���Ȃ����mid/mml�ɂ���*/
	
	if (strchr(filename, '.') == NULL)
		strcat(filename, ".mid");
	
	if (strchr(filename2, '.') == NULL)
		strcat(filename2, ".mml");
	
	mid2mml(filename, filename2);
	
	return 0;
}
