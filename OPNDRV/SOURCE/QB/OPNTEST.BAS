'OPN演奏サンプルプログラム
'$INCLUDE: 'OPNLIBQB.BI'

IF OPNCHECK = 0 THEN
	PRINT "OPNDRVが常駐していません"
	END
END IF

IF OPNCHECK < 200 THEN
	PRINT "OPNDRVのバージョンが古すぎます"
	END
END IF

INPUT "OPNファイル名を入力してください"; filename$

PRINT "ロード中"

OPEN filename$ FOR BINARY AS #1
s$ = ""
DO WHILE NOT EOF(1)
	s$ = s$ + INPUT$(1, #1)
LOOP

status = OPNSTART(s$)

IF status = 0 THEN
	PRINT "OPN形式ではありません"
	END
END IF

IF status = -1 THEN
	PRINT "OPN用バッファが足りません"
	PRINT (LEN(s$) + 1023) / 1024; "Kバイト以上のバッファが必要です"
	END
END IF

PRINT "演奏中です"
PRINT "何かキーを押すと終了します"

WHILE INKEY$ = "": WEND

CALL MEND

