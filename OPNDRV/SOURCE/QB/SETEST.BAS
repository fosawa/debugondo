'SE演奏サンプルプログラム
'$INCLUDE: 'OPNLIBQB.BI'

IF OPNCHECK = 0 THEN
	PRINT "OPNDRVが常駐していません"
	END
END IF

IF OPNCHECK < 200 THEN
	PRINT "OPNDRVのバージョンが古すぎます"
	END
END IF

INPUT "SEファイル名を入力してください"; filename$

PRINT "ロード中"

OPEN filename$ FOR BINARY AS #1
s$ = ""
DO WHILE NOT EOF(1)
	s$ = s$ + INPUT$(1, #1)
LOOP

status = SESTORE(s$)

IF status = 0 THEN
	PRINT "SE形式ではありません"
	END
END IF

IF status = -1 THEN
	PRINT "SE用バッファが足りません"
	PRINT (LEN(s$) + 1023) \ 1024; "Kバイト以上のバッファが必要です"
	END
END IF

PRINT "ロード終了"

maxse = GETSEMAX(s$)

INPUT "鳴らしたいSE番号を入力してください(nで終了)"; snum$
IF snum$ = "n" OR snum$ = "N" THEN END

DO
snum = VAL(snum$)
IF snum < 0 OR snum >= maxse THEN
	PRINT "SE番号が大きすぎます"
	ELSE
	sestart (snum)
END IF
INPUT "鳴らしたいSE番号を入力してください(nで終了)"; snum$
LOOP WHILE snum$ <> "n" AND snum$ <> "N"

CALL MEND

