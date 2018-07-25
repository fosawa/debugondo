DECLARE FUNCTION OPNCHECK ()
DECLARE FUNCTION GETDIVISION (dat$)
DECLARE FUNCTION GETOPNVER (dat$)
DECLARE FUNCTION OPNSTART (dat$)
DECLARE SUB MEND ()
DECLARE SUB MFADE (fmspeed, psgspeed)
DECLARE FUNCTION MREAD! (ch!, workno!)
DECLARE FUNCTION VARREAD (num)
DECLARE SUB MESCON ()
DECLARE SUB MESCOF ()
DECLARE SUB TEMPOCHANGE (tempo!, division!)
DECLARE FUNCTION REGREAD! (regno!, sel!)
DECLARE FUNCTION GETSEMAX! (dat$)
DECLARE FUNCTION GETSEVER! (dat$)
DECLARE FUNCTION SESTORE! (dat$)
DECLARE SUB SESTART (num!)
DECLARE FUNCTION MIDIINIT! (interface!)
DECLARE FUNCTION MIDISTART! (dat$)
DECLARE SUB MIDISTOP ()
DECLARE SUB MIDICONT ()
DECLARE SUB MIDITEMPO (tempo)
DECLARE SUB MIDICLOSE ()
DECLARE SUB MIDIEXT ()
DECLARE SUB MIDISEND (dat)
DECLARE FUNCTION GETMIDI! ()
DECLARE SUB MIDIFADE (pstart, pend, speed)
DECLARE FUNCTION PCMPLAY! (dat$)
DECLARE FUNCTION GETPCMVER! (dat$)
DECLARE SUB PCMSTOP ()
DECLARE FUNCTION GETSTATUS! ()

'$INCLUDE: 'QB.BI'


FUNCTION GETDIVISION (dat$)
'OPN �t�@�C����division��Ԃ�
'0�ŗ^����ꂽ�f�[�^��OPN�`���łȂ�
	IF MID$(dat$, 1, 3) <> "OPN" THEN
		GETDIVISION = 0
		EXIT FUNCTION
	END IF

	GETDIVISION = ASC(MID$(dat$, 6, 1)) + ASC(MID$(dat$, 7, 1)) * 256
	
END FUNCTION

FUNCTION GETMIDI
'MIDI���t�󋵂�����
'�߂�l: ����8bit 0 = ���t���A1 = �ҋ@���A 2 = ���t�I��
'    ���8bit �x���V�e�B�␳�l
	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = 26 * 256
	
	CALL interrupt(&H41, inregs, outregs)
	
	GETMIDI = outregs.ax
END FUNCTION

FUNCTION GETOPNVER (dat$)
'OPN�t�@�C���̃o�[�W������Ԃ�
'0�ŗ^����ꂽ�f�[�^��OPN�`���łȂ����Ƃ�����
'�����łȂ���΁A�f�[�^�o�[�W������100�{�l��Ԃ�
	IF MID$(dat$, 1, 3) <> "OPN" THEN
		GETOPNVER = 0
		EXIT FUNCTION
	END IF
	
	GETOPNVER = ASC(MID$(dat$, 4, 1)) * 100 + ASC(MID$(dat$, 5, 1))
	
END FUNCTION

FUNCTION GETPCMVER (dat$)
'PCM�f�[�^�̃o�[�W�����𓾂�BPCM�f�[�^�łȂ��ꍇ�ɂ�0
'
	IF MID$(dat$, 1, 3) <> "PCM" THEN
		GETPCMVER = 0
		EXIT FUNCTION
	END IF
	
	GETPCMVER = ASC(MID$(dat$, 4, 1)) * 100 + ASC(MID$(dat$, 5, 1))
END FUNCTION

FUNCTION GETSEMAX (dat$)
'SE�t�@�C�����ǂ����𒲂ׂ�
'SE�t�@�C���Ȃ�Ίi�[����Ă���SE�i���o�[�ő�l���A�����łȂ����0��Ԃ�
	
	IF MID$(dat$, 1, 3) <> "SE " THEN
		GETSEMAX = 0
		EXIT FUNCTION
	END IF

	GETSEMAX = ASC(MID$(dat$, 6, 1))
	
END FUNCTION

FUNCTION GETSEVER (dat$)
'SE�t�@�C���̃o�[�W������Ԃ�
'SE�t�@�C���łȂ����0���ASE�t�@�C���ł���΃o�[�W������100�{��Ԃ�

	IF MID$(dat$, 1, 3) <> "SE " THEN
		GETSEVER = 0
		EXIT FUNCTION
	END IF

	GETSEVER = ASC(MID$(dat$, 4, 1)) * 100 + ASC(MID$(dat$, 5, 1))

END FUNCTION

FUNCTION GETSTATUS
'�e��X�e�[�^�X�𒲂ׂ�
'   �߂�l:
'
'   �r�b�g0: FM�����Đ� (0:�Đ���,1:��~��)
'   �r�b�g1: PCM�����Đ�(0:�Đ���,1:��~��)
'   �r�b�g4: �|�[�g�ԍ���(0:188h, 1:288h)
'   �r�b�g5: PCM������(0:���݂��Ȃ�,1:���݂���)
'   �r�b�g6: YM2608��(0:���݂��Ȃ�,1:���݂���)
'   �r�b�g7: FM�����{�[�h��(0:���݂��Ȃ�,1:���݂���)

	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = 17 * 256
	
	CALL interrupt(&H41, inregs, outregs)
	
	GETSTATUS = outregs.ax AND &HFF
END FUNCTION

SUB MEND
'���t�I��
	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = &H100
	CALL interrupt(&H41, inregs, outregs)
	
END SUB

SUB MESCOF
'�ꎞ��~����FM���������ĊJ���܂�
	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = &H400
	CALL interrupt(&H41, inregs, outregs)
	
END SUB

SUB MESCON
'���t����FM���������ꎞ��~���܂�
	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = &H300
	CALL interrupt(&H41, inregs, outregs)
	
END SUB

SUB MFADE (fmspeed, psgspeed)
'�t�F�[�h�A�E�g
	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = &H600
	inregs.bx = fmspeed * 256 + psgspeed
	CALL interrupt(&H41, inregs, outregs)
	
END SUB

SUB MIDICLOSE
'MIDI�����X�g�b�v������
	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = 21 * 256
	
	CALL interrupt(&H41, inregs, outregs)
END SUB

SUB MIDICONT
'MIDI�R���e�B�j���[

	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = 24 * 256
	
	CALL interrupt(&H41, inregs, outregs)
END SUB

SUB MIDIEXT
'MIDI�g�����^�C�x���g�I��
	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = 29 * 256
	
	CALL interrupt(&H41, inregs, outregs)
END SUB

SUB MIDIFADE (pstart, pend, speed)
'MIDI�t�F�[�h
'  pstart����pend�܂�speed���ԂŃx���V�e�B�␳�l��ύX����
'
	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = 25 * 256
	inregs.cx = speed
	inregs.dx = pend * 256 + pstart
	
	CALL interrupt(&H41, inregs, outregs)
END SUB

FUNCTION MIDIINIT (interface)
'MIDI���̏�����
' interface = 1: MPU-401, 2 = RS-232C
' �߂�l: 0 ����A0�ȊO�C���^�[�t�F�[�X�g�p�s��

	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = 20 * 256
	inregs.dx = interface
	CALL interrupt(&H41, inregs, outregs)
	
	MIDIINIT = outregs.ax
	
END FUNCTION

SUB MIDISEND (dat)
'MIDI��1�o�C�g����
	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = 30 * 256
	inregs.dx = dat
	
	CALL interrupt(&H41, inregs, outregs)
END SUB

FUNCTION MIDISTART (dat$)
'MIDI�X�^�[�g
'�߂�l: 0 = ����I���A 1 = �W��MIDI�t�@�C���łȂ��A -1 = �o�b�t�@������Ȃ�

	DIM inregs AS RegTypeX
	DIM outregs AS RegTypeX
	
	inregs.ax = 22 * 256
	inregs.bx = SADD(dat$)
	inregs.dx = 0
	inregs.cx = LEN(dat$)
	inregs.es = VARSEG(dat$)
	
	CALL interruptx(&H41, inregs, outregs)
	
	MIDISTART = outregs.ax
END FUNCTION

SUB MIDISTOP
'MIDI�X�g�b�v

	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = 23 * 256
	
	CALL interrupt(&H41, inregs, outregs)
END SUB

SUB MIDITEMPO (tempo)
'MIDI�e���|�␳
'�e���|��tempo/256.0�{�ɂ���
	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = 27 * 256
	inregs.dx = tempo
	
	CALL interrupt(&H41, inregs, outregs)
END SUB

FUNCTION MREAD (ch, workno)
'OPNDRV�̓������[�N�l�𓾂�
'   ch = �`���l��(1�`17)(16,17�͊��荞�݃`���l��)
'   workno = ���[�N�ԍ�

	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = &H800
	inregs.dx = workno * 256 + ch
	CALL interrupt(&H41, inregs, outregs)
	
	MREAD = outregs.ax \ 256
END FUNCTION

FUNCTION OPNCHECK
'OPNDRV�̃o�[�W�����ԍ���100�{��Ԃ�
	ID$ = "VRDNPO"
	
	DEF SEG = 0
	vectoffset = PEEK(&H41 * 4) + PEEK(&H41 * 4 + 1) * 256
	vectseg = PEEK(&H41 * 4 + 2) + PEEK(&H41 * 4 + 3) * 256
	
	DEF SEG = vectseg
	
	FOR i = 0 TO 5
		vectoffset = vectoffset - 1
		IF PEEK(vectoffset) <> ASC(MID$(ID$, i + 1, 1)) THEN
			OPNCHECK = 0
			EXIT FUNCTION
		END IF
	NEXT
	
	OPNCHECK = (PEEK(vectoffset - 1) - &H30) + (PEEK(vectoffset - 2) - &H30) * 10 + (PEEK(vectoffset - 4) - &H30) * 100
	
END FUNCTION

FUNCTION OPNSTART (dat$)
'OPN�t�@�C���̉��t
	DIM inregs AS RegTypeX
	DIM outregs AS RegTypeX
	
	IF MID$(dat$, 1, 3) <> "OPN" THEN
		OPNSTART = 0
		EXIT FUNCTION
	END IF

	OPNSTART = ASC(MID$(dat$, 4, 1)) * 100 + ASC(MID$(dat$, 5, 1))
	
	inregs.ax = 18 * 256
	inregs.bx = SADD(dat$) + 7
	inregs.dx = 0
	inregs.cx = LEN(dat$) - 7
	inregs.es = VARSEG(dat$)
	
	CALL interruptx(&H41, inregs, outregs)
	
	IF outregs.ax = -1 THEN
		OPNSTART = -1
	END IF

END FUNCTION

FUNCTION PCMPLAY (dat$)
'PCM�f�[�^�����t����
'
'�߂�l: ��PCM�`�� = -2, �o�b�t�@������Ȃ� = -1, ����I�� = PCM�f�[�^�o�[�W����'
	DIM inregs AS RegTypeX
	DIM outregs AS RegTypeX
	
	IF MID$(dat$, 1, 3) <> "PCM" THEN
		PCMPLAY = 0
		EXIT FUNCTION
	END IF
	
	PCMPLAY = ASC(MID$(dat$, 4, 1)) * 100 + ASC(MID$(dat$, 5, 1))
	
	inregs.ax = 42 * 256
	inregs.bx = SADD(dat$) + 5
	inregs.es = VARSEG(dat$)
	inregs.dx = 0
	inregs.cx = LEN(dat$) - 5
	
	CALL interruptx(&H41, inregs, outregs)
	
	IF outregs.ax <> 0 THEN
		PCMPLAY = outregs.ax
	END IF
	
END FUNCTION

SUB PCMSTOP
'PCM���~�߂�
	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = 41 * 256
	
	CALL interrupt(&H41, inregs, outregs)
END SUB

FUNCTION REGREAD (regno, sel)
'FM�����������W�X�^�̒l������
'regno = ���W�X�^�ԍ�, sel = �Z���N�^(0 = YM2203�݊�����, 1 = YM2608�݊�����)
'
	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = 16 * 256
	inregs.dx = sel * 256 + regno
	
	CALL interrupt(&H41, inregs, outregs)
	
	REGREAD = inregs.ax AND &HFF
	
END FUNCTION

SUB SESTART (num)
'�X�g�A�������ʉ���炷
'num = SE�ԍ�
	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = 2 * 256
	inregs.bx = num
	
	CALL interrupt(&H41, inregs, outregs)
	
END SUB

FUNCTION SESTORE (dat$)
'���ʉ�������o�b�t�@�ɃX�g�A����
'dat$=SE�t�@�C����ǂݍ��񂾐擪�|�C���^
'SE�f�[�^�łȂ����0��SE�f�[�^�ł���΁A�o�[�W������100�{�l��Ԃ�
'�o�b�t�@������Ȃ����ɂ�-1��Ԃ�
	DIM inregs AS RegTypeX
	DIM outregs AS RegTypeX

	IF MID$(dat$, 1, 3) <> "SE " THEN
		SESTORE = 0
		EXIT FUNCTION
	END IF
	
	
	inregs.ax = 19 * 256
	inregs.bx = SADD(dat$)
	inregs.es = VARSEG(dat$)
	inregs.dx = 0
	inregs.cx = LEN(dat$)
	
	CALL interruptx(&H41, inregs, outregs)
	
	IF outregs.ax = -1 THEN
		SESTORE = -1
		EXIT FUNCTION
	END IF

	SESTORE = ASC(MID$(dat$, 4, 1)) * 100 + ASC(MID$(dat$, 5, 1))
	
END FUNCTION

SUB TEMPOCHANGE (tempo, division)
'�e���|�`�F���W���܂�
	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = 15 * 256
	inregs.bx = 256 - 208200 / (division * tempo)
	CALL interrupt(&H41, inregs, outregs)
	
END SUB

FUNCTION VARREAD (num)
'�����ϐ��l�𓾂܂�
	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = 50 * 256
	inregs.dx = num
	CALL interrupt(&H41, inregs, outregs)
	
	VARREAD = outregs.ax AND 255
END FUNCTION
