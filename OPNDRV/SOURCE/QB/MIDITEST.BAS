'MIDI���t�T���v���v���O����
'$INCLUDE: 'OPNLIBQB.BI'

IF OPNCHECK = 0 THEN
	PRINT "OPNDRV���풓���Ă��܂���"
	END
END IF

IF OPNCHECK < 200 THEN
	PRINT "OPNDRV�̃o�[�W�������Â����܂�"
	END
END IF

INPUT "MIDI�t�@�C��������͂��Ă�������"; filename$

PRINT "���[�h��"

OPEN filename$ FOR BINARY AS #1
s$ = ""
DO WHILE NOT EOF(1)
	s$ = s$ + INPUT$(1, #1)
LOOP

INPUT "�C���^�[�t�F�[�X���w�肵�Ă�������(1=MPU401,2=RS232C)"; interfase
IF interfase <> 1 AND interfase <> 2 THEN
	PRINT "�C���^�[�t�F�[�X�͈̔͂��s���ł�"
	END
END IF

status = MIDIINIT(interfase)

IF status <> 0 THEN
	PRINT "�w�肵���C���^�[�t�F�[�X�͎g�p�ł��܂����"
	END
END IF

status = MIDISTART(s$)

IF status = 1 THEN
	PRINT "MIDI�`���ł͂���܂���"
	END
END IF

IF status = -1 THEN
	PRINT "MIDI�p�o�b�t�@������܂���"
	PRINT (LEN(s$) + 1023) / 1024; "K�o�C�g�ȏ�̃o�b�t�@���K�v�ł�"
	END
END IF

PRINT "���t���ł�"
PRINT "�����L�[�������ƏI�����܂�"

WHILE INKEY$ = "": WEND

CALL MIDISTOP
