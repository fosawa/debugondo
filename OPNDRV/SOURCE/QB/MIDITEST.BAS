'MIDI演奏サンプルプログラム
'$INCLUDE: 'OPNLIBQB.BI'

IF OPNCHECK = 0 THEN
	PRINT "OPNDRVが常駐していません"
	END
END IF

IF OPNCHECK < 200 THEN
	PRINT "OPNDRVのバージョンが古すぎます"
	END
END IF

INPUT "MIDIファイル名を入力してください"; filename$

PRINT "ロード中"

OPEN filename$ FOR BINARY AS #1
s$ = ""
DO WHILE NOT EOF(1)
	s$ = s$ + INPUT$(1, #1)
LOOP

INPUT "インターフェースを指定してください(1=MPU401,2=RS232C)"; interfase
IF interfase <> 1 AND interfase <> 2 THEN
	PRINT "インターフェースの範囲が不正です"
	END
END IF

status = MIDIINIT(interfase)

IF status <> 0 THEN
	PRINT "指定したインターフェースは使用できませんﾌ"
	END
END IF

status = MIDISTART(s$)

IF status = 1 THEN
	PRINT "MIDI形式ではありません"
	END
END IF

IF status = -1 THEN
	PRINT "MIDI用バッファが足りません"
	PRINT (LEN(s$) + 1023) / 1024; "Kバイト以上のバッファが必要です"
	END
END IF

PRINT "演奏中です"
PRINT "何かキーを押すと終了します"

WHILE INKEY$ = "": WEND

CALL MIDISTOP

