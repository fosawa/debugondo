'PCM演奏サンプルプログラム
'$INCLUDE: 'OPNLIBQB.BI'

IF OPNCHECK = 0 THEN
	PRINT "OPNDRVが常駐していません"
	END
END IF

IF OPNCHECK < 200 THEN
	PRINT "OPNDRVのバージョンが古すぎます"
	END
END IF

INPUT "PCMファイル名を入力してください"; filename$

PRINT "ロード中"

OPEN filename$ FOR BINARY AS #1
s$ = ""
DO WHILE NOT EOF(1)
	s$ = s$ + INPUT$(1, #1)
LOOP

status = PCMPLAY(s$)

IF status = -2 THEN
	PRINT "PCM形式ではありません"
	END
END IF

IF status = -1 THEN
	PRINT "PCM用バッファが足りません"
	PRINT (LEN(s$) + 1023) / 1024; "Kバイト以上のバッファが必要です"
	END
END IF

PRINT "演奏中です"
PRINT "何かキーを押すと終了します"

WHILE INKEY$ = "": WEND

CALL PCMSTOP

