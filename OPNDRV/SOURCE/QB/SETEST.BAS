'SE���t�T���v���v���O����
'$INCLUDE: 'OPNLIBQB.BI'

IF OPNCHECK = 0 THEN
	PRINT "OPNDRV���풓���Ă��܂���"
	END
END IF

IF OPNCHECK < 200 THEN
	PRINT "OPNDRV�̃o�[�W�������Â����܂�"
	END
END IF

INPUT "SE�t�@�C��������͂��Ă�������"; filename$

PRINT "���[�h��"

OPEN filename$ FOR BINARY AS #1
s$ = ""
DO WHILE NOT EOF(1)
	s$ = s$ + INPUT$(1, #1)
LOOP

status = SESTORE(s$)

IF status = 0 THEN
	PRINT "SE�`���ł͂���܂���"
	END
END IF

IF status = -1 THEN
	PRINT "SE�p�o�b�t�@������܂���"
	PRINT (LEN(s$) + 1023) \ 1024; "K�o�C�g�ȏ�̃o�b�t�@���K�v�ł�"
	END
END IF

PRINT "���[�h�I��"

maxse = GETSEMAX(s$)

INPUT "�炵����SE�ԍ�����͂��Ă�������(n�ŏI��)"; snum$
IF snum$ = "n" OR snum$ = "N" THEN END

DO
snum = VAL(snum$)
IF snum < 0 OR snum >= maxse THEN
	PRINT "SE�ԍ����傫�����܂�"
	ELSE
	sestart (snum)
END IF
INPUT "�炵����SE�ԍ�����͂��Ă�������(n�ŏI��)"; snum$
LOOP WHILE snum$ <> "n" AND snum$ <> "N"

CALL MEND
