'OPN���t�T���v���v���O����
'$INCLUDE: 'OPNLIBQB.BI'

IF OPNCHECK = 0 THEN
	PRINT "OPNDRV���풓���Ă��܂���"
	END
END IF

IF OPNCHECK < 200 THEN
	PRINT "OPNDRV�̃o�[�W�������Â����܂�"
	END
END IF

INPUT "OPN�t�@�C��������͂��Ă�������"; filename$

PRINT "���[�h��"

OPEN filename$ FOR BINARY AS #1
s$ = ""
DO WHILE NOT EOF(1)
	s$ = s$ + INPUT$(1, #1)
LOOP

status = OPNSTART(s$)

IF status = 0 THEN
	PRINT "OPN�`���ł͂���܂���"
	END
END IF

IF status = -1 THEN
	PRINT "OPN�p�o�b�t�@������܂���"
	PRINT (LEN(s$) + 1023) / 1024; "K�o�C�g�ȏ�̃o�b�t�@���K�v�ł�"
	END
END IF

PRINT "���t���ł�"
PRINT "�����L�[�������ƏI�����܂�"

WHILE INKEY$ = "": WEND

CALL MEND
