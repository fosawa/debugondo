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
'OPN ファイルのdivisionを返す
'0で与えられたデータ列がOPN形式でない
	IF MID$(dat$, 1, 3) <> "OPN" THEN
		GETDIVISION = 0
		EXIT FUNCTION
	END IF

	GETDIVISION = ASC(MID$(dat$, 6, 1)) + ASC(MID$(dat$, 7, 1)) * 256
	
END FUNCTION

FUNCTION GETMIDI
'MIDI演奏状況を見る
'戻り値: 下位8bit 0 = 演奏中、1 = 待機中、 2 = 演奏終了
'    上位8bit ベロシティ補正値
	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = 26 * 256
	
	CALL interrupt(&H41, inregs, outregs)
	
	GETMIDI = outregs.ax
END FUNCTION

FUNCTION GETOPNVER (dat$)
'OPNファイルのバージョンを返す
'0で与えられたデータがOPN形式でないことを示す
'そうでなければ、データバージョンの100倍値を返す
	IF MID$(dat$, 1, 3) <> "OPN" THEN
		GETOPNVER = 0
		EXIT FUNCTION
	END IF
	
	GETOPNVER = ASC(MID$(dat$, 4, 1)) * 100 + ASC(MID$(dat$, 5, 1))
	
END FUNCTION

FUNCTION GETPCMVER (dat$)
'PCMデータのバージョンを得る。PCMデータでない場合には0
'
	IF MID$(dat$, 1, 3) <> "PCM" THEN
		GETPCMVER = 0
		EXIT FUNCTION
	END IF
	
	GETPCMVER = ASC(MID$(dat$, 4, 1)) * 100 + ASC(MID$(dat$, 5, 1))
END FUNCTION

FUNCTION GETSEMAX (dat$)
'SEファイルかどうかを調べる
'SEファイルならば格納されているSEナンバー最大値を、そうでなければ0を返す
	
	IF MID$(dat$, 1, 3) <> "SE " THEN
		GETSEMAX = 0
		EXIT FUNCTION
	END IF

	GETSEMAX = ASC(MID$(dat$, 6, 1))
	
END FUNCTION

FUNCTION GETSEVER (dat$)
'SEファイルのバージョンを返す
'SEファイルでなければ0を、SEファイルであればバージョンの100倍を返す

	IF MID$(dat$, 1, 3) <> "SE " THEN
		GETSEVER = 0
		EXIT FUNCTION
	END IF

	GETSEVER = ASC(MID$(dat$, 4, 1)) * 100 + ASC(MID$(dat$, 5, 1))

END FUNCTION

FUNCTION GETSTATUS
'各種ステータスを調べる
'   戻り値:
'
'   ビット0: FM音源再生 (0:再生中,1:停止中)
'   ビット1: PCM音源再生(0:再生中,1:停止中)
'   ビット4: ポート番号が(0:188h, 1:288h)
'   ビット5: PCM音源が(0:存在しない,1:存在する)
'   ビット6: YM2608が(0:存在しない,1:存在する)
'   ビット7: FM音源ボードが(0:存在しない,1:存在する)

	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = 17 * 256
	
	CALL interrupt(&H41, inregs, outregs)
	
	GETSTATUS = outregs.ax AND &HFF
END FUNCTION

SUB MEND
'演奏終了
	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = &H100
	CALL interrupt(&H41, inregs, outregs)
	
END SUB

SUB MESCOF
'一時停止中のFM音源部を再開します
	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = &H400
	CALL interrupt(&H41, inregs, outregs)
	
END SUB

SUB MESCON
'演奏中のFM音源部を一時停止します
	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = &H300
	CALL interrupt(&H41, inregs, outregs)
	
END SUB

SUB MFADE (fmspeed, psgspeed)
'フェードアウト
	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = &H600
	inregs.bx = fmspeed * 256 + psgspeed
	CALL interrupt(&H41, inregs, outregs)
	
END SUB

SUB MIDICLOSE
'MIDI部をストップさせる
	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = 21 * 256
	
	CALL interrupt(&H41, inregs, outregs)
END SUB

SUB MIDICONT
'MIDIコンティニュー

	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = 24 * 256
	
	CALL interrupt(&H41, inregs, outregs)
END SUB

SUB MIDIEXT
'MIDI拡張メタイベントオン
	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = 29 * 256
	
	CALL interrupt(&H41, inregs, outregs)
END SUB

SUB MIDIFADE (pstart, pend, speed)
'MIDIフェード
'  pstartからpendまでspeed時間でベロシティ補正値を変更する
'
	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = 25 * 256
	inregs.cx = speed
	inregs.dx = pend * 256 + pstart
	
	CALL interrupt(&H41, inregs, outregs)
END SUB

FUNCTION MIDIINIT (interface)
'MIDI部の初期化
' interface = 1: MPU-401, 2 = RS-232C
' 戻り値: 0 正常、0以外インターフェース使用不可

	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = 20 * 256
	inregs.dx = interface
	CALL interrupt(&H41, inregs, outregs)
	
	MIDIINIT = outregs.ax
	
END FUNCTION

SUB MIDISEND (dat)
'MIDIに1バイト送る
	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = 30 * 256
	inregs.dx = dat
	
	CALL interrupt(&H41, inregs, outregs)
END SUB

FUNCTION MIDISTART (dat$)
'MIDIスタート
'戻り値: 0 = 正常終了、 1 = 標準MIDIファイルでない、 -1 = バッファが足りない

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
'MIDIストップ

	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = 23 * 256
	
	CALL interrupt(&H41, inregs, outregs)
END SUB

SUB MIDITEMPO (tempo)
'MIDIテンポ補正
'テンポをtempo/256.0倍にする
	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = 27 * 256
	inregs.dx = tempo
	
	CALL interrupt(&H41, inregs, outregs)
END SUB

FUNCTION MREAD (ch, workno)
'OPNDRVの内部ワーク値を得る
'   ch = チャネル(1～17)(16,17は割り込みチャネル)
'   workno = ワーク番号

	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = &H800
	inregs.dx = workno * 256 + ch
	CALL interrupt(&H41, inregs, outregs)
	
	MREAD = outregs.ax \ 256
END FUNCTION

FUNCTION OPNCHECK
'OPNDRVのバージョン番号の100倍を返す
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
'OPNファイルの演奏
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
'PCMデータを演奏する
'
'戻り値: 非PCM形式 = -2, バッファが足りない = -1, 正常終了 = PCMデータバージョン'
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
'PCMを止める
	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = 41 * 256
	
	CALL interrupt(&H41, inregs, outregs)
END SUB

FUNCTION REGREAD (regno, sel)
'FM音源内部レジスタの値を見る
'regno = レジスタ番号, sel = セレクタ(0 = YM2203互換部分, 1 = YM2608互換部分)
'
	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = 16 * 256
	inregs.dx = sel * 256 + regno
	
	CALL interrupt(&H41, inregs, outregs)
	
	REGREAD = inregs.ax AND &HFF
	
END FUNCTION

SUB SESTART (num)
'ストアした効果音を鳴らす
'num = SE番号
	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = 2 * 256
	inregs.bx = num
	
	CALL interrupt(&H41, inregs, outregs)
	
END SUB

FUNCTION SESTORE (dat$)
'効果音を内部バッファにストアする
'dat$=SEファイルを読み込んだ先頭ポインタ
'SEデータでなければ0をSEデータであれば、バージョンの100倍値を返す
'バッファが足りない時には-1を返す
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
'テンポチェンジします
	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = 15 * 256
	inregs.bx = 256 - 208200 / (division * tempo)
	CALL interrupt(&H41, inregs, outregs)
	
END SUB

FUNCTION VARREAD (num)
'内部変数値を得ます
	DIM inregs AS RegType
	DIM outregs AS RegType
	
	inregs.ax = 50 * 256
	inregs.dx = num
	CALL interrupt(&H41, inregs, outregs)
	
	VARREAD = outregs.ax AND 255
END FUNCTION

