	;OPN DRIVERS	MIDI +
	;		with expander & Psg voice speaking system
	;		
	;		原作（８８版）	堀川豊史
	;		移植（９８版）	大澤文孝
	;
		JUMPS
	;
	
	;
	;返値
	;
	;ヘルプメッセージ	     ＦＣＨ
	;サウンドボード有り常駐した　１
	;サウンドボード無し常駐した　０
	;サウンドボードが86ボード    2
	;サウンドボード有り既に常駐　８１Ｈ
	;サウンドボード無し既に常駐　８０Ｈ
	;開放した　　　　　　　　　　ＦＦＨ
	;パラメータエラー　　　　　　ＦＥＨ
	;常駐していないのに／ｒ　　　ＦＤＨ
	
	
ver equ '2.00'

vect	equ 041h			;制御ベクタ
TIME_PCMSIZE	EQU 1024


DATAPORT	EQU	0E0D0h	;ＭＩＤＩインタフェースデータポート
STSCMDPORT	EQU	0E0D2h	;ＭＩＤＩインタフェースコマンド/ステータスポート
DRR		EQU	40h	;データ受信可能ビット取得マスク
DSR		EQU	80h	;データ送信可能ビット取得マスク
MPUACK		EQU	0FEh	;ＡＣＫデータ

CLK5MHZ		EQU 24576	;5MHzの時の10msに対するカウンタ値
;CLK5MHZ		EQU 29491	;5MHzの時の10msに対するカウンタ値
;CLK5MHZ		EQU 20000	;5MHzの時の10msに対するカウンタ値
CLK8MHZ		EQU 19968	;8MHzの時の10msに対するカウンタ値
		
MAXTRK		EQU 50		;最大50トラックまで対応

	DOSSEG
	
	
	.MODEL	TINY
	
	.CODE
	
	ORG 100H

	assume cs:@CODE,ds:@CODE, es:@CODE
	
progstart:
	jmp start

vectsetflag	db 	0
debug	db	0
se_storef	db	0

sebuffer	dw	?
fmbuffer	dw	?
midibuffer	dw	?
pcmbuffer	dw	?

sebuffersize	dw	0
fmbuffersize	dw	0
midibuffersize	dw	0
pcmbuffersize	dw	0

opnoffset	dw	?

pcmover	db	0
pcmoffset	dw	0, 0
pcmsize	dw	0, 0
pcm0send	dw	0

buf188	db 256 dup (0)
buf18a	db 256 dup (0)

useg	dw ?
seseg	dw	?		;効果音用のセグメント
tbf	db 0
var	db 100 dup (0)		;変数０－９９

ston 	db 00h			;カナ／ＣＡＰＳなし
shift	db ?
tflag	db 0

voicefdumy db 0			;必ず０
voicef	db 0
voicef2 db 0

voicedouble db 0
timerbplay  db 0

ioport1	dw	188h
ioport2	dw	18ah
ioport3	dw	18ch
ioport4	dw	18eh

bord86	db	0
ext86	db	0
pcmplas	db	0
pcm_j	dw	0
temposetflag	db	0

alg	db	0
ff	db	0

dummyopn	LABEL WORD
	rept	15
	dw	offset opn_dat - offset dummyopn
	endm
opn_dat	db 	81h, 7fh, 00h, 41h, 00h, 80h, 0a0h, 0f9h, 0ffh

jptab	dw offset tieoff
	dw offset tieon
	dw offset schang
	dw offset volum11
	dw offset volum2
	dw offset volum3
	dw offset keytim
	dw offset slide
	dw offset vibini
	dw offset error
	dw offset zurash
	dw offset fadein
	dw offset pend
	dw offset temp
	dw offset opnco1
	dw offset opnco2
	dw offset jump
	dw offset let
	dw offset addi
	dw offset subs
	dw offset ifthen
	dw offset ifthen2
	dw offset error ;96h
	dw offset error	;97h
	dw offset error	;98h
	dw offset error ;99h
	dw offset error ;9ah
	dw offset error ;9bh
	dw offset error ;9ch
	dw offset error ;9dh
	dw offset error ;9eh
	dw offset error ;9fh
	;
	dw offset jump2 ;a0h
	dw offset let2	;a1h
	dw offset addi2 ;a2h
	dw offset subs2 ;a3h
	dw offset iifthen ;a4h
	dw offset iifthen2 ;a5h
	dw offset error
	dw offset schang2	;a7h	(ver1.1以降でのみ対応)
	dw offset lfo		;ハードウェアLFO設定
	dw offset lr
	dw offset amspms
	dw offset lop
	dw offset b_lop
	dw offset d_vibini
	dw offset f_pcm
	dw offset f_pcmstop
	dw offset f_pcmstart
	dw offset iife
	dw offset iifp
	
rth_jptab	dw offset tieoff
	dw offset tieon
	dw offset error
	dw offset rth_volum
	dw offset volum2
	dw offset volum3
	dw offset keytim
	dw offset slide
	dw offset vibini
	dw offset error
	dw offset zurash
	dw offset fadein
	dw offset pend
	dw offset temp
	dw offset opnco1
	dw offset opnco2
	dw offset jump
	dw offset let
	dw offset addi
	dw offset subs
	dw offset ifthen
	dw offset ifthen2
	dw offset error ;96h
	dw offset error	;97h
	dw offset error	;98h
	dw offset error ;99h
	dw offset error ;9ah
	dw offset error ;9bh
	dw offset error ;9ch
	dw offset error ;9dh
	dw offset error ;9eh
	dw offset error ;9fh
	;
	dw offset jump2 ;a0h
	dw offset let2	;a1h
	dw offset addi2 ;a2h
	dw offset subs2 ;a3h
	dw offset iifthen ;a4h
	dw offset iifthen2 ;a5h
	dw offset error
	dw offset error		;a7h	(ver1.1以降でのみ対応)
	dw offset error
	dw offset error
	dw offset error
	dw offset lop
	dw offset b_lop
	dw offset error
	dw offset f_pcm
	dw offset f_pcmstop
	dw offset f_pcmstart
	dw offset iife
	dw offset iifp

algot	db 8,8,8,8,0ah,0eh,0eh,0fh

ftable	dw 26ah,28fh,2b6h,2dfh,30bh,339h
	dw 36ah,39eh,3d5h,410h,44eh,48fh
	
bordf	db 0

nch	db 0
pflag	db 1
allstopflag	dw	0
tempo	db 0
fmfrug	db 0
musicp	db 0
volkk	db 0
pcmflag	db 1

work:

	;FM音源
play1	db 20h dup (0)
play2	db 20h dup (0)
play3	db 20h dup (0)
	;SSG音源
play4	db 20h dup (0)
play5	db 20h dup (0)
play6	db 20h dup (0)
	;FM音源拡張用
play7	db 20h dup (0)
play8	db 20h dup (0)
play9	db 20h dup (0)

	;リズム音源用
play10	db 20h dup (0)
play11	db 20h dup (0)
play12	db 20h dup (0)
play13	db 20h dup (0)
play14	db 20h dup (0)
play15	db 20h dup (0)

	;割り込み用
int1	db 20h dup (0)	;割り込み用ワーク
int2	db 20h dup (0)


	
vecsoundoff dw ?
vecsoundseg dw ?

pftab	dw 0ee8h,0e12h,0d48h,0c88h,0bd4h,0b2ah
	dw 0a8ah,9f2h,964h,8dch,85eh,7e6h

musicvect db ?
maskf	db ?

pjptab	dw offset ptieof
	dw offset ptieon
	dw offset penvch
	dw offset penvof
	dw offset evolum
	dw offset espeed
	dw offset pnset
	dw offset pslide
	dw offset pvibr
	dw offset pnt
	dw offset pzura
	dw offset pfouts
	dw offset pend
	dw offset pnslid
	dw offset opnco1
	dw offset opnco2
	dw offset jump
	dw offset let
	dw offset addi
	dw offset subs
	dw offset ifthen
	dw offset ifthen2
	dw offset pinte
	dw offset stemp	
	dw offset henvsw
	dw offset henvsp
	dw offset henvfm
	dw offset exon
	dw offset exoff
	dw offset settimera
	dw offset ips ;9eh
	dw offset voice ;9fh
	;
	dw offset jump2 ;a0h
	dw offset let2	;a1h
	dw offset addi2 ;a2h
	dw offset subs2 ;a3h
	dw offset iifthen ;a4h
	dw offset iifthen2 ;a5h
	dw offset voice2ch ;a6h
	dw offset penvch2	;a7h (ver1.1以降でのみ対応)
	dw offset temp
	dw offset error
	dw offset error
	dw offset lop
	dw offset b_lop
	dw offset error
	dw offset f_pcm
	dw offset f_pcmstop
	dw offset f_pcmstart
	dw offset iife
	dw offset iifp

default_fm	db 113,49,113,49,15,20,0,0,84,91,81,88,1,3,10,10,2,2,5,8,35,32,23,23,0,0,0,0,52

invmask	db ?
fmply	db ?

tn	db 03fh
s3work	db 0c3h		;ret code

timera	dw 656
ptimera dw 656

exflag	db 0
tf	db ?

voicestate:

v1off	dw ?
v1seg	dw ?
v1byte	dw ?
v1sel	db ?

v2off	dw ?
v2seg	dw ?
v2byte	dw ?
v2sel	db ?

	;MIDI + ワークエリア追加分
	
midiext		db	0	;標準MIDIextender off

markt		label 	dword
		rept	MAXTRK
		dw	?
		dw	?
		endm

header		label
leng		dd	?
format		dw	?
ntrks		dw	?
division	dw	?
addrs		label 	word
		rept	MAXTRK
		dw	?
		dw	?
		endm
miditempo	dd	?
int08off	dw	?
int08seg	dw	?
int02off	dw	?
int02seg	dw	?
int0coff	dw	?
int0cseg	dw	?

port2		db 	?
gate		dw	?
clk		dw	?
timer		dw	?
midishift	db	?
timef8		dw	?

midisub		dw	?
		dw	?
		
counts		label	word
		rept	MAXTRK
		dw	0
		dw	0
		db	80h		;ランニングステータス用
		db	0		;予備
		endm

cmd		label	byte
		; 0  -  07fh
		rept	80h
		db	0
		endm
		; 80h
		rept	10h
		db	2
		endm
		; 90h
		rept	10h
		db	2
		endm
		; a0h
		rept	10h
		db	2
		endm
		; b0h
		rept	10h
		db	2
		endm
		; c0h
		rept	10h
		db	1
		endm
		; d0h
		rept	10h
		db	1
		endm
		; e0h
		rept	10h
		db	2
		endm
		; f0h
		db	?
		db	1
		db	2
		db	1
		db	?
		db	?
		db	0
		db	0
		db	0
		db	?
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		
buf		db	1030 dup (?)
buflen		dw	?
bufaddr		dw	?
popaddr		dw	?
stop		db	1

port35		db	?
inter		db	?
m_init		db	0

note		db	16 dup (0)	;note on/off情報

		;fade in/out用ワーク
E		dw	?
dltx		dw	?
dlty		dw	?
velo		dw	?
gtime		dw	?
sy		dw	?
dtempo		dw	256
playtrk		dw	?

opnwrt:	
	push	di
	push	dx
	pushf
	
	
	mov	dx, [ioport1]
	mov	di, offset buf188
	
	cmp	[ext86], 0
	jz	normalfm
	
	mov	dx, [ioport3]
	mov	di, offset buf18a
	
	cmp	bh, 30h
	jae	ext_port
	
	;レジスタ番号が0-2fhの時にはioport1を使う
	mov	dx, [ioport1]
	mov	di, offset buf188
	
	cmp	bh, 28h
	jnz	ext_port
	
	;レジスタ番号が28h(キーON/OFF)の時には、ビット2を立てる(チャンネル4-6)
	or	bl, 4
	
ext_port:
	cmp	[bord86], 0
	jnz	normalfm
	
	push	ax
	mov	ah, 0
	mov	al, bh
	add	di, ax
	mov	[di], bl
	pop	ax
	
	popf
	pop	dx
	pop	di
	
	ret	;86ボードなしで拡張が呼ばれたらret
	
normalfm:
opnwrt1:
	in al,dx
	and al,80h
	jnz opnwrt1
	
	jmp $+2
	jmp $+2
	
	;レジスタ選択
	mov al,bh
	out dx,al
	
	jmp $+2
	jmp $+2
	jmp $+2
	jmp $+2
	
	;2.1μsのウェイト
	rept	4
	out	5fh, al
	endm
	
	
opnwrt2:in al,dx
	and al,80h
	jnz opnwrt2
	
	;データ出力
	mov al,bl
	add dx, 2
	out dx,al
	
	;ウェイトを入れる
	cmp	bh, 0fh
	jbe	wait21
	cmp	bh, 10h
	jz	wait72
	cmp	bh, 9eh
	jbe	wait194
	
	;アドレスa0-b6
	;5.9μsのwaite
	rept	10
	out	5fh, al	;0.6
	endm
	jmp	opnwrt3
wait21:
	;アドレス0-0f
	;2.1μsのウェイト
	rept	4
	out	5fh, al
	endm
	jmp	opnwrt3
	
wait72:
	;アドレス10
	;72.0μsのウェイト
	rept	120
	out	5fh, al
	endm
	jmp	opnwrt3
	
wait194:
	;アドレス11-9e
	;19.2μsのウェイト
	rept	33
	out	5fh, al
	endm
	jmp	opnwrt3
	
opnwrt3:
	
	push	ax
	mov	ah, 0
	mov	al, bh
	add	di, ax
	mov	[di], bl
	pop	ax
	
	popf
	pop dx
	pop di
	
	ret

mididmy		PROC	NEAR
		;midicallのダミールーチン
		
		;キャリーを立てずに戻って何も処理しないことをしめす
		
		clc
		retf	;NEARのくせにfarで戻るなんて卑怯！
		
mididmy		ENDP

midijump	PROC	NEAR
		;midiファイルエキスパンダ
		;
		;エキスパンダをonにすることで、次のメッセージを取り込む
		;
		;ff 07 04 'M''a''r''k'
		;		->アドレスマーク
		;ff 07 04 'J''u''m''p'
		;		->マークアドレスにジャンプ
		;
		;マークは、各トラック当たり1個
		;
		;ゲームには、無限ループ音楽が不可欠であることから用意
		;
		
		cmp	al, 0ffh
		jnz	midijumpret
		
		cmp	byte ptr [di + 1], 7
		jnz	midijumpret
		
		cmp	byte ptr [di + 2], 4
		jnz	midijumpret
		
		;Markかどうか
		
		cmp	byte ptr [di + 3], 'M'
		jnz	notmark
		cmp	byte ptr [di + 4], 'a'
		jnz	notmark
		cmp	byte ptr [di + 5], 'r'
		jnz	notmark
		cmp	byte ptr [di + 6], 'k'
		jnz	notmark
		
		;Markである
		
		mov	bx, cx
		sal	bx, 1
		sal	bx, 1
		mov	es:[word ptr offset markt + bx], di
		mov	es:[word ptr offset markt + bx + 2], ds
		
		jmp	midijumpret
		
notmark:
		;Jumpか?
		cmp	byte ptr [di + 3], 'J'
		jnz	midijumpret
		cmp	byte ptr [di + 4], 'u'
		jnz	midijumpret
		cmp	byte ptr [di + 5], 'm'
		jnz	midijumpret
		cmp	byte ptr [di + 6], 'p'
		jnz	midijumpret
		
		;jumpである
		
		mov	bx, cx
		sal	bx, 1
		sal	bx, 1
		
		lds	di, es:[offset markt + bx]
		
midijumpret:
		clc
		ret
		ENDP
		

init232c	PROC	NEAR
		;RS-232Cのイニシャライズ
		;割り込み状態を保存
		pushf
		
		push	ds
		push	es
		
		mov	ax, es
		mov	ds, ax
		
		mov	ax, 350ch	;rs232c
		int	21h
		
		cmp	[m_init], 1
		jz	setinit232c_1

		mov	cs:[int0coff], bx
		mov	ax, es
		mov	cs:[int0cseg], ax
		
setinit232c_1:

		mov	ax, cs
		mov	ds, ax
		mov	dx, offset rssend
		mov	ax, 250ch
		int	21h
		
		pop	es
		
		cli
		in	al, 2
		cmp	cs:[m_init], 1
		jz	setinit232c_2
		mov	es:[port2], al
setinit232c_2:	
		and	al, 0efh
		out	2, al
		
		in	al, 35h
		jmp	$+2
		jmp	$+2
		jmp	$+2
		jmp	$+2
		cmp	cs:[m_init], 1
		jz	setinit232c_3
		mov	es:[port35],al
setinit232c_3:
		and	al, 0f8h	;受信／送信割り込み禁止
		out	35h, al
		
		;通信速度の設定
		
		;クロック判定
		push	es
		xor	ax, ax
		mov	es, ax
		mov	al, es:[0501h]
		pop	es
		test	al, 80h
		jz	clk5
		;8MHzクロック(1.9968MHz)
		mov	dx, 4	;分周比 4 ( = 31.20 KHz)
		jmp	clkpass
clk5:
		;5MHzクロック(2.4576MHz)
		mov	dx, 5	;分周比 5 ( = 30.72 KHz)
		
clkpass:
		mov	al, 0b6h
		out	77h, al	;mode 3
		jmp	$+2
		jmp	$+2
		jmp	$+2
		jmp	$+2
		mov	al, dl
		out	75h, al
		jmp	$+2
		jmp	$+2
		jmp	$+2
		jmp	$+2
		mov	al, dh
		out	75h, al
		
		;モード設定
		
		;ダミーコマンド3回
		rept	3
		mov	al, 0
		out	32h, al
		;リカバリタイムウェイト
		mov	cx, 32
		loop	$
		endm
		
		;8251 reset
		mov	al, 40h
		out	32h, al
		;リカバリタイムウェイト
		mov	cx, 32
		loop	$
		
		;
		mov	al, 01001110B
		out	32h, al
		;リカバリタイムウェイト
		mov	cx, 32
		loop	$
		
		;ER/RS ON
		mov	al, 37h
		out	32h, al
		
		pop	ds
		popf
		
		ret
		
init232c	ENDP

initmpu		PROC	NEAR
		;MPU401 ＭＩＤＩインタフェースを初期化する
		push	cx
		push	dx
		pushf
		cli
		;データ受信可能になるまで待つ
		xor	cx, cx
wait_r_ready:	mov	dx, stscmdport
		in	al, dx
		test	al, drr
		jz	r_ready
		mov	dx, dataport
		in	al, dx
		loop	wait_r_ready
		jmp	time_out_r
r_ready:	mov	al, 0ffh
		;MIDIインタフェースをリセットする
		out	dx, al
		;データ送信可能になるまで待つ
		xor	cx, cx
wait_s_ready:	mov	dx, stscmdport
		in	al, dx
		test	al, dsr
		jnz	s_ready
		mov	dx, dataport
		in	al,dx
		loop	wait_s_ready
		jmp	time_out_s
s_ready:	mov	dx,stscmdport
dummy_wait:	in	al,dx
		test	al,drr
		jnz	dummy_wait
		;MIDIインタフェースをＵＡＲＴモードに設定する
		mov	dx, stscmdport
		mov	al, 3fh
		out	dx, al
		mov	bx,100h
		;データ送信可能になるまで待つ
		xor	cx, cx
wait_s_ready2:	mov	dx,stscmdport
		in	al,dx
		test	al,dsr
		jz	data_s_ready2
		loop	wait_s_ready2
		dec	bx
		jnz	wait_s_ready2
		jmp	time_out_a
		;ACKデータ(FE)を受信するまで待つ
data_s_ready2:	mov	dx,dataport
		in	al,dx
		cmp	al,mpuack
		jne	wait_s_ready2
		clc
		xor	ax,ax
midi_reset_e:	
		popf
		pop	dx
		pop	cx
		ret
time_out_r:	mov	ax,1
		jmp	midi_reset_e
time_out_s:	mov	ax,2
		jmp	midi_reset_e
time_out_a:	mov	ax,3
		jmp	midi_reset_e
initmpu		ENDP

outdata2	macro
		local	@@2
		
		push	dx
		push	di
		push	ax
		
		mov	di, es:[bufaddr]

		stosb
		cmp	di, offset buf + 1024
		jnz	@@2
		mov	di, offset buf
@@2:
		mov	es:[bufaddr], di
		inc	es:[buflen]

		;RS-232C送信割り込み許可
		
		mov	al, 5
		out	37h, al
		
		pop	ax
		pop	di
		pop	dx
		
		endm

mpusend		PROC	NEAR
		;MPU401インターフェースにバッファ内容を送信する
		
		mov	si, [popaddr]
		mov	cx, [buflen]
rrp:
		jcxz	rrp4
		
		mov	dx, stscmdport
		push	cx
		mov	cx, 60000
		;データ受信可能まで待つ
rrp2:		in	al, dx
		test	al, drr
		jz	rrpok1
		loop	rrp2
		pop	cx
		jmp	rrp4
rrpok1:		
		pop	cx
		lodsb
		mov	dx, dataport
		out	dx, al
		
		dec	cx
		cmp	si, offset buf + 1024
		jnz	rrp3
		mov	si, offset buf
rrp3:
		mov	[popaddr], si
		dec	[buflen]
		jmp	rrp
rrp4:
		ret
		
mpusend		ENDP

rssend		PROC	FAR
		
		push	ax
		push	cx
		push	si
		push	ds
		
		mov	ax, cs
		mov	ds, ax
		
		;RS-232Cにバッファ内のデータを送信
		
		mov	si, [popaddr]
		mov	cx, [buflen]
rsrrp:
		jcxz	rsrrp4
		
		;データ受信可能かどうか
rsrrp2:		in	al, 32h
		test	al, 1
		jnz	rsrrpok1
		
		jmp	rsrrp4
rsrrpok1:		
		
		lodsb
		
		out	30h, al
		
		dec	cx
		cmp	si, offset buf + 1024
		jnz	rsrrp3
		mov	si, offset buf
rsrrp3:
		mov	[popaddr], si
		dec	[buflen]
		;jmp	rsrrp
rsrrp4:
		cmp	[buflen],0
		jnz	rsrrp5
		;バッファが空になったので、割り込みを禁止する
		mov	al, 4
		out	37h, al
rsrrp5:
		;EOI発行
		mov	al, 20h
		out	0, al
		
		pop	ds
		pop	si
		pop	cx
		pop	ax
		
		iret
rssend		ENDP

closemidi	PROC
		pushf
		push	ax
		push	dx
		push	ds
		mov	ax, cs
		mov	ds, ax

		cmp	[m_init], 1
		jnz	noclose
		cli
		mov	[stop], 1
		mov	[m_init], 0
		;all note off
		call	alloff

		;タイマ割り込みを禁止
		in	al, 02
		or	al, 01h
		out	02h, al
		
		;ベクタセット
		push	ds
		mov	ax, 2508h
		lds	dx, [dword ptr int08off]
		int	21h
		pop	ds
		
		test	[inter], 1
		jz	c232
		;MPU close
		;バッファー内のデータを出力
		call	mpusend
		call	initmpu
c232:
		test	[inter], 2
		jz	noc232
		
		;RS-232C close
		;バッファー内のデータを出力
		
		push	si
		push	cx
		
		mov	si, [popaddr]
		mov	cx, [buflen]
_rsrrp:
		jcxz	_rsrrp4
		
		push	cx
		mov	cx, 60000
		;データ受信可能かどうか
_rsrrp2:	in	al, 32h
		test	al, 1
		jnz	_rsrrpok1
		loop	_rsrrp2
		pop	cx
		jmp	_rsrrp5
_rsrrpok1:		
		pop	cx
		lodsb
		out	30h, al
		dec	cx
		cmp	si, offset buf + 1024
		jnz	_rsrrp3
		mov	si, offset buf
_rsrrp3:
		mov	[popaddr], si
		dec	[buflen]
		jmp	_rsrrp
_rsrrp4:
		;バッファが空になったので、割り込みを禁止する
		mov	al, 4
		out	37h, al
_rsrrp5:

		pop	cx
		pop	si
		
		call	close232c
noc232:
		;タイマ割り込みを戻す
		mov	al, [port2]
		out	02h, al
noclose:
		pop	ds
		pop	dx
		pop	ax
		popf
		ret
closemidi	ENDP

close232c	PROC	NEAR
		push	ds
		lds	dx,[dword ptr int0coff]
		mov	ax, 250ch
		int	21h

		mov	ax, cs
		mov	ds, ax
		
		mov	al,[port35]
		out	35h, al
		
		pop	ds

		ret
		
close232c	ENDP

incdi		macro
		push	ax
		mov	ax, ds
		add	di, 1	;inc	diは、不可(CY不変)
		adc	ax, 0
		mov	ds, ax
		pop	ax
		endm
		
int08		PROC	FAR
		
		;MIDI演奏部
		;タイマ割り込みで演奏する
		push	ax
		push	bx
		push	cx
		push	dx
		push	di
		push	si
		push	ds
		push	es
		
		cld
		
repeat:
		mov	ax, cs
		mov	es, ax
		mov	ds, ax
		
		;気持ちだけMIDI CLK f8hに対応
		;あんまり、正確なタイミングで発生しないけど
		cmp	es:[timef8], 0
		jnz	timf8_1
		mov	al, 0f8h
		outdata2
timf8_1:
		inc	es:[timef8]
		mov	ax, es:[division]
		cmp	es:[timef8], ax
		jbe	timef8_2
		mov	es:[timef8], 0
timef8_2:
		
		;EOIの発行
		
		mov	al, 20h
		out	0h, al
		
		;タイマ割り込みマスク
		in	al, 2
		or	al, 1
		out	2, al
		
		sti
		
		;シフトキー状態の入手
		mov	ah, 2
		int 	18h
		
		mov 	es:[shift], al
		mov  	ah, es:[ston]
		and 	al, ah
		jz 	_noshift
		;キーロックによる抑制なら、キーオフにする
		
		;all key note off
		call	alloff
_noshift:	
		;MIDI送信
		
		
		test	[inter], 2
		jnz	inter232
		;MPUインターフェース
		call	mpusend
		jmp	no_re
inter232:
		;RS-232Cインターフェース
		;	割り込みで処理するのでここでは送信しない
no_re:
		cmp	es:[stop], 0
		jnz	pass
		
		;fade in / outのベロシティーの処理
		cmp	es:[gtime], 0
		jz	novelo
		dec	es:[gtime]
		mov	ax, es:[dlty]
		add	es:[E], ax
velo_2:
		jl	velo_1
		mov	ax, es:[sy]
		add	es:[velo], ax
		mov	ax, es:[dltx]
		sub	es:[E], ax
		jmp	velo_2
velo_1:
		
novelo:		
		;演奏開始
		mov	cx, es:[ntrks]
		
		mov	si, offset counts
		mov	bx, offset addrs
		mov	es:[playtrk], 0
playlop:
		push	bx
		push	cx
		push	si
		push	es
		
		cmp	byte ptr es:[si+5], 0
		jnz	trkplayend
		mov	ax, es:[si]
		mov	dx, es:[si + 2]
		sub	ax, 1
		sbb	dx, 0
		mov	es:[si], ax
		mov	es:[si + 2], dx
		or	ax, dx
		jz	midiplay1
		test	dx, 8000h
		jnz	midiplay1
		
		jmp	next
		
trkplayend:
		inc	es:[playtrk]
		mov	ax, es:[ntrks]
		cmp	ax, es:[playtrk]
		jnz	next
		mov	es:[stop], 2
		
		call	alloff		;all note off
		mov	al, 0fch	;stop command
		outdata2
		
		jmp	next
midiplay1:
		
		;そのトラックのデータアドレスを取得
		lds	di, es:[bx]
trklop:
		
		;イベントコード取得
		mov	al,[di]
		
		push	ax
		push	bx
		push	cx
		push	si
		push	es
		
		
		mov	dx, offset _retaddr
		push	cs
		push	dx
		
		push	es:[word ptr midisub + 2]
		push	es:[word ptr midisub]
		
		neg	cx
		add	cx, es:[ntrks]
		
		retf
_retaddr:
		pop	es
		pop	si
		pop	cx
		pop	bx
		pop	ax
		
		jc	nextdata
		
		cmp	es: [midiext], 0
		jz	_noext
		
		push	bx
		push	cx
		
		neg	cx
		add	cx, es:[ntrks]
		
		call	midijump
		
		pop	cx
		pop	bx
_noext:
		mov	al, [di]
		incdi
		
		
		cmp	al, 0ffh
		jz	metaivent
		
		cmp	al, 0f0h
		jz	sysexevent
		
		cmp	al, 0f7h
		jz	sysexevent
		
		;普通のMIDIデータ
		
		;ランニングステータスかどうかのチェック
		test	al,80h
		jnz	norunning
		mov	al, es:[si + 4]
		sub	di, 1
		push	ax
		mov	ax, ds
		sbb	ax, 0
		mov	ds, ax
		pop	ax
		
norunning:
		;alを送る
		outdata2
run_1:
		;ランニングステータス保存
		mov	es:[si + 4], al

		;note on/offならば、情報を保存
		mov	ah, al
		and	ah, 0f0h
		
		cmp	ah, 90h
		jnz	nonoteon
		;note on
		push	ax
		push	bx
		mov	bx, ax
		and	bx, 0fh
		mov	es:[note + bx],1
		;fadein/out処理のため、独自の処理になる
		outdata2
		mov	al,[di]
		incdi
		outdata2
		
		mov	al,[di]
		incdi
		
		;shiftキーによる抑制があるかどうか
		
		push	ax
		mov 	al, es:[shift]
		mov  	ah, es:[ston]
		and 	al, ah
		pop	ax
		jz 	_noshift2
		mov	al, 0
		jmp	_shift2
_noshift2:
		;フェード処理する
		mov	ah, byte ptr es:[velo]
		mul	ah
		rept	7
		shr	ax, 1
		endm
		
_shift2:
		outdata2
		
		
		pop	bx
		pop	ax
		jmp	nextdata
		
nonoteon:
		cmp	ah, 80h
		jnz	nonoteoff
		;note off
		push	bx
		mov	bx, ax
		and	bx, 0fh
		mov	es:[note + bx],0
		pop	bx
nonoteoff:
		;コマンドがいくつの引き数を持つかをチェック
		push	bx
		xor	bx, bx
		mov	bl, al
		push	cx
		
		mov	cl, es:[offset cmd + bx]
		mov	ch, 0
		
		;引き数を送る
		jcxz	int08_3
int08_2:		
		mov	al,[di]
		incdi
		outdata2
		loop	int08_2
int08_3:
		pop	cx
		pop	bx
		
		
		jmp	nextdata
metaivent:
		;メタイベント
		
		
		mov	al,[di]
		incdi
		
		cmp	al,2fh
		jnz	meta_1
		
		cmp	byte ptr [di],0
		jnz	meta_1
		
		;トラックのおわり(ff 2f 00)
		incdi
		mov	byte ptr es:[si + 5], 1
		
		
		jmp	nexttrk
meta_1:
		
		cmp	al,51h
		jnz	meta_2
		
		cmp	byte ptr [di],3
		jnz	meta_2
		
		;テンポ変更
		incdi
		
		;テンポのロード
		mov	dh, 0
		mov	dl, [di]
		incdi
		
		mov	ah, [di]
		incdi
		mov	al, [di]
		incdi
		
		mov	word ptr es:[miditempo + 0], ax
		mov	word ptr es:[miditempo + 2], dx
		
		;
		;(これを1000*divisionで割る)
		
		push	bx
		mov	bx, 10000
		div	bx
		
		mov	dx, es:[clk]
		mul	dx
		
		mov	bx, es:[division]
		
		;除算エラーが出ないようにする
		;dx:ax / bx < 65535 ならばOK
		;ということは、
		;dx:ax < bx << 16  ならばOK
		;おおざっぱに言って
		;dx < bxならばOK
		mov	es:[midishift],0
divlp:
		cmp	dx,bx
		jb	divok
		shr	dx,1
		rcr	ax,1
		inc	es:[midishift]
		jmp	divlp
divok:
		div	bx
		
		;axに、dtempoを掛けて、256で割る
		mov	dx, es:[dtempo]
		mul	dx
		mov	al, ah
		mov	ah, dl
		mov	dl, dh
		
divlp2:
		cmp	dl, 0
		jz	divokp
		shr	dl, 1
		rcr	ax, 1
		inc	es:[midishift]
		jmp	divlp2
divokp:
		
		mov	es:[timer], ax
		
		mov	bx, ax
		mov	al, bl
		out	71h, al
		jmp	$+2
		jmp	$+2
		jmp	$+2
		jmp	$+2
		mov	al, bh
		out	71h, al
		
		pop	bx
		jmp	nextdata
meta_2:


meta_else:
		
		;対応しないメタイベント
		
		;空読みする
		;データ長の取得
		xor	ax, ax
lop3:
		mov	dl, [di]
		incdi
		rept	7
		sal	ax, 1
		endm
		push	dx
		and	dl, 07fh
		add	al, dl
		adc	ah, 0
		pop	dx
		test	dl, 80h
		jnz	lop3
		
		add	di, ax
		jmp	nextdata
		

sysexevent:	
		;システム拡張イベント(f0 ?? ... / f7 ?? ...)
		;
		
		cmp	al,0f0h
		jnz	sys_1
		
		;0f0hコードをMIDIに送る
		outdata2
sys_1:
		;データ長の取得（可変長7bit）
		xor	ax, ax
lop4:
		mov	dl, [di]
		incdi
		rept	7
		sal	ax, 1
		endm
		push	dx
		and	dl, 07fh
		add	al, dl
		adc	ah, 0
		pop	dx
		test	dl, 80h
		jnz	lop4
		
		push	cx
		mov	cx, ax
		jcxz	nextdatar
lop5:
		mov	al,[di]
		incdi
		outdata2
		loop	lop5
nextdatar:
		pop	cx
		jmp	nextdata

		
nextdata:
		;次のデータを読む
		;ゲートタイムの取得（可変長7bit）
		
		push	bx
		xor	ax, ax
		xor	dx, dx
lop2:
		mov	bl, [di]
		incdi
		rept	7
		sal	ax, 1
		rcl	dx, 1
		endm
		push	bx
		and	bl, 07fh
		add	al, bl
		adc	ah, 0
		adc	dx, 0
		pop	bx
		test	bl, 80h
		jnz	lop2
		
		pop	bx
		
		push	cx
		mov	cl,es:[midishift]
		mov	ch,0
		jcxz	rotpass
cllop:
		shl	ax,1
		rcl	dx,1
		loop	cllop
rotpass:
		pop	cx
		
		mov	es:[bx], di
		push	ax
		mov	ax, ds
		mov	es:[bx+2], ax
		pop	ax
		
		mov	es:[si], ax
		mov	es:[si + 2], dx
		
		or	ax, dx
		jz	midiplay1
		
		test	dx, 8000h
		jnz	midiplay1
		
		jmp	nexttrk
		
		
nexttrk:
next:
		pop	es
		pop	si
		pop	cx
		pop	bx
		
		add	si, 6
		add	bx, 4
		
		loop	playlop
pass:
		;タイマー割り込みを要求しているか
		mov	al, 0ah		;IRR read command
		out	0, al
		jmp	$+2
		jmp	$+2
		jmp	$+2
		jmp	$+2
		in	al, 0
		test	al, 1
		jnz	passr	;かかってた
		jmp	passn
passr:
		;タイマー割り込みがタイマー割り込み処理中にかかってしまった
		;jmp	repeat
passn:
		;タイマ割り込み許可
		cli
		in	al, 2
		and	al, 0feh
		out	2, al
		
		pop	es
		pop	ds
		pop	si
		pop	di
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		iret
int08		ENDP

alloff		PROC	NEAR
		;MIDI all note off
		push	ax
		push	bx
		push	cx
		push	ds
		push	es
		
		cld
		
		mov	ax, cs
		mov	ds, ax
		mov	es, ax
		
		mov	bx, 0
		mov	cx, 16
allop:
		cmp	byte ptr [note + bx],0
		jz	allnext
		;all note offを送信
		mov	al, 0b0h
		add	al, bl
		outdata2
		mov	al, 123
		outdata2
		mov	al, 0
		outdata2
allnext:
		inc	bx
		loop	allop
		
		pop	es
		pop	ds
		pop	cx
		pop	bx
		pop	ax
		ret
		
alloff		ENDP

r_midiplay:	
		;MIDIの演奏をする(常駐版)
		;	es:bx <- MIDIデータアドレス
		;	dx:cx <- データサイズ
		
		add	cx, 15
		adc	dx, 0
	
		rept	4
		shr	dx, 1
		rcr	cx, 1
		endm
	
		cmp	cx, [midibuffersize]
		jbe	r_midi2
	
		;メモリが足りない
		pop bx		;return addr dummy
		pop ax		
	
		pop es
		pop ds
		pop si
		pop di
		pop dx
		pop cx
		pop bx
	
		mov	ax, 0ffffh
		iret
	
r_midi2:
		push	di
		push	si
		
		push	ds
		push	es
	
		mov	si, bx
		mov	di, 0
	
		mov	ax, es
		mov	ds, ax
	
		mov	es, cs:[midibuffer]
	
r_midi3:
		cld
		rept	16
		movsb
		endm
		loop	r_midi3
	
		pop	es
		pop	ds
		pop	si
		pop	di
	
		mov	es, [midibuffer]
		mov	bx, 0
		
		jmp	midiplay
		
		
midiplay	PROC	NEAR
		;MIDIの演奏を開始するルーチン
		;		es:bx <- MIDIデータのトップアドレス
		
		push	es
		pop	ds
		
		cld
		mov	ax, cs
		mov	es, ax
		
		;
		cmp	es:[m_init], 0
		jnz	midip_ok
		;イニシャライズされていない
		mov	ax, 2
		jmp	nook
		
midip_ok:
		;停止中であることを確認
		cmp	es:[stop], 1
		jz	midip_stop_ok
		
		call	midistop
		
midip_stop_ok:
		push	bx
		pop	si
		
		;念のため、ds:siを、dsができるだけ大きくなるようにしておく
		mov	ax, ds
		push	si
		shr	si, 1
		shr	si, 1
		shr	si, 1
		shr	si, 1
		add	ax, si
		pop	si
		and	si, 0fh
		mov	ds, ax
		
		;ヘッダブロックの読みだし
		
		;識別子チェック
		cmp	byte ptr [si + 0], 'M'
		jz	normalmidi
		cmp	byte ptr [si + 128], 'M';MAC binaryが付いているか
		jnz	midierror
		add	si, 128
normalmidi:
		cmp	byte ptr [si + 1], 'T'
		jnz	midierror
		cmp	byte ptr [si + 2], 'h'
		jnz	midierror
		cmp	byte ptr [si + 3], 'd'
		jnz	midierror
		add	si, 4
		
		;ヘッダブロックの長さ
		lodsb
		mov	es:[byte ptr leng + 3], al
		lodsb
		mov	es:[byte ptr leng + 2], al
		lodsb
		mov	es:[byte ptr leng + 1], al
		lodsb
		mov	es:[byte ptr leng + 0], al
		
		;フォーマット
		lodsw
		xchg	al, ah
		mov	es:[format], ax
		cmp	ax, 3	;ﾌｫｰﾏｯﾄ 0,1,2 のみ対応
		jae	midierror
		
		;トラックブロック数
		lodsw
		xchg	al, ah
		cmp	ax, MAXTRK	;トラック最大数
		jbe	midi_1
		mov	ax, MAXTRK
midi_1:
		mov	es:[ntrks], ax
		
		;4分音符当たりの分解能
		lodsw
		xchg	al, ah
		mov	es:[division], ax
		cmp	ax, 0
		jle	midierror		;実時間サポートはしない
		
		
		;各トラックのスタートアドレスを得る
		mov	cx, es:[ntrks]
		mov	di, offset addrs
		mov	bx, offset counts
addrlop:
		;識別子チェック
		cmp	byte ptr [si + 0], 'M'
		jnz	midierror
		cmp	byte ptr [si + 1], 'T'
		jnz	midierror
		cmp	byte ptr [si + 2], 'r'
		jnz	midierror
		cmp	byte ptr [si + 3], 'k'
		jnz	midierror
		add	si, 4
		
		lodsw
		xchg	al, ah
		mov	dx, ax
		lodsw
		xchg	al, ah
		
		;ゲートタイムの取得（可変長7bit）
		push	dx
		push	si
		push	ax
		push	bx
		
		xor	dx, dx
		xor	ax, ax
mid_1:
		mov	bl, [si]
		inc	si
		rept	7
		sal	ax, 1
		rcl	dx, 1
		endm
		push	bx
		and	bl, 07fh
		add	al, bl
		adc	ah, 0
		adc	dx, 0
		pop	bx
		test	bl, 80h
		jnz	mid_1
		
		pop	bx
		
		mov	es:[bx], ax
		mov	es:[bx + 2], dx
		mov	byte ptr es:[bx + 5], 0
		add	bx, 6
		
		
		pop	dx
		
		;この時のsiの値を入れる
		
		mov	ax, si
		stosw
		mov	ax, ds
		stosw
		pop	si
		
		;ds:siに、増加分を加える
		add	si, dx
		pop	dx
		adc	ax, dx
		
		push	si
		shr	si, 1
		shr	si, 1
		shr	si, 1
		shr	si, 1
		add	ax, si
		mov	ds, ax
		pop	si
		and	si, 15
		
		loop	addrlop

		jmp	ok
midierror:		
		mov	ax, 1
		jmp	nook
ok:		
		mov	al, 0fah	;start
		outdata2
		push	ds
		
		mov	ax, cs
		mov	ds, ax
		mov	di, offset markt
		mov	si, offset addrs
		mov	cx, [ntrks]
		sal	cx, 1
		rep	movsw
		
		pop	ds
		mov	es:[timef8], 0
		mov	es:[stop], 0
		mov	ax, 0
		
nook:
		;直接ここから戻ってしまう
		;ちょっときたないけど
		
		pop bx		;return addr dummy
		pop bx		;ax dummy pop 
		
		pop es
		pop ds
		pop si
		pop di
		pop dx
		pop cx
		pop bx
		
		iret

midiplay	ENDP

midisend	PROC	NEAR
		;dlの値をMIDI PORTに送信
		push	es
		mov	ax, cs
		mov	es, ax
		
		cld
		
		mov	al, dl
		
		pushf
		cli
		outdata2
		popf
		
		pop	es
		ret
		ENDP
		
midistop	PROC	NEAR
		;MIDI	stop
		push	ax
		push	es
		
		cld
		
		mov	ax, cs
		mov	es, ax

		call	alloff		;all note off
		mov	es:[stop], 1
		
		mov	al, 0fch	;stop command
		outdata2
		
		pop	es
		pop	ax
		ret
		
midistop	ENDP

midicont	PROC	NEAR
		;MIDI	Continue
		push	ax
		push	es
		
		cld
		
		mov	ax, cs
		mov	es, ax
		
		mov	al, 0fbh	;continue command
		
		outdata2
		
		mov	es:[stop], 0
		pop	es
		pop	ax
		ret
		
midicont	ENDP

midicall	PROC	NEAR
		;MIDIサブルーチン設定
		;	es:bx ; call program addr
		mov	ax, cs
		mov	ds, ax
		
		pushf
		cli
		mov	[word ptr midisub], bx
		mov	[word ptr midisub + 2], es
		popf
		
		
		ret
midicall	ENDP

midiextend	PROC	NEAR
		;MIDIサブルーチン設定
		;	es:bx ; call program addr
		mov	ax, cs
		mov	ds, ax
		
		mov	[midiext], 1
		
		ret
midiextend	ENDP


miditem		PROC	NEAR
		;テンポの補正を行なう
		;	dx = 補正値
		;
		;この補正によって
		;	tempo = tempo * 補正値 / 256
		;となる
		push	ds
		mov	ax, cs
		mov	ds, ax
		
		mov	[dtempo], dx
		
		;テンポの再設定
		
		mov	ax, word ptr [miditempo + 0]
		mov	dx, word ptr [miditempo + 2]
		
		;
		;(これを1000*divisionで割る)
		
		push	bx
		mov	bx, 10000
		div	bx
		
		mov	dx, [clk]
		mul	dx
		
		mov	bx, [division]
		
		;除算エラーが出ないようにする
		;dx:ax / bx < 65535 ならばOK
		;ということは、
		;dx:ax < bx << 16  ならばOK
		;おおざっぱに言って
		;dx < bxならばOK
		mov	[midishift],0
_divlp:
		cmp	dx,bx
		jb	_divok
		shr	dx,1
		rcr	ax,1
		inc	[midishift]
		jmp	_divlp
_divok:
		div	bx
		;axに、dtempoを掛けて、256で割る
		mov	dx, [dtempo]
		mul	dx
		mov	al, ah
		mov	ah, dl
		mov	dl, dh
		
_divlp2:
		cmp	dl, 0
		jz	_divokp
		shr	dl, 1
		rcr	ax, 1
		inc	[midishift]
		jmp	_divlp2
_divokp:
		mov	[timer], ax
		
		mov	bx, ax
		pushf
		cli
		mov	al, bl
		out	71h, al
		jmp	$+2
		jmp	$+2
		jmp	$+2
		jmp	$+2
		mov	al, bh
		out	71h, al
		popf
		
		pop	bx

		pop	ds
		ret
		
miditem		ENDP


midifade	PROC	NEAR
		;MIDI FADE OUT
		;cx = フェードのgatetime
		;
		;ベロシティーの割合
		;	from dl/128 to dh/128
		push	ax
		push	bx
		push	cx
		push	dx
		push	ds
		mov	ax, cs
		mov	ds, ax
		
		mov	[gtime], cx
		mov	[dltx], cx
		
		mov	bh, 0
		mov	bl, dl
		mov	[velo], bx
		mov	ah, 0
		mov	al, dh
		
		sub	ax, bx
		jg	mid_f1
		jl	mid_f2
		;dl = dh
		mov	[sy], 0
		jmp	mid_fe
mid_f2:
		;dl > dh
		neg	ax
		mov	[sy], -1
		jmp	mid_fe
mid_f1:
		;dl < dh
		mov	[sy], 1
mid_fe:
		cmp	ax, 128
		jbe	mid_f3
		mov	ax, 128
mid_f3:
		mov	[dlty], ax
		
		neg	cx
		sar	cx, 1
		mov	[E], cx
		
		pop	ds
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret
midifade	ENDP


midi		PROC	NEAR
		;MIDIの割り込みイニシャライズルーチン
		;
		;		dl <- インターフェース選択
		;			1: MPU401
		;			2: RS-232C
		;			

		push	es
		pop	ds
		
		cld
		
		mov	ax, cs
		mov	es, ax
		mov	es:[inter], dl	;インターフェース保存
		
		mov	es:[midisub], offset mididmy
		mov	es:[midisub + 2], es
		
		;MIDI イニシャライズ
		test	es:[inter], 1
		jz	nompu
		call	initmpu
		cmp	ax, 0
		jnz	k_error	;MPU interfaceが付いていない
		
nompu:
		test	es:[inter], 2
		jz	no232
		call	init232c
no232:		
		;バッファセット
		mov	es:[bufaddr], offset buf
		mov	es:[popaddr], offset buf
		mov	es:[buflen], 0
		mov	es:[stop], 1
		mov	es:[division], 48
		
		;note = 0 (off)
		I = 0
		rept	16
		mov	byte ptr es:[note+I], 0
		endm
		
		;タイマ割り込み(int 08h)セット
		
		;タイマ割り込み禁止
		in	al, 02h
		cmp	[m_init], 1
		jz	notsetvect08_1
		mov	es:[port2], al
notsetvect08_1:
		or	al, 01h
		out	02h, al
		
		;既存のベクタを保存
		push	es
		pop	ds
		
		push	es
		mov	ax, 3508h	;timer
		int	21h
		
		cmp	[m_init], 1
		jz	notsetvect08
		
		
		mov	[int08off], bx
		mov	ax, es
		mov	[int08seg], ax
		
notsetvect08:

		pop	es
		
		;ベクタセット
		push	ds
		mov	ax, 2508h
		mov	dx, offset int08
		push	cs
		pop	ds
		int	21h
		pop	ds
		
		;8253を設定する
		;
		;
		xor	ax, ax
		mov	es, ax
		mov	bx, CLK5MHZ
		mov	al, es:[0501h]
		test	al, 80h
		jz	timer5
		mov	bx, CLK8MHZ
timer5:
		mov	[clk],bx
		
		;テンポの設定
		
		mov	word ptr [miditempo + 0], 500000 and 0ffffh
		mov	word ptr [miditempo + 2], 500000 shr 16
		
		mov	ax, word ptr [miditempo + 0]
		mov	dx, word ptr [miditempo + 2]

		mov	[velo], 128
		mov	[gtime], 0
		mov	[dtempo], 256
		mov	[midiext], 0
		;
		;(これを1000*divisionで割る)
		
		mov	bx, 10000
		div	bx
		
		mov	dx, [clk]
		mul	dx
		
		mov	bx, [division]

		;除算エラーが出ないようにする
		;dx:ax / bx < 65535 ならばOK
		;ということは、
		;dx:ax < bx << 16  ならばOK
		;おおざっぱに言って
		;dx < bxならばOK
		mov	[midishift],0
_divlp3:
		cmp	dx,bx
		jb	_divok2
		shr	dx,1
		rcr	ax,1
		inc	[midishift]
		jmp	_divlp3
_divok2:
		div	bx
		mov	[timer], ax
		
		mov	bx, ax
		mov	al, 36h		;mode 3
		out	77h, al
		jmp	$+2
		jmp	$+2
		jmp	$+2
		jmp	$+2
		mov	al, bl
		out	71h, al
		jmp	$+2
		jmp	$+2
		jmp	$+2
		jmp	$+2
		mov	al, bh
		out	71h, al
		
		;タイマ割り込み許可
		in	al, 02h
		and	al, 0feh
		out	02h, al
		mov	[m_init], 1
		xor	ax, ax
k_error:
		
		;直接ここから戻ってしまう
		;ちょっときたないけど
		
		pop bx		;return addr dummy
		pop bx		;ax dummy pop 
		
		pop es
		pop ds
		pop si
		pop di
		pop dx
		pop cx
		pop bx
		
		iret
		
midi		ENDP
	

init:	;割り込み設定
	push cs
	pop ds
	mov [useg],es
	mov	[seseg], es
	;既に割り込みがセットされているかチェック
	push es
	mov ah,35h
	mov al,[musicvect]	;サウンド割り込みロード
	add al,8
	int 21h
	
	cmp bx,offset mdrv
	jnz init2
	mov ax,es
	mov cx,cs
	cmp ax,cx
	jnz init2
	;既に割り込みベクタがかかっていると判断される
	pop es
	
	;fm initialize
	mov dx,[ioport1]
oon1:	in al,dx
	and al,80h
	jnz oon1
	jmp $+2
	jmp $+2
	mov al,2dh
	out dx,al
	
	mov bh,7
	mov bl,00111111B
	call opnwrt
	
	mov cx,656	;タイマーＡ周期デフォルト
	mov [timera],cx
	mov [ptimera],cx
	
	mov bh,25h
	mov bl,cl
	and bl,3
	call opnwrt
	
	mov bh,24h
	shr cx,1
	shr cx,1
	mov bl,cl
	call opnwrt
	
	mov bx,2680h	;タイマーＢ周期セット
	mov al,bl
	mov [tempo],al
	call opnwrt
	
	
	;割り込みの許可
	
	test [musicvect],8
	jnz tvect8_15
	;vect 0-7
	in al,2
	mov ah,[maskf]
	not ah
	and al,ah
	jmp $+2
	jmp $+2
	out 2,al
	jmp tvectnoed
	
tvect8_15:
	;vect 8-15
	in al,0ah
	mov ah,[maskf]
	not ah
	and al,ah
	jmp $+2
	jmp $+2
	out 0ah,al
tvectnoed:
	
	mov bx,273ch
	call opnwrt
	
	mov bx,273ah	;タイマ始動　　立ち上がりでスタート
	call opnwrt
	
	ret

init2:	
	mov [vecsoundoff],bx
	mov [vecsoundseg],es
	mov	[vectsetflag], 1
	mov ah,25h
	mov al,[musicvect]	;サウンド割り込みセット
	add al,8
	mov dx,offset mdrv
	int 21h
	
	;
	pop es
	
settemp:
	
	;割り込みセット
	
	;fm initialize
	mov dx,[ioport1]
on1:	in al,dx
	and al,80h
	jnz on1
	jmp $+2
	jmp $+2
	;mov al,2dh
	;out dx,al
	
	mov bh,7
	mov bl,00111111B
	call opnwrt
	
	mov cx,656	;タイマーＡ周期デフォルト
	mov [timera],cx
	mov [ptimera],cx
	
	mov bh,25h
	mov bl,cl
	and bl,3
	call opnwrt
	
	mov bh,24h
	shr cx,1
	shr cx,1
	mov bl,cl
	call opnwrt
	
	mov bx,2680h	;周期セット
	mov al,bl
	mov [tempo],al
	call opnwrt
	
	test [musicvect],8
	jnz yvect8_15
	;vect 0-7
	in al,2
	mov [invmask],al
	mov ah,[maskf]
	not ah
	and al,ah
	jmp $+2
	jmp $+2
	out 2,al
	jmp yvectnoed
	
yvect8_15:
	;vect 8-15
	in al,0ah
	mov [invmask],al
	mov ah,[maskf]
	not ah
	and al,ah
	jmp $+2
	jmp $+2
	out 0ah,al
yvectnoed:
	
	mov bx,273ch	;フラグを下げる
	call opnwrt
	
	mov bx,273ah	;タイマＢ始動
	call opnwrt
	
	ret
	
	EVEN
	
mdrv:   cli
	push ax
	push bx
	push cx
	push dx
	push di
	push si
	push ds
	push es
	push bp
	
	push cs
	pop ds

	cmp	[pcmplas], 0
	jz	notpcmplas
	
	mov	dx, 0a468h
	in	al, dx
	test	al, 00010000B
	jz	notpcmplas
	
	;PCM FIFO割り込み
	
	
	;データ送信
	
	mov	es, [pcmoffset + 2]
	mov	bx, [pcmoffset]
	
	mov	dx, [pcmsize + 2]
	mov	cx, [pcmsize]
	
	;
	mov	ax, TIME_PCMSIZE / 2
_pcms_lp:
	push	ax
	;FIFOがフルかどうか?
	push	dx
	mov	dx, 0a466h
	in	al, dx
	pop	dx
	
	test	al, 80h
	jnz	_pcms_2	;FIFOフル
	
	cmp	[pcmover], 0
	jz	_pcms_5
	
	push	dx
	mov	dx, 0a46ch
	mov	al, 0
	out	dx, al
	pop	dx
	
	inc	[pcm0send]
	cmp	[pcm0send], 32767
	jb	notpcmstop
	
	;PCM割り込みストップ&ループ飛びだし
	push	dx
	mov	dx, 0a468h
	
	mov	al, 0
	out	dx, al
	
	mov	[pcmflag], 1
	mov	[pcm_j], 0ffffh
	
	pop	dx
	
	pop	ax
	
	jmp	notpcmplas
	
notpcmstop:
	jmp	_pcms_4
	
_pcms_5:
	mov	al, es:[bx]
	push	dx
	mov	dx, 0a46ch
	out	dx, al
	pop	dx
	
	inc	bx
	jnz	_pcms_3
	mov	ax, es
	add	ax, 1000h
	mov	es, ax
	
_pcms_3:
	sub	cx, 1
	sbb	dx, 0
	
	mov	ax, cx
	or	ax, dx
	jnz	_pcms_4
	
	mov	[pcmover], 1
	mov	[pcm0send], 0
	
_pcms_4:
	pop	ax
	dec	ax
	jnz	_pcms_lp
	;jmp	_pcms_lp
	
	jmp	_pcm_k
_pcms_2:
	pop	ax
	
_pcm_k:	
	mov	[pcmoffset + 2], es
	mov	[pcmoffset], bx
	
	mov	[pcmsize + 2], dx
	mov	[pcmsize], cx

	;FIFO割り込みクリア
	mov	dx, 0a468h
	in	al, dx
	
	test	al, 00010000B
	jz	notpcmflag
	
	and	al, 11101111B
	out	dx, al
	
	or	al, 00010000B
	out	dx, al
	
	jmp	notpcm_1
	
notpcmflag:
	
	
notpcmplas:
	
	
	;PCMフラグが立っていたら、演奏タイマの割り込み時にも少しFIFOに送る
	
	cmp	[pcmflag], 0
	jnz	notpcm_1
	
	;データ送信
	
	mov	es, [pcmoffset + 2]
	mov	bx, [pcmoffset]
	
	mov	dx, [pcmsize + 2]
	mov	cx, [pcmsize]
	
	;
	mov	ax, 128	;@@@
g_pcms_lp:
	push	ax
	;FIFOがフルかどうか?
	push	dx
	mov	dx, 0a466h
	in	al, dx
	pop	dx
	
	test	al, 80h
	jnz	g_pcms_2	;FIFOフル
	
	cmp	[pcmover], 0
	jz	g_pcms_5
	
	push	dx
	mov	dx, 0a46ch
	mov	al, 0
	out	dx, al
	cmp	[pcm_j], 0ffffh
	jz	pcm___1
	inc	[pcm_j]
pcm___1:
	
	pop	dx
	
	inc	[pcm0send]
	cmp	[pcm0send], 32767
	jb	g_notpcmstop
	
	;PCM割り込みストップ&ループ飛びだし
	push	dx
	mov	dx, 0a468h
	
	mov	al, 0
	out	dx, al
	
	mov	[pcmflag], 1
	mov	[pcm_j], 0ffffh
	
	pop	dx
	
	pop	ax
	
	jmp	g_notpcmplas
	
g_notpcmstop:
	jmp	g_pcms_4
	
g_pcms_5:
	mov	al, es:[bx]
	push	dx
	mov	dx, 0a46ch
	out	dx, al
	cmp	[pcm_j], 0ffffh
	jz	pcm___2
	inc	[pcm_j]
pcm___2:
	pop	dx
	
	inc	bx
	jnz	g_pcms_3
	mov	ax, es
	add	ax, 1000h
	mov	es, ax
	
g_pcms_3:
	sub	cx, 1
	sbb	dx, 0
	
	mov	ax, cx
	or	ax, dx
	jnz	g_pcms_4
	
	mov	[pcmover], 1
	mov	[pcm0send], 0
	
g_pcms_4:
	pop	ax
	test	[pcm_j], 8000h
	jz	g_pcms_notst
	
	cmp	[pcm_j], 0ffffh
	jz	g_pcms_notst
	
	;FIFOスタート
	push	ax
	push	dx
	
	IF 0
	
	;FIFO割り込みフラグリセット
	mov	dx, 0a468h
	in	al, dx
	
	;トリガOFF
	and	al, 11101111B
	out	dx, al
	
	;トリガON
	or	al, 00010000B
	out	dx, al
	
	;FIFOスタート
	or	al, 10000000B
	out	dx, al
	
	;割り込み許可
	or	al, 00100000B
	out	dx, al

	ENDIF
	
	mov	[pcm_j], 0ffffh
	
	pop	dx
	pop	ax
	
	jmp	g_pcm_k
	
g_pcms_notst:
	dec	ax
	jnz	g_pcms_lp
	
	jmp	g_pcm_k
g_pcms_2:
	pop	ax
	
g_pcm_k:	
	mov	[pcmoffset + 2], es
	mov	[pcmoffset], bx
	
	mov	[pcmsize + 2], dx
	mov	[pcmsize], cx
	
g_notpcmplas:

	
notpcm_1:
	mov dx,[ioport1]
	in al,dx
	mov [tflag],al
	
	;timer reset
	
	;mov cx,[timera]		;タイマーＡ周期
	;mov bh,24h
	;push cx
	;shr cx,1
	;shr cx,1
	;mov bl,cl
	;call opnwrt
	;pop cx
	;mov bh,25h
	;and cl,3
	;mov bl,cl
	;call opnwrt
	
	cmp	[temposetflag], 0
	jz	nottemposet
	
	mov al,[tempo]
	mov bl,al
	mov bh,26h
	call opnwrt
	mov	[temposetflag], 0
	
nottemposet:

	mov bh,27h
	mov bl,00111111B
	cmp [exflag],1
	jz ata
	;timer-a ﾅｼ
	and bl,11111010B
ata:	call opnwrt
	
	
	
	test [musicvect],8
	jnz vect8_15
	;vect 0-7
	in al,2
	or al,[maskf]
	jmp $+2
	jmp $+2
	jmp $+2
	jmp $+2
	jmp $+2
	out 2,al
	;eoi
	mov al,01100000B
	or al,[musicvect]
	mov al,20h
	jmp $+2
	jmp $+2
	jmp $+2
	jmp $+2
	jmp $+2
	out 0,al
	jmp vectnoed
	
vect8_15:
	;vect 8-15
	in al,0ah
	or al,[maskf]
	jmp $+2
	jmp $+2
	jmp $+2
	jmp $+2
	jmp $+2
	out 0ah,al
	mov al,01100000B
	or al,[musicvect]
	and al,11100111B
	mov al,20h
	jmp $+2
	jmp $+2
	jmp $+2
	jmp $+2
	jmp $+2
	out 8,al
	
	
vectnoed:
	sti
	
	mov ah,2
	int 18h		;シフトキー状態の入手
	
	mov [shift],al
	mov ah,[ston]
	and al,ah
	jz noshift
	;キーロックによる抑制なら、キーオフにする
	;まず、ＦＭ
	mov bx,2800h
	call opnwrt
	inc bx
	call opnwrt
	inc bx
	call opnwrt
	;ＰＳＧ
	mov bl,[tn]
	or bl,00111111B
	mov bh,7
	call opnwrt
noshift:
	
	mov es,[useg]
	
	
	
	;タイマのどちらの割り込みかを調べる
	
	mov al,[tflag]
	test al,1
	jz tmb		;not timer a
	
	
tma:
	;Timer a
	;サウンドエキスパンダ　
	;	＆　Ｐｓｇ　ｖｏｉｃｅ　ｓｐｅａｋｉｎｇ　ｓｙｓｔｅｍ　本体
	;
	mov [fmfrug],1
	
	mov al,[pflag]
	or al,al
	jnz sno
	
	mov al,[exflag]		;エキスパンダ使用をするか
	or al,al
	jz sno

	mov al,1
	mov [nch],al
	mov di,offset play5
	mov al,[di+2]
	or al,al	;割り込み演奏中か
	jz sn1
	
	cmp [voicef],0
	jz snn1		;音声出力中か
	mov bl,[tn]
	mov bh,7
	call opnwrt
	push es
	mov es,[v1seg]
	mov si,[v1off]
	mov al,es:[si]
	and [v1sel],1
	jnz vc11
	shr al,1
	shr al,1
	shr al,1
	shr al,1
	jmp vc12
vc11:	and al,0fh
	inc [v1off]
	dec [v1byte]
	jnz vc12
	;割り込み終了
	mov byte ptr [di+2],0
	mov [voicef],0
	;割り込みＡをデフォルトに戻す
	mov [timerbplay],0
	mov ax,[ptimera]
	mov [timera],ax
	
	mov cx,[timera]		;タイマーＡ周期
	
	cli
	mov bh,25h
	mov bl,cl
	and bl,3
	call opnwrt
	
	mov bh,24h
	shr cx,1
	shr cx,1
	mov bl,cl
	call opnwrt
	
	sti
	mov bx,offset play5
	call settn
	cmp [voicedouble],0
	jz vc12
	mov bx,offset play6
	call settn
	
vc12:	inc [v1sel]
	mov bh,9
	mov bl,al
	mov dx,[ioport1]
	mov al,bh
	out dx,al
	mov dx,[ioport2]
	mov al,bl
	out dx,al
	cmp [voicedouble],0
	jz vcg1
	inc bh
	mov dx,[ioport1]
	mov al,bh
	out dx,al
	mov dx,[ioport2]
	mov al,bl
	out dx,al
vcg1:
	pop es
	jmp sn1
	
snn1:	
	push	es
	mov	es, [seseg]
	mov di,offset int1
	call psgply
	pop	es
	;
sn1:	mov al,2
	mov [nch],al
	mov di,offset play6
	mov al,[di+2]
	or al,al
	jz sn2
	cmp [voicef2],0
	jz snn2
	mov bl,[tn]
	mov bh,7
	call opnwrt
	
	push es
	mov es,[v2seg]
	mov si,[v2off]
	mov al,es:[si]
	and [v2sel],1
	jnz vc21
	shr al,1
	shr al,1
	shr al,1
	shr al,1
	jmp vc22
vc21:	and al,0fh
	inc [v2off]
	dec [v2byte]
	jnz vc22
	;割り込み終了
	mov byte ptr [di+2],0
	mov [voicef2],0
	mov [timerbplay],0
	
	mov ax,[ptimera]
	mov [timera],ax
	
	mov cx,[timera]		;タイマーＡ周期
	
	cli
	mov bh,25h
	mov bl,cl
	and bl,3
	call opnwrt
	
	mov bh,24h
	shr cx,1
	shr cx,1
	mov bl,cl
	call opnwrt
	sti
	
	
	mov bx,offset play6
	call settn
	cmp [voicedouble],0
	jz vc22
	mov bx,offset play5
	call settn
	
vc22:	inc [v2sel]
	mov bh,10
	mov bl,al
	mov dx,[ioport1]
	mov al,bh
	out dx,al
	mov dx,[ioport2]
	mov al,bl
	out dx,al
	cmp [voicedouble],0
	jz vcg2
	dec bh
	mov dx,[ioport1]
	mov al,bh
	out dx,al
	mov dx,[ioport2]
	mov al,bl
	out dx,al
vcg2:	
	pop es
	jmp sn2
	
snn2:	
	push	es
	mov	es, [seseg]
	mov di,offset int2
	call psgply
	pop	es
sn2:
sno:
	;mov cx,[timera]		;タイマーＡ周期
	;mov bh,24h
	;push cx
	;shr cx,1
	;shr cx,1
	;mov bl,cl
	;call opnwrt
	;pop cx
	;mov bh,25h
	;and cl,3
	;mov bl,cl
	;call opnwrt
	
	mov al,[tflag]
	and al,2
	jnz tmb		;timer b
	;Ｔｉｍｅｒ－Ａ割り込み処理中にＴｉｍｅｒ－Ｂ割り込みがかかっているかもしれない
	mov dx,[ioport1]
	in al,dx
	and al,2
	jnz tmb
	
	
	jmp nona
	
	
tmb:
	;Timer B 
	
	;
	
	;
tttm:	mov al,[pflag]	;演奏許可フラグ
	or al,al
	jnz no
	
	cmp [timerbplay],0
	jnz no
	
	xor al,al	;ＦＭ音源
	mov [fmfrug],al
	mov [ext86], al
	mov [nch],al
	mov di,offset play1
	
	test [allstopflag], 1000000B
	jnz	n_plfm1
	call fmplay

n_plfm1:
	mov [nch],1
	mov di,offset play2
	
	test [allstopflag], 10000000B
	jnz	n_plfm2
	call fmplay
	
n_plfm2:
	mov [nch],2
	mov di,offset play3
	
	test [allstopflag], 100000000B
	jnz	n_plfm3
	call fmplay

n_plfm3:
	;
	
	mov [ext86], 1
	mov [nch],0
	mov di,offset play7
	test [allstopflag], 1000000000B
	jnz	n_plfm4
	
	call fmplay
n_plfm4:

	mov [nch],1
	mov di,offset play8

	test [allstopflag], 10000000000B
	jnz	n_plfm5
	call fmplay

n_plfm5:
	mov [nch],2
	mov di,offset play9
	
	test [allstopflag], 100000000000B
	jnz	n_plfm6
	call fmplay

n_plfm6:
	
	;RTH
	
	mov	[fmfrug], 2
	mov	[nch], 0
	mov	di, offset play10
	test [allstopflag], 1B
	jnz	n_plrth1
	
	call	rthplay
n_plrth1:

	mov	[nch], 1
	mov	di, offset play11

	test [allstopflag], 10B
	jnz	n_plrth2

	call	rthplay

n_plrth2:	
	mov	[nch], 2
	mov	di, offset play12
	
	test [allstopflag], 100B
	jnz	n_plrth3
	
	call	rthplay

n_plrth3:

	mov	[nch], 3
	mov	di, offset play13
	
	test [allstopflag], 1000B
	jnz	n_plrth4
	
	call	rthplay

n_plrth4:
	mov	[nch], 4
	mov	di, offset play14
	
	test [allstopflag], 10000B
	jnz	n_plrth5
	
	call	rthplay
	
n_plrth5:
	mov	[nch], 5
	mov	di, offset play15

	test [allstopflag], 100000B
	jnz	n_plrth6
	
	call	rthplay
	
n_plrth6:
	mov [ext86], 0
	
	mov [fmfrug],1
	mov [nch],0
	mov di,offset play4
	
	test [allstopflag], 1000000000000B
	jnz	n_plpsg1
	
	call psgply
n_plpsg1:
	mov [nch],1
	mov di,offset play5
	
	test [allstopflag], 10000000000000B
	jnz	n_plpsg2
	
	call psgply
	
n_plpsg2:
	cmp [tbf],0
	jnz n1
	mov di,offset play5
	mov al,[di+2]
	or al,al
	jz n1
	push	es
	mov	es, [seseg]
	mov di,offset int1	;psg 2ch 割り込み
	call psgply
	pop	es
n1:	mov [nch],2
	mov di,offset play6
	
	test [allstopflag], 100000000000000B
	jnz	n_plpsg3
	
	call psgply
	
n_plpsg3:
	cmp [tbf],0
	jnz no
	mov di,offset play6
	mov al,[di+2]
	or al,al
	jz no
	push	es
	mov	es, [seseg]
	mov di,offset int2	;psg 3ch 割り込み
	call psgply
	pop	es

no:
	
	;mov al,[tempo]
	;mov bl,al
	;mov bh,26h
	;call opnwrt
	
nona:
	;mov cx,[timera]		;タイマーＡ周期
	;mov bh,24h
	;push cx
	;shr cx,1
	;shr cx,1
	;mov bl,cl
	;call opnwrt
	;pop cx
	;mov bh,25h
	;and cl,3
	;mov bl,cl
	;call opnwrt
	
	;mov al,[tempo]
	;mov bl,al
	;mov bh,26h
	;call opnwrt
	
	cli
	

	mov bh,27h
	mov bl,00111111B
	
	cmp [exflag],1
	jz aata
	;timer-a ﾅｼ
	and bl,11111010B
	
aata:	call opnwrt


	cmp	[pcmplas], 0
	jz	notpcmres
	
pcm___lop:
	mov	dx, 0a468h
	in	al, dx
	test	al, 00010000B
	jz	notpcmres
	
	;FIFO割り込みクリア
	mov	dx, 0a468h
	in	al, dx
	
	and	al, 11101111B
	out	dx, al
	
	or	al, 00010000B
	out	dx, al
	
	;PCM FIFO割り込みリセット
	
	mov	es, [pcmoffset + 2]
	mov	bx, [pcmoffset]
	
	mov	dx, [pcmsize + 2]
	mov	cx, [pcmsize]
	
	;
	mov	ax, TIME_PCMSIZE * 2
t_pcms_lp:
	push	ax
	;FIFOがフルかどうか?
	push	dx
	mov	dx, 0a466h
	in	al, dx
	pop	dx
	
	test	al, 80h
	jnz	t_pcms_2	;FIFOフル
	
	cmp	[pcmover], 0
	jz	t_pcms_5
	
	push	dx
	mov	dx, 0a46ch
	mov	al, 0
	out	dx, al
	pop	dx
	
	inc	[pcm0send]
	cmp	[pcm0send], 32767
	jb	t_notpcmstop
	
	;PCM割り込みストップ&ループ飛びだし
	push	dx
	mov	dx, 0a468h
	
	mov	al, 0
	out	dx, al
	
	mov	[pcmflag], 1
	mov	[pcm_j], 0ffffh
	
	pop	dx
	
	pop	ax
	
	jmp	t_notpcmplas
	
t_notpcmstop:
	jmp	t_pcms_4
	
t_pcms_5:
	mov	al, es:[bx]
	push	dx
	mov	dx, 0a46ch
	out	dx, al
	pop	dx
	
	inc	bx
	jnz	t_pcms_3
	mov	ax, es
	add	ax, 1000h
	mov	es, ax
	
t_pcms_3:
	sub	cx, 1
	sbb	dx, 0
	
	mov	ax, cx
	or	ax, dx
	jnz	t_pcms_4
	
	mov	[pcmover], 1
	mov	[pcm0send], 0
	
t_pcms_4:
	pop	ax
	dec	ax
	jnz	t_pcms_lp
	;jmp	_pcms_lp
	
	jmp	t_pcm_k
t_pcms_2:
	pop	ax
	
t_pcm_k:	
	mov	[pcmoffset + 2], es
	mov	[pcmoffset], bx
	
	mov	[pcmsize + 2], dx
	mov	[pcmsize], cx

	
	jmp	pcm___lop
	
t_notpcmplas:
notpcmres:
	
	test [musicvect],8
	jnz qvect8_15
	;vect 0-7
	in al,2
	mov ah,[maskf]
	not ah
	and al,ah
	jmp $+2
	jmp $+2
	jmp $+2
	jmp $+2
	out 2,al
	jmp qvectnoed
	
qvect8_15:
	;vect 8-15
	in al,0ah
	mov ah,[maskf]
	not ah
	and al,ah
	jmp $+2
	jmp $+2
	jmp $+2
	jmp $+2
	out 0ah,al
	
qvectnoed:

	cmp	[pcmplas], 0
	jz	r_notpcmres
	
	mov	dx, 0a468h
	in	al, dx
	
	test	al, 00010000B
	jz	r_notpcmres
	
	and	al, 11101111B
	out	dx, al
	
	or	al, 00010000B
	out	dx, al
	

r_notpcmres:
	pop bp
	pop es
	pop ds
	pop si
	pop di
	pop dx
	pop cx
	pop bx
	pop ax
	
	iret
	
;
;

	;RTH play
rthplay:
	mov	si, di
	
	mov	al, [si]	;leng counter
	or	al, al
	jz	rth_change		;次の命令を読む
	
	;
	dec	al
	mov	[si], al
	inc	si
	
	;ボリューム値の取得
	;上位ビットよりL(1) R(1) B(1) VOL(5)の順で入っている
	;	B=0でKEY ON B=1でKEY OFF
	
	mov	al, [si]
	inc	si
	
	;タイかどうか?
	cmp	byte ptr [di + 3], 0
	jnz	not_rthkeyonoff	;タイならばキーON/OFFしない
	
	;すでにKEYON/OFFしたかどうか
	cmp	byte ptr [di + 18], 0
	jz	not_rthkeyonoff
	
	;keyon かつボリュームが0ならば、セットしない
	test	al, 111111B
	jz	not_rthkeyonoff
	
	;ボリューム設定
	push	ax
	mov	bh, [nch]
	add	bh, 18h
	mov	bl, al
	and	bl, 11011111B
	call	opnwrt
	
	;キーON/OFF
	
	mov al,[shift]
	
	mov ah,[ston]
	and al,ah
	pop	ax
	jnz not_rthkeyonoff
	
	mov	bh, 10h
	mov	bl, al
	and	bl, 00100000B
	sal	bl, 1
	sal	bl, 1
	
	push	cx
	mov	cl, [nch]
	mov	al, 1
	sal	al, cl
	or	bl, al
	pop	cx
	call	opnwrt
	
	;キーONした印
	mov	byte ptr [di + 18], 0
	
not_rthkeyonoff:

	;
	;fade out 処理をする(予定)(全リズム音源共通ボリュームによる)
	;
	
	mov	al, [di + 26]
	or	al, al
	jz	not_rthfadeout
	
	dec	byte ptr [di + 12]
	jnz	rth__1
	
	mov	al, [di + 26]
	mov	[di + 12], al
	
	mov	al, [di + 14]
	cmp	al, 0
	jnz	rth__2
	
	mov	byte ptr [di + 26], 0
	jmp	not_rthfadeout
	
rth__2:
	dec	al
	mov	[di + 14], al
	
	mov	bh, 11h
	mov	bl, al
	call	opnwrt
	
rth__1:
	

not_rthfadeout:

	
	ret


	;fm play
	
fmplay: mov si,di
	
	mov al,[si]	;leng counter
	or al,al
	jz change
	
play:	dec al
	mov [si],al
	inc si
	mov al,[si]	;休符か
	or al,al
	jnz lab1
	
	cmp byte ptr [di+18],0
	jz nokeyoff
	mov byte ptr [di+18],0
	mov bl,[nch]
	mov bh,28h
	call opnwrt
nokeyoff:
	ret
	
lab1:	inc si
	inc si
	dec byte ptr [si-1]	;key on leng --
	jnz fopr
	
	mov al,[si]		;鯛じゃなくて　タイか？
	or al,al
	jnz fopr		;タイ
	
keyoff: mov bh,28h		;key on/off addr
	mov al,[nch]
	mov bl,al
	call opnwrt
	mov byte ptr [di+18],0		;key on flag clear
	;
fopr:	inc si
	mov al,[si]
	mov cl,al
	inc si
	push si
	mov al,[si]
	mov ch,al
	inc si
	mov al,[si]		;ポルタメント
	or al,al
	jz vibrat
	mov bl,al
	mov bh,0
	cmp al,80h
	jb l2
	dec bh
l2:	add cx,bx
;
vibrat: 
	cmp	byte ptr [di + 31], 0
	jnz	fout
	
	inc si	;flat time
	mov al,[si]
	or al,al
	jz v1	;then fout
	dec al
	mov [si],al
	jmp fout
	
v1:	inc si
	mov al,[si]
	or al,al	;speed=0 ?
	jz v2
	dec al
	mov [si],al
	jmp fout
	;
v2:	mov al,[di+21]	;spped set again
	mov [si],al
	
	inc si
	mov al,[si]	;上昇＝０　下降＝０
	or al,al
	jnz kakou
	
jousou: mov bh,0
	inc si
	mov al,[si]
	mov bl,al
	add cx,bx
	inc si
	dec byte ptr [si]	;加える回数を満たしたか
	jnz fout
	mov al,[di+22]		;timer set again
	add al,al
	mov [si],al
	mov byte ptr [di+9],1
	jmp fout
	
kakou:	inc si
	mov bh,0
	mov al,[si]
	mov bl,al
	neg	bx
	add cx,bx
	inc si
	dec byte ptr [si]
	jnz fout
	mov al,[di+22]
	add al,al
	mov [si],al
	mov byte ptr [di+9],0
	
fout:	pop si
	mov al,ch
	mov [si],al
	
f1:	mov al,[nch]
	add al,0a4h	;fnumber 1
	mov bh,al
	mov bl,ch
	call opnwrt
	dec bh
	dec bh
	dec bh
	dec bh
	dec si		;fnumber 2
	mov al,cl
	mov [si],al
	mov bl,al
	call opnwrt
	
;
fadeo:	mov al,[di+26]	;fade out ?
	or al,al
	jz keyon
	dec byte ptr [di+12]	;fade out speed --
	jnz keyon
	mov al,[di+26]	;fix out speed
	mov [di+12],al
	mov bl,[di+13]	;algorythm
	mov bh,0
	mov cx,offset algot
	add bx,cx
	mov bl,[bx]
	
	push si
	test bl,1
	jz lab2
	call op1v
lab2:	test bl,2
	jz lab3
	call op2v
lab3:	test bl,4
	jz lab4
	call op3v
lab4:	test bl,8
	jz lab5
	call op4v
lab5:	
	pop si
keyon:	
	
	;トレモロ
	cmp	byte ptr [di + 31], 0
	jz	v_keyon
	
	mov	al, [di + 7]	;フラットタイム
	or	al, al
	jz	v_v1
	dec	al
	mov	[di + 7], al
	jmp	v_fout
	
v_v1:
	mov	al, [di + 8]	;speed
	or	al, al
	jz	v_v2
	dec	al
	mov	[di + 8], al
	jmp	v_fout
	
v_v2:
	mov	al, [di + 21]
	mov	[di + 8], al
	
	mov	al, [di + 9]
	or	al, al
	jnz	v_kakou
	
v_jousou:
	mov	al, [di + 10]
	add	[di + 30], al
	
	dec	byte ptr [di + 11]
	jnz	v_fout
	mov	al, [di + 22]
	add	al, al
	mov	[di + 11], al
	mov	byte ptr [di + 9], 1
	
	jmp	v_fout
	
v_kakou:
	mov	al, [di + 10]
	sub	[di + 30], al
	
	dec	byte ptr [di + 11]
	jnz	v_fout
	mov	al, [di + 22]
	add	al, al
	mov	[di + 11], al
	mov	byte ptr [di + 9], 0
	
	jmp	v_fout
	
v_fout:
	
	mov	cl, [di + 31]
	
	test	cl, 1
	jz	n_op1
	
	;OP1
	mov	al, [di + 14]
	add	al, [di + 30]
	
	mov	bh, 40h
	add	bh, [nch]
	mov	bl, al
	
	call	opnwrt
	
n_op1:
	test	cl, 2
	jz	n_op2
	
	;OP2
	mov	al, [di + 15]
	add	al, [di + 30]
	
	mov	bh, 48h
	add	bh, [nch]
	mov	bl, al
	
	call	opnwrt
	
n_op2:
	test	cl, 4
	jz	n_op3
	
	;OP3
	mov	al, [di + 16]
	add	al, [di + 30]
	
	mov	bh, 44h
	add	bh, [nch]
	mov	bl, al
	
	call	opnwrt
	
n_op3:
	test	cl, 8
	jz	n_op4
	
	;OP4
	mov	al, [di + 16]
	add	al, [di + 30]
	
	mov	bh, 4ch
	add	bh, [nch]
	mov	bl, al
	
	call	opnwrt

n_op4:


v_keyon:
	mov al,[di+18]
	or al,al
	jnz lab6
	ret
lab6:	
	mov al,[shift]
	
	mov ah,[ston]
	and al,ah
	jnz labb7
	
	mov bh,28h
	mov al,[nch]
	add al,0f0h
	mov bl,al
	call opnwrt
labb7:	
	mov byte ptr [di+18],0
	
return: ret

op1v:	push bx
	mov al,[di+14]
	inc al
	mov [di+14],al
	cmp al,127
	jb o1
;	mov byte ptr [di+26],0
o1:	mov bl,al
	mov bh,40h
	mov al,[nch]
	add al,bh
	mov bh,al
	call opnwrt
	pop bx
	ret
	
op2v:	push bx
	mov al,[di+15]
	inc al
	mov [di+15],al
	cmp al,127
	jb o2
;	mov byte ptr [di+26],0
o2:	mov bl,al
	mov bh,48h
	mov al,[nch]
	add al,bh
	mov bh,al
	call opnwrt
	pop bx
	ret

op3v:	push bx
	mov al,[di+16]
	inc al
	mov [di+16],al
	cmp al,127
	jb o3
;	mov byte ptr [di+26],0
o3:	mov bl,al
	mov bh,44h
	mov al,[nch]
	add al,bh
	mov bh,al
	call opnwrt
	pop bx
	ret

op4v:	push bx
	mov al,[di+17]
	inc al
	mov [di+17],al
	cmp al,127
	jb o4
;	mov byte ptr [di+26],0
o4:	mov bl,al
	mov bh,4ch
	mov al,[nch]
	add al,bh
	mov bh,al
	call opnwrt
	pop bx
	ret

	;change next(RTH)
	
rth_change:

	mov	si, word ptr [di + 19]

rth_sub:
	mov	al, byte ptr es:[si]
	mov	word ptr [di + 19], si
	inc	si
	test	al, 80h
	jnz	rth_func	;コマンド
	
	;音長、音程(ボリューム値)
	
	;音長
	mov	[di], al
	mov	[di + 2], al
	;ボリューム値
	mov	al, byte ptr es:[si]
	inc	si
	mov	[di + 1], al
	
	;ワークイニシャライズ
	mov 	byte ptr [di+18], 1	;キーON/OFFフラグ
	mov 	al, [di+3]	;タイ
	or 	al, al
	jz 	rth_jpa
	cmp 	al, 2
	jnz 	rth_jp3
	dec 	byte ptr [di+3]
rth_jpa:	
	mov 	al, [di+25]
	mov 	[di+7], al
	mov 	al, [di+21]
	mov 	[di+8], al
	mov 	byte ptr [di+9], 0
	mov 	al, [di+22]
	mov 	[di+11], al
	
rth_jp3:
	mov 	word ptr [di+19], si
	jmp 	rthplay
	
	;リズムファンクション
rth_func:	
	and 	al, 07fh
	add 	al, al
	mov 	bx, offset rth_jptab
	xor 	ah, ah
	add 	bx, ax
	jmp 	[bx]


;
;change next (fm)
;

change:	mov si,word ptr [di+19]
	
ffsub:	mov al,byte ptr es:[si]
	mov word ptr [di + 19], si
	inc si
	test al,080h
	jnz func
	
;
fleng:	mov [di],al
	mov bl,al
	mov bh,[di+23]
	or  bh, bh
	jz	jp1
jp2:	dec bh
	jz jp1
	shr al,1
	jmp jp2
jp1:	cmp al,bl
	jnz jp4
	dec al
	jnz jp4
	mov al,1
jp4:	mov [di+2],al
;
	mov al,es:[si]
	mov [di+1],al
	or al,al

	jz rest
;
	and al,0fh
	mov bl,al
	mov bh,0
	mov cx,offset ftable-2
	add bx,bx
	add bx,cx
	mov bx,[bx]
	
	mov al,es:[si]
	and al,0f0h
	sar al,1
	or al,bh
	mov bh,al
	
	mov al,[di+24]
	mov cl,al
	mov ch,0
	cmp al,07fh
	jb jp5
	dec bh
jp5:	add cx,bx
;
	;タイオン　かつ　ポルタメント中なら、周波数はそのまま
;	cmp byte ptr [di+3],0
;	jz nonpolt
;	cmp byte ptr [di+6],0
;	jnz polt
;nonpolt:
	mov word ptr [di+4],cx

polt:	inc si

kinit:	mov byte ptr [di+18],1
	mov byte ptr [di + 30], 0
	mov al,[di+3]
	or al,al
	jz jpa
	cmp al,2
	jnz jp3
	dec byte ptr [di+3]
jpa:	mov al,[di+25]
	mov [di+7],al
	mov al,[di+21]
	mov [di+8],al
	mov byte ptr [di+9],0
	mov al,[di+22]
	mov [di+11],al
	
jp3:	mov word ptr [di+19],si
	jmp fmplay
	
rest:	mov byte ptr [di+1],0
	mov al,[nch]
	mov bx,2800h
	add al,bl
	mov bl,al
	call opnwrt
	inc si
	jmp jp3
	;
func:	and al,07fh
	add al,al
	mov bx,offset jptab
	xor ah,ah
	add bx,ax
	jmp [bx]
	
error: 	mov al,1
	mov [pflag],al
	ret
	
	;
	;
	;ｺﾏﾝﾄﾞ ｴﾝﾄﾘｰ
	
	;
	
tieoff:	mov byte ptr [di+3],0
	push bx
	mov bh,28h
	mov al,[nch]
	mov bl,al
	call opnwrt
	pop bx
	cmp	[fmfrug], 0
	jz ffsub
	jmp	rth_sub
tieon:	mov byte ptr [di+3],2
	cmp	[fmfrug], 0
	jz ffsub
	jmp	rth_sub
	
schang2:
	;Sound change
	;	Ver 1.1以降で対応
	;	相対番地で、設定が可能
	;
	;sl/rr を　0xff にする
	mov bh,[nch]
	add bh,80h
	mov bl,0ffh
	call opnwrt
	add bh,4
	call opnwrt
	add bh,4
	call opnwrt
	add bh,4
	call opnwrt
	
	;変更ここまで
	
	mov bl,[nch]
	mov bh,28h
	call opnwrt


	mov al,es:[si]
	mov cl,al
	inc si
	mov al,es:[si]
	mov ch,al
	inc si
	
	add	cx, si
	sub	cx, 2
	
	jmp	s1
	
schang: ;Sound change
	
	;Ver 1.04変更点
	
	;sl/rr を　0xff にする
	mov bh,[nch]
	add bh,80h
	mov bl,0ffh
	call opnwrt
	add bh,4
	call opnwrt
	add bh,4
	call opnwrt
	add bh,4
	call opnwrt
	
	;変更ここまで
	
	mov bl,[nch]
	mov bh,28h
	call opnwrt


	mov al,es:[si]
	mov cl,al
	inc si
	mov al,es:[si]
	mov ch,al
	inc si
	;mov word ptr [di+19],si
s1:	push si
	mov si,cx

	mov bh,30h
	mov al,[nch]
	add al,bh
	mov bh,al
	
	rept 4
	
	mov bl,es:[si]
	call opnwrt
	inc si
	inc bh
	inc bh
	inc bh
	inc bh
	
	endm
	
	;フェードアウト中に、音量が大きくならないように
	mov	al, es:[si + 24]
	and	al, 7
	mov	[alg], al
	
	mov bl,es:[si]
	
	cmp	byte ptr [di + 26], 0
	jz	n_f1
		
	cmp	bl, [di + 14]
	ja	n_f1
	
	push	bx
	mov	bl, [alg]
	mov	bh, 0
	mov	al, [offset algot + bx]
	test	al, 1
	pop	bx
	jz	n_f1
	
	mov	bl, [di + 14]
n_f1:
	call opnwrt
	mov [di+14],bl
	inc si
	inc bh
	inc bh
	inc bh
	inc bh
	
	mov bl,es:[si]
	
	cmp	byte ptr [di + 26], 0
	jz	n_f2
	
	cmp	bl, [di + 16]
	ja	n_f2
	
	push	bx
	mov	bl, [alg]
	mov	bh, 0
	mov	al, [offset algot + bx]
	test	al, 4
	pop	bx
	jz	n_f2
	
	mov	bl, [di + 16]
n_f2:

	call opnwrt
	mov [di+16],bl
	inc si
	inc bh
	inc bh
	inc bh
	inc bh
	
	mov bl,es:[si]
	
	cmp	byte ptr [di + 26], 0
	jz	n_f3
		
	cmp	bl, [di + 14]
	ja	n_f3
	
	push	bx
	mov	bl, [alg]
	mov	bh, 0
	mov	al, [offset algot + bx]
	test	al, 2
	pop	bx
	jz	n_f3
	
	mov	bl, [di + 15]
n_f3:

	call opnwrt
	mov [di+15],bl
	inc si
	inc bh
	inc bh
	inc bh
	inc bh
	
	mov bl,es:[si]
	
	cmp	byte ptr [di + 26], 0
	jz	n_f4
		
	cmp	bl, [di + 17]
	ja	n_f4
	
	push	bx
	mov	bl, [alg]
	mov	bh, 0
	mov	al, [offset algot + bx]
	test	al, 8
	pop	bx
	jz	n_f4
	
	mov	bl, [di + 17]
n_f4:

	
	call opnwrt
	mov [di+17],bl
	inc si
	inc bh
	inc bh
	inc bh
	inc bh
	
	rept 20
	mov bl,es:[si]
	call opnwrt
	inc si
	inc bh
	inc bh
	inc bh
	inc bh
	endm
	
	mov al,10h
	add al,bh
	mov bh,al
	mov bl,es:[si]
	call opnwrt
	mov al,bl
	and al,7
	mov [di+13],al
	
	;mov bh,28h
	;mov al,[nch]
	;mov bl,al
	;call opnwrt
	
	mov byte ptr [di+1],0
	
	pop si
	;悪いが、パッチ当てで逃げるぞーーーー
	;mov al,0a4h
	;add al,[nch]
	;mov bh,al
	;mov bl,0
	;call opnwrt
	;
	;sub bh,4
	;call opnwrt
	
s2:	jmp ffsub
	
keytim: mov al,es:[si]
	inc si
	mov [di+23],al
	cmp	[fmfrug], 0
	jz ffsub
	jmp	rth_sub
	
slide:	mov al,es:[si]
	inc si
	mov [di+6],al
	cmp	[fmfrug], 0
	jz ffsub
	jmp	rth_sub
	
zurash:	mov al,es:[si]
	inc si
	mov [di+24],al
	cmp	[fmfrug], 0
	jz ffsub
	jmp	rth_sub
	
fadein:	mov al,es:[si]
	inc si
	mov [di+12],al
	mov [di+26],al
	cmp	[fmfrug], 0
	jz ffsub
	jmp	rth_sub
	
vibini:	mov al,es:[si]
	inc si
	mov [di+25],al

	mov al,es:[si]
	inc si
	mov [di+21],al

	mov al,es:[si]
	inc si
	mov [di+10],al

	mov al,es:[si]
	inc si
	mov [di+22],al

	mov byte ptr [di + 31], 0
	
	cmp	[fmfrug], 0
	jz ffsub
	jmp	rth_sub
	
	
d_vibini:	
	mov al,es:[si]
	inc si
	mov [di+25],al

	mov al,es:[si]
	inc si
	mov [di+21],al

	mov al,es:[si]
	inc si
	mov [di+10],al

	mov al,es:[si]
	inc si
	mov [di+22],al
	
	mov al, es:[si]
	inc si
	mov [di + 31], al
	
	cmp	[fmfrug], 0
	jz ffsub
	jmp	rth_sub
	
jump:	mov si,word ptr es:[si]
	mov al,[fmfrug]
	or al,al
	cmp al,0
	jz ffsub
	cmp al, 1
	jz psub
	jmp	rth_sub
	
jump2:	add si,word ptr es:[si]
	mov al,[fmfrug]
	cmp al,0
	jz ffsub
	cmp al, 1
	jz psub
	jmp rth_sub

iife:
	;YM2608ならばジャンプ
	cmp	[bord86], 1
	jz	iife_jmp
	;ジャンプしない
	add	si, 2
	jmp	iife_rt
	
iife_jmp:
	add	si, word ptr es:[si]
iife_rt:
	mov al,[fmfrug]
	cmp al,0
	jz ffsub
	cmp al, 1
	jz psub
	jmp rth_sub

iifp:
	;PCM機能が使えるならばジャンプ
	cmp	[pcmplas], 1
	jz	iifp_jmp
	;ジャンプしない
	add	si, 2
	jmp	iifp_rt
	
iifp_jmp:
	add	si, word ptr es:[si]
iifp_rt:
	mov al,[fmfrug]
	cmp al,0
	jz ffsub
	cmp al, 1
	jz psub
	jmp rth_sub

lop:
	mov	al, es:[si]
	dec	al
	mov	es:[si], al
	
	cmp	al, 0
	jnz	lop_jp
	;ジャンプしない
	mov	al,es:[si + 1]
	mov	es:[si], al
	inc	si
	inc	si
	inc	si
	inc	si
	
	jmp	lop_1
lop_jp:
	;ジャンプする
	inc	si
	inc	si
	add	si, es:[si]
	
lop_1:
	mov	al, [fmfrug]
	cmp	al, 0
	jz	ffsub
	cmp	al, 1
	jz	psub
	jmp	rth_sub

b_lop:
	mov	al, es:[si]
	dec	al
	mov	es:[si], al
	
	cmp	al, 0
	jz	b_lop_jp
	;ジャンプしない
	inc	si
	inc	si
	inc	si
	inc	si
	
	jmp	b_lop_1
b_lop_jp:
	;ジャンプする
	mov	al,es:[si + 1]
	mov	es:[si], al

	inc	si
	inc	si
	add	si, es:[si]
	
b_lop_1:
	mov	al, [fmfrug]
	cmp	al, 0
	jz	ffsub
	cmp	al, 1
	jz	psub
	jmp	rth_sub
	
	
let:	push di
	mov di,word ptr es:[si]
	inc si
	inc si
	mov al,es:[si]
	inc si
	mov es:[di],al
	pop di
	
	mov al,[fmfrug]
	or al,al
	jz ffsub
	cmp al, 1
	jz psub
	jmp rth_sub

let2:	push di
	mov al,es:[si]
	mov ah,0
	mov di,offset var
	add di,ax
	
	inc si
	mov al,es:[si]
	inc si
	mov [di],al
	pop di
	
	mov al,[fmfrug]
	or al,al
	jz ffsub
	cmp al, 1
	jz psub
	jmp	rth_sub
	
addi:	push di
	mov di,word ptr es:[si]
	inc byte ptr es:[di]
	inc si
	inc si
	pop di
	
	mov al,[fmfrug]
	or al,al
	jz ffsub
	cmp al, al
	jz psub
	jmp	rth_sub

addi2:	push di
	mov di,offset var
	mov al,es:[si]
	mov ah,0
	add di,ax
	inc byte ptr [di]
	inc si
	pop di
	
	mov al,[fmfrug]
	or al,al
	jz ffsub
	cmp al, 1
	jz psub
	jmp rth_sub

subs:	push di
	mov di,word ptr es:[si]
	dec byte ptr es:[di]
	inc si
	inc si
	pop di
	
	mov al,[fmfrug]
	or al,al
	jz ffsub
	cmp al, 1
	jz psub
	jmp rth_sub
	
subs2:	push di
	mov di,offset var
	mov al,es:[si]
	mov ah,0
	add di,ax
	dec byte ptr [di]
	inc si
	pop di
	
	mov al,[fmfrug]
	or al,al
	jz ffsub
	cmp al, 1
	jz psub
	jmp	rth_sub
	
ifthen: push di
	mov di,word ptr es:[si]
	cmp byte ptr es:[di],0
	jnz if1ret
	mov si,word ptr es:[si+2]
	dec si
	dec si
	dec si
	dec si
if1ret:	inc si
	inc si
	inc si
	inc si
	
	pop di
	mov al,[fmfrug]
	or al,al
	jz ffsub
	cmp al, 1
	jz psub
	jmp	rth_sub
	
iifthen:push di
	mov di,offset var
	mov al,es:[si]
	mov ah,0
	add di,ax
	cmp byte ptr [di],0
	jnz iif1ret
	inc si
	add si,word ptr es:[si]
	dec si
	dec si
	dec si
iif1ret:inc si
	inc si
	inc si
	
	pop di
	mov al,[fmfrug]
	or al,al
	jz ffsub
	cmp al, 1
	jz psub
	jmp	rth_sub
	
rth_volum:
	mov al, [di + 12]
	cmp al, 0
	jz	rth_v1
	
	jmp	rth_sub
	
rth_v1:
	mov al, es:[si]
	inc	si
	
	mov	bh, 11h
	mov	bl, al
	mov	[di + 14], al
	
	call	opnwrt
	
	jmp	rth_sub
	
	
volum11:mov al,[di+12]
	cmp al,0
	jz volum1
	inc si
	cmp	[fmfrug], 0
	jz ffsub
	jmp	rth_sub

lfo:
	mov al, es:[si]
	inc	si
	mov	bh, 22h
	mov	bl, al
	call	opnwrt
	jmp	ffsub

lr:
	;LR設定
	mov	al, es:[si]
	inc	si
	sal	al, 1
	sal	al, 1
	sal	al, 1
	sal	al, 1
	sal	al, 1
	sal	al, 1
	mov	bh, 0b4h
	add	bh, [nch]
	mov	bl, al
	push	di
	mov	di, offset buf188
	cmp	[ext86], 0
	jz	lr_1
	mov	di, offset buf18a
lr_1:
	mov	al, bh
	mov	ah, 0
	add	di, ax
	mov	al, [di]
	and	al, 03fh
	or	bl, al
	pop	di
	call	opnwrt
	
	jmp	ffsub
	
amspms:
	;ams/pms
	mov	al, es:[si]
	inc	si
	mov	bh, 0b4h
	add	bh, [nch]
	mov	bl, al
	
	push	di
	mov	di, offset buf188
	cmp	[ext86], 0
	jz	amspms_1
	mov	di, offset buf18a
amspms_1:
	mov	al, bh
	mov	ah, 0
	add	di, ax
	mov	al, [di]
	and	al, 0c0h
	or	bl, al
	pop	di
	call	opnwrt
	
	;amon設定
	mov	al, es:[si]
	inc	si
	push	di
	mov	di, offset buf188
	cmp	[ext86], 0
	jz	amspms_2
	mov	di, offset buf18a
amspms_2:
	mov	bl, [nch]
	mov	bh, 0
	add	bl, 060h
	add	di, bx
	mov	bh, bl
	
	push	cx
	mov	cx, 4
amspms_4:
	push	cx
	shr	al, 1
	jnc	amspms_3
	
	mov	bl, [di]
	or	bl, 80h
	push	ax
	call	opnwrt
	pop	ax
	
amspms_3:
	add	di, 4
	add	bh, 4
	
	pop	cx
	loop	amspms_4
	pop	cx
	pop	di
	
	
	jmp	ffsub
	
volum1: mov al,[di+13]
	mov ah,0
	mov bx,offset algot
	add bx,ax
	mov al,es:[si]
	inc si
	mov ah,[bx]
	
	test ah,1
	jz v1a
	call op1
v1a:	test ah,2
	jz v1b
	call op2
v1b:	test ah,4
	jz v1c
	call op3
v1c:	test ah,8
	jz v1d
	call op4
v1d:	
s3:	jmp ffsub
	
op1:	mov [di+14],al
	mov bl,al
	mov bh,40h
	mov al,[nch]
	add al,bh
	mov bh,al
	call opnwrt
	ret
	
op2:	mov [di+15],al
	mov bl,al
	mov bh,48h
	mov al,[nch]
	add al,bh
	mov bh,al
	call opnwrt
	ret

op3:	mov [di+16],al
	mov bl,al
	mov bh,44h
	mov al,[nch]
	add al,bh
	mov bh,al
	call opnwrt
	ret

op4:	mov [di+17],al
	mov bl,al
	mov bh,4ch
	mov al,[nch]
	add al,bh
	mov bh,al
	call opnwrt
	ret

volum2: mov al,[di+13]
	mov ah,0
	mov bx,offset algot
	add bx,ax
	mov al,es:[si]
	inc si
	mov ah,[bx]
	
	test ah,1
	jnz v2a
	call op1
v2a:	test ah,2
	jnz v2b
	call op2
v2b:	test ah,4
	jnz v2c
	call op3
v2c:	test ah,8
	jnz v2d
	call op4
v2d:	
	cmp	[fmfrug], 0
	jz ffsub
	jmp	rth_sub
	
volum3: mov al,es:[si]
	inc si
	mov ah,al
	mov al,es:[si]
	inc si
	
	cmp ah,0
	jnz v3a
	call op1
v3a:	cmp ah,1
	jnz v3b
	call op2
v3b:	cmp ah,3
	jnz v3c
	call op3
v3c:	cmp ah,4
	jnz v3d
	call op4
v3d:	
	cmp	[fmfrug], 0
	jz ffsub
	jmp	rth_sub
	
ifthen2:push di
	mov di,word ptr es:[si]
	cmp byte ptr es:[di],0
	jz if2ret
	dec byte ptr es:[di]	;これを忘れたために２時間費やした
	mov si,word ptr es:[si+2]
	dec si
	dec si
	dec si
	dec si
if2ret:	inc si
	inc si
	inc si
	inc si
	pop di
	mov al,[fmfrug]
	or al,al
	jz ffsub
	cmp al, 1
	jz psub
	jmp rth_sub

iifthen2:
	push di
	mov di,offset var
	mov al,es:[si]
	mov ah,0
	add di,ax
	cmp byte ptr [di],0
	jz iif2ret
	dec byte ptr [di]	;これを忘れたために２時間費やした
	inc si
	add si,word ptr es:[si]
	dec si
	dec si
	dec si
iif2ret:inc si
	inc si
	inc si
	pop di
	mov al,[fmfrug]
	or al,al
	jz ffsub
	cmp al, 1
	jz psub
	jmp	rth_sub
	
temp:	mov al,es:[si]
	inc si
	mov [tempo],al
	mov	[temposetflag], 1
	mov al,[fmfrug]
	or al,al
	jz ffsub
	cmp al, 1
	jz psub
	jmp	rth_sub

tempochange:	;テンポをblで設定された値に変更する
	;
	cmp	[bordf], 0
	jnz	tempochange_f
	ret
tempochange_f:
	mov	[tempo], bl
	cli
	mov bh,26h
	call opnwrt
	sti
	
	ret

opnco1:	mov al,es:[si]
	mov bh,al
	inc si
	mov al,es:[si]
	mov bl,al
	inc si
	call opnwrt
	mov al,[fmfrug]
	or al,al
	jz ffsub
	cmp al, 1
	jz psub
	jmp	rth_sub
	
opnco2:	mov al,es:[si]
	mov bh,al
	inc si
	mov al,es:[si]
	mov bl,al
	inc si
	call opnwrt
	ret
	
;
;	psg play
;

psgply:	
	mov si,di
	mov al,[si]
	or al,al
	jz pchang
	;
pplay:	dec al
	mov [si],al
	inc si
	mov al,[si]
	or al,al
	jnz lp1
	ret	;ret z
lp1:	inc si
	mov al,[si]
	or al,al
	jz lp2
	ret	;ret nz
lp2:	inc si
	;
	mov bh,7
	mov al,[tn]
	mov bl,al
	call opnwrt
	;
	
	mov al,[di+8]
	or al,al
	jnz prevf
	inc si
	mov al,[si]
	or al,al
	jnz veo
	
	mov bh,[di+9]
	inc si
	inc si
	inc si
	jmp vfade
	
prevf:	mov bh,[di+9]
	mov al,bh
	or al,al
	jz vfr
	dec byte ptr [di+27]
	jnz vfr
	mov al,[di+8]
	mov [di+27],al
	dec bh
vfr:	inc si
	nop
	mov [volkk],bh		;cs:
	cmp byte ptr [si],0
	jnz veo
	inc si
	inc si
	inc si
	jmp vfade
	
veo:	inc si
	push di
	mov di,word ptr [si]
	mov cx,di
	mov al,es:[di]
	pop di
	inc si
	inc si
	mov bl,byte ptr [si]
	mov bh,0
	
	test al,80h
	jz ve1
	and al,0fh
	mov [di+9],al
	mov bh,al
	mov bx,0
	
ve1:	add cx,bx
vfade2:	mov word ptr [di+5],cx
	mov byte ptr [di+9],al
	mov bh,al
	cmp byte ptr [di+8],0
	jz vfade
	inc si
	inc si
	mov al,[volkk]		;cs:
	mov [si],al
	mov al,[nch]
	add al,8
	cmp bh,[volkk]		;cs:
	jl noset
	mov bh,[volkk]		;cs:
	jmp noset
	
	;
vfade:	inc si
vopr:	inc si
	mov al,[nch]
	add al,8
	
	mov [si],bh
noset:
	inc si
	push bx
	mov bl,[nch]
	mov ah,bh
	xor bh,bh
	cmp byte ptr [bx+offset voicef-1],0
	jnz vpass1	;音声出力中ならボリュームの変更はなし
	
vcok:
	push ax
	mov al,[shift]
	mov ah,[ston]
	and al,ah
	pop ax
	
	jnz vpass1
	
	cmp byte ptr [di+2],0
	jnz vpass1
	
	mov bl,ah
	mov bh,al
	call opnwrt
vpass1:	pop bx
	
	;
	mov al,[si]
	and al,1
	jnz noise
	inc si
	push si
	mov cx,word ptr [si]
	inc si
	inc si
	mov al,[si]
	mov bl,al
	mov bh,0
	cmp al,80h
	jb p1
	dec bh
p1:	add cx,bx
;
pv1b:	inc si
	mov al,[si]
	or al,al
	jz pv1
	dec al
	mov [si],al
	jmp pout
pv1:	inc si
	mov al,[si]
	or al,al
	jz pv2
	dec al
	mov [si],al
	jmp pout
pv2:	mov al,[di+23]
	mov [si],al
	inc si
	mov al,[si]
	or al,al
	jnz pkakou
pjosho: mov bh,0
	inc si
	mov al,[si]
	mov bl,al
	add cx,bx
	inc si
	dec byte ptr [si]
	jnz pout
	mov al,[di+24]
	add al,al
	mov [si],al
	mov byte ptr [di+16],1
	jmp pout
pkakou: inc si
	mov bh,0
	mov al,[si]
	mov bl,al
	neg	bx
	add cx,bx
	inc si
	dec byte ptr [si]
	jnz pout
	mov al,[di+24]
	add al,al
	mov [si],al
	mov byte ptr [di+16],0
;
pout:	pop si
	mov al,[nch]
	mov bl,al
	xor bh,bh
	cmp byte ptr [offset voicef-1+bx],0
	jnz vpass2	;音声出力中は、周波数変更もなし
	
	add al,al
	mov bh,al
	mov bl,cl
	call opnwrt
	inc bh
	mov bl,ch
	call opnwrt
vpass2:	mov [si],cx
	inc si
	;
noise:	mov al,[di+10]
	and al,8
	jz lp3
	ret
lp3:	mov bl,[di+19]
	dec byte ptr [di+20]
	jnz nout
	mov al,[di+28]
	mov [di+20],al
	mov al,[di+21]
	add al,bl
	mov bl,al
nout:	mov bh,6
	call opnwrt
	mov [di+19],bl
	ret
	;
	;change next (psg)
	;
	
pchang:	mov si,word ptr [di+29]
	;
psub:	mov al,es:[si]
	mov word ptr [di + 29], si
	inc si
	test al,80h
	jnz pfunc
	;
plengt: mov [di+0],al
	mov al,[di+3]
	cmp al,1
	jz pc1
	cmp al,2
	jnz pb2
	dec byte ptr [di+3]
	mov al,[di+2]
	or al,al
	jnz pb1
pb2:	mov al,[nch]
	add al,8
	mov bh,al
	mov bl,0
	call opnwrt
pb1:	mov al,[di+22]
	mov [di+14],al
	mov al,[di+23]
	mov [di+15],al
	mov al,[di+24]
	mov [di+18],al
	mov al,[di+25]
	mov [di+5],al
	mov al,[di+26]
	mov [di+6],al
	;
pc1:	mov al,es:[si]
	mov [di+1],al
	test byte ptr [di+10],1
	jnz pnoise
	and al,0fh
	mov cx,offset pftab-2
	add al,al
	mov bl,al
	mov bh,0
	add cx,bx
	;
	push di
	mov di,cx
	mov bl,[di]
	inc di
	mov bh,[di]
	pop di
	mov al,es:[si]
	inc si
	and al,0f0h
	shr al,1
	shr al,1
	shr al,1
	shr al,1
ll1:	jz ll2
	shr bx,1
	dec al
	jmp ll1
ll2:	mov ch,0
	mov al,[di+31]
	cmp al,80h
	jb pc2
	dec ch
pc2:	mov cl,al
	add cx,bx
	;ポルタメント中　かつ　タイオンなら　周波数は再設定しない
	;cmp byte ptr [di+3],0
	;jz pnonpolt
	;cmp byte ptr [di+13],0
	;jnz ppolt
	
;pnonpolt:
	mov [di+11],cx
ppolt:	mov [di+29],si
	jmp psgply
	;
pnoise: inc si
	mov [di+19],al
	mov [di+29],si
	jmp psgply
	
;
pfunc:	and al,7fh
	add al,al
	mov cx,offset pjptab
	mov bl,al
	mov bh,0
	add bx,cx
	mov cx,[bx]
	jmp cx


	;
	;
	;ｺﾏﾝﾄﾞ ｴﾝﾄﾘｰ (psg)
	;
	;
	
ptieof:	mov byte ptr [di+3],0
	mov al,[nch]
	add al,8
	mov bh,al
	mov bl,0
	call opnwrt
	jmp psub
ptieon: mov byte ptr [di+3],2
	jmp psub
pnt:	mov al,es:[si]
	mov [di+10],al
	inc si
	mov bh,al
	or bh,11000000B
	mov al,[nch]
	or al,al
	jz pn2
pn1:	stc
	rcl bh,1
	dec al
	jnz pn1
pn2:	mov bl,24h
	mov al,[nch]
	or al,al
	jnz pn3
	mov bl,9
	jmp pn5
pn3:	cmp al,1
	jnz pn4
	mov bl,12h
	jmp pn5
pn4:	
pn5:	mov al,[tn]
	or al,bl
	and al,bh
	mov [tn],al
	jmp psub

penvch2:
	push	ax
	mov	ax, es:[si]
	add	ax, si
	add	si, 2
	mov	[di + 25], ax
	pop	ax
	jmp	psub
	
penvch:	mov al,es:[si]
	inc si
	mov [di+25],al
	mov al,es:[si]
	inc si
	mov [di+26],al
	jmp psub
	
penvof: mov al,es:[si]
	mov [di+4],al
	inc si
	jmp psub

evolum: mov al,[di+8]
	cmp al,0
	jz evolum2
	inc si
	jmp psub
evolum2:
	mov al,es:[si]
	mov [di+9],al
	inc si
	jmp psub
	
espeed: mov al,es:[si]
	mov [di+7],al
	inc si
	jmp psub

pnset:  mov al,es:[si]
	mov [di+19],al
	inc si
	jmp psub
	
pslide: mov al,es:[si]
	mov [di+13],al
	inc si
	jmp psub
	
pvibr:	mov al,es:[si]
	inc si
	mov [di+22],al
	mov al,es:[si]
	inc si
	mov [di+23],al
	mov al,es:[si]
	inc si
	mov [di+17],al
	mov al,es:[si]
	inc si
	mov [di+24],al
	jmp psub
	
pzura:	mov al,es:[si]
	inc si
	mov [di+31],al
	jmp psub
	
pfouts: mov al,es:[si]
	inc si
	mov [di+27],al
	mov [di+8],al
	jmp psub
	
pend:	;jmp psub
	cmp	[fmfrug], 0
	jz	pend_fm
	cmp	[fmfrug], 1
	jz	pend_psg
pend_rth:
	;リズム
	mov	ax, 1
	mov	cl, [nch]
	sal	ax, cl
	or	[allstopflag], ax
	jmp	pend_2
pend_fm:
	;FM
	mov	ax, 1000000B
	cmp	[ext86], 0
	jz	pend_fm1
	mov	ax, 1000000000B
pend_fm1:
	mov	cl, [nch]
	sal	ax, cl
	or	[allstopflag], ax
	jmp	pend_2
	
pend_psg:
	;PSG
	mov	ax, 1000000000000B
	mov	cl, [nch]
	sal	ax, cl
	or	[allstopflag], ax
	jmp	pend_2
	
pend_2:
	cmp	[allstopflag], 0111111111111111B
	jnz	pend_1
	
	push di
	push si
	call c_mend
	pop si
	pop di

pend_1:	
	dec si
	ret
	
pnslid:	mov al,es:[si]
	inc si
	mov [di+28],al
	mov al,es:[si]
	inc si
	mov [di+21],al
	jmp psub
	
pinte:  mov al,[nch]
	mov cx,offset play4
	rol al,1
	rol al,1
	rol al,1
	rol al,1
	rol al,1
	mov bl,al
	mov bh,0
	add bx,cx
	push bx
	;その割り込みチャンネルの音を止める
	mov bh,[nch]
	add bh,8
	mov bl,0
	call opnwrt	;volum = 0
	pop bx

settn:	;tn を　戻す
	;bx に　ワーク先頭
	push di
	push bx
	mov di,bx
	mov al,[di+10]
	mov bh,al
	or bh,11000000B
	mov al,[nch]
	or al,al
	jz ppn2
ppn1:	stc
	rcl bh,1
	dec al
	jnz ppn1
ppn2:	
	mov al,[nch]
	mov bl,24h
	or al,al
	jnz ppn3
	mov bl,9
	jmp ppn5
ppn3:	cmp al,1
	jnz ppn5
	mov bl,12h
	
ppn5:	mov al,[tn]
	or al,bl
	and al,bh
	mov [tn],al
	
	pop bx
	pop di
	
	inc bx
	inc bx
	mov byte ptr [bx],0
	ret
	
	;expand Command
	;
stemp:	mov al,es:[si]
	inc si
	mov ah,92
	mul ah
	mov cx,1024
	sub cx,ax		;周期は、１０２４－９２ｎ
	mov ax,[timera]
	mov [ptimera],ax
	mov [timera],cx
	mov cx,[timera]		;タイマーＡ周期
	cli
	mov bh,25h
	mov bl,cl
	and bl,3
	call opnwrt
	
	mov bh,24h
	shr cx,1
	shr cx,1
	mov bl,cl
	call opnwrt
	sti
	
	jmp psub
	
henvsw: mov al,es:[si]
	inc si
	or al,al
	jz sjp5
	mov al,[di+9]
	or al,10h
	mov bl,al
	mov [di+9],al
	mov al,[nch]
	add al,8
	mov bh,al
	call opnwrt
	jmp psub
sjp5:	and al,0fh
	mov bl,al
	mov [di+9],al
	mov al,[nch]
	add al,8
	mov bh,al
	call opnwrt
	jmp psub
	
henvsp: mov al,es:[si]
	inc si
	mov bl,al
	mov bh,11
	call opnwrt
	mov al,es:[si]
	inc si
	mov bl,al
	mov bh,12
	call opnwrt
	jmp psub
	
henvfm: mov al,es:[si]
	inc si
	mov bl,al
	mov bh,13
	call opnwrt
	jmp psub
	
exon:	mov [exflag],1
	jmp psub
exoff:	mov [exflag],0
	jmp psub
ips:	mov al,es:[si]
	inc si
	mov [tbf],al
	jmp psub
settimera:
	mov ax,word ptr es:[si]
	inc si
	inc si
	mov cx,[timera]
	mov [ptimera],cx
	mov [timera],ax
	mov cx,[timera]		;タイマーＡ周期
	cli
	mov bh,25h
	mov bl,cl
	and bl,3
	call opnwrt
	
	mov bh,24h
	shr cx,1
	shr cx,1
	mov bl,cl
	call opnwrt
	sti
	
	jmp psub
	

voice:	;疑似音声合成
	mov [voicedouble],0
	mov bl,[nch]
	xor bh,bh
	dec bl
	cmp bl,0
	jz voice2
	mov [voicef2],1
	mov bx,offset v2off
	mov ah,0fbh
	push ax
	jmp voice3
voice2: mov [voicef],1
	mov bx,offset v1off
	mov ah,0fdh
	push ax
voice3:	
	mov ax,es:[si]
	inc si
	inc si
	mov [bx],ax
	inc bx
	inc bx
	mov ax,es:[si]
	inc si
	inc si
	mov [bx],ax
	inc bx
	inc bx
	mov ax,es:[si]
	mov [bx],ax
	inc bx
	inc bx
	mov byte ptr [bx],0
	
	inc si
	inc si
	;周波数は０
	mov al,[nch]
	sal al,1
	mov bh,al
	mov bl,0
	call opnwrt
	inc bh
	mov bl,0
	call opnwrt
	;ボリュームも０
	mov al,[nch]
	add al,8
	mov bh,al
	mov bl,0
	call opnwrt
	;トーンモードの設定
	pop ax
	mov al,7
	mov dx,[ioport1]
	out dx,al
	jmp $+2
	jmp $+2
	mov dx,[ioport2]
	in al,dx
	and al,ah
	jmp $+2
	jmp $+2
	out dx,al
	;
	mov [tbf],1
	mov [exflag],1
	
	ret
	;全然関係無いけど、「ＲＥＴ」をカナで打つと「スイカ」になるんだね

voice2ch:
	;疑似音声合成同時２ＣＨ発声
	mov [timerbplay],1
	mov [voicef],1
	mov [voicef2],1
	mov ax,es:[si]
	inc si
	inc si
	mov [v1off],ax
	mov [v2off],ax
	mov ax,es:[si]
	inc si
	inc si
	mov [v1seg],ax
	mov [v2seg],ax
	mov ax,es:[si]
	mov [v1byte],ax
	mov [v2byte],ax
	mov [v1sel],0
	mov [v2sel],0
	mov [voicedouble],1
	inc si
	inc si
	;周波数は０
	mov bh,2
	mov bl,0
	call opnwrt
	inc bh
	call opnwrt
	inc bh
	call opnwrt
	inc bh
	call opnwrt
	
	;ボリュームも０
	mov bh,9
	call opnwrt
	inc bh
	call opnwrt
	
	;トーンモードの設定
	mov al,[tn]
	or al,00110000B
	and al,11111001B
	mov [tn],al
	mov bh,7
	mov bl,al
	call opnwrt
	
	;
	mov [tbf],1
	mov [exflag],1
	
	ret
	;全然関係無いけど、「ＲＥＴ」をカナで打つと「スイカ」になるんだね

sstart: 
	mov [exflag],1
	ret
send:	
	mov [exflag],0
	ret
sinton: 
	cmp	[bordf], 0
	jnz	sinton_f
	ret
sinton_f:
	mov [exflag],1
	mov si,bx
	mov es,[useg]
	mov al,es:[bx]
	mov cl,al
	and al,3fh
	test cl,80h
	jz spj1
	mov di,offset play5+2
	cmp [voicef],0
	jnz spj1
	cmp al,[di]
	jb spj1
	mov [di],al
	mov di,offset int1
	call swset
spj1:	test cl,40h
	jz sjp2
	mov di,offset play6+2
	cmp [voicef2],0
	jnz sjp2
	cmp al,[di]
	jb sjp2
	mov [di],al
	mov di,offset int2
	add	si, 2
	call swset
sjp2:	ret

sinton2: 
	cmp	[bordf], 0
	jnz	sinton2_f
	ret
sinton2_f:
	mov [exflag],1
	mov si,bx
	;mov es,[useg]
	mov	[seseg], es
	mov al,es:[bx]
	mov cl,al
	and al,3fh
	test cl,80h
	jz ss_spj1
	mov di,offset play5+2
	cmp [voicef],0
	jnz spj1
	cmp al,[di]
	jb ss_spj1
	mov [di],al
	mov di,offset int1
	call swset
	
ss_spj1:	
	test cl,40h
	jz ss_sjp2
	mov di,offset play6+2
	cmp [voicef2],0
	jnz ss_sjp2
	cmp al,[di]
	jb ss_sjp2
	mov [di],al
	mov di,offset int2
	add si, 2
	call swset
ss_sjp2:
	;OPN演奏中かどうか
	cmp	[pflag], 0
	jz	ss_sjp_rt	;演奏中
	
	;ダミーデータ演奏
	mov	ax, cs
	mov	es, ax
	mov	bx, offset dummyopn
	
	call	mss1
	
ss_sjp_rt:

	ret

swset:  push ax
	push di
	mov al,0
	rept 20h
	mov [di],al
	inc di
	endm
	pop di
	inc si
	mov al,es:[si]
	mov [di+29],al
	inc si
	mov al,es:[si]
	mov [di+30],al
	sub	si, 2
	pop ax
	ret
	
	
	
	;
	;external fade out
	;
	;		bh<--fm speed bl<---psg speed
	;
mfout:	
	cmp [bordf], 0
	jnz	mfout_f
	ret
mfout_f:
	mov di,offset play1
	call meput1
	mov di,offset play2
	call meput1
	mov di,offset play3
	call meput1
	mov di,offset play4
	call meput2
	mov di,offset play5
	call meput2
	mov di,offset play6
	call meput2
	mov di, offset play7
	call meput1
	mov di, offset play8
	call meput1
	mov di, offset play9
	call meput1
	
	mov di, offset play10
	call meput1
	mov di, offset play11
	call meput1
	mov di, offset play12
	call meput1
	mov di, offset play13
	call meput1
	mov di, offset play14
	call meput1
	mov di, offset play15
	call meput1
	
	ret
	
meput1: mov [di+12],bh
	mov [di+26],bh
	ret
meput2: mov [di+8],bl
	mov [di+27],bl
	ret
	
	;
	;music clear
	;
	
mend:	
	cmp [bordf], 0
	jnz	mend_f
	jmp	mend_ret
mend_f:
	cmp [pflag],1
	jnz menh
	jmp	mend_ret
menh:	
	mov al,1
	mov [pflag],al
	mov [musicp],0
	mov al,[voicef]
	or al,[voicef2]
	jz mend2
	;mov ax,[ptimera]
	;mov [timera],ax
mend2:	mov [voicef],0
	mov [voicef2],0
	mov dl,[nch]
	
	;
	mov cx,offset nulvis
	push cx
	xor al,al
	mov [nch],al
	call s13
	pop cx
	push cx
	mov al,1
	mov [nch],al
	call s13
	pop cx
	push	cx
	mov al,2
	mov [nch],al
	call s13
	pop	cx
	;
	cmp	[bord86], 0
	jz	mend__1
	
	;拡張部分KEY OFF
	push cx
	mov [nch],0
	call s13_2
	pop cx
	push cx
	mov [nch],1
	call s13_2
	pop cx
	push	cx
	mov [nch],2
	call s13_2
	pop	cx
	
mend__1:
	;
	mov bx,073fh
	call opnwrt
	
	;タイマーの停止
	mov bx,2730h
	call opnwrt
	
	mov [nch],dl
	
mend_ret:
	
	ret

c_mend:	
	cmp [bordf], 0
	jnz	c_mend_f
	ret
c_mend_f:
	cmp [pflag],1
	jnz 	c_menh
	ret
c_menh:	
	mov al,1
	mov [pflag],al
	mov [musicp],0
	mov al,[voicef]
	or al,[voicef2]
	jz c_mend2
	;mov ax,[ptimera]
	;mov [timera],ax
c_mend2:	mov [voicef],0
	mov [voicef2],0
	mov dl,[nch]
	
	;
	mov cx,offset nulvis
	push cx
	xor al,al
	mov [nch],al
	call s13
	pop cx
	push cx
	mov al,1
	mov [nch],al
	call s13
	pop cx
	push	cx
	mov al,2
	mov [nch],al
	call s13
	pop	cx
	;
	cmp	[bord86], 0
	jz	mend__1
	
	;拡張部分KEY OFF
	push cx
	mov [nch],0
	call s13_2
	pop cx
	push cx
	mov [nch],1
	call s13_2
	pop cx
	push	cx
	mov [nch],2
	call s13_2
	pop	cx
	
c_mend__1:
	;
	mov bx,073fh
	call opnwrt
	
	;タイマーの停止
	mov bx,2730h
	call opnwrt
	
	mov [nch],dl
	
	ret
	
s13:	push si
	push di
	mov si,cx
	
	mov di,offset work
	mov al,[nch]
	sal al,1
	sal al,1
	sal al,1
	sal al,1
	sal al,1
	xor ah,ah
	add di,ax
	
	
	mov bh,30h
	mov al,[nch]
	add al,bh
	mov bh,al
	
	rept 4
	
	mov bl,[si]
	call opnwrt
	inc si
	inc bh
	inc bh
	inc bh
	inc bh
	
	endm
	
	mov bl,[si]
	call opnwrt
	mov [di+14],bl
	inc si
	inc bh
	inc bh
	inc bh
	inc bh
	
	mov bl,[si]
	call opnwrt
	mov [di+16],bl
	inc si
	inc bh
	inc bh
	inc bh
	inc bh
	
	mov bl,[si]
	call opnwrt
	mov [di+15],bl
	inc si
	inc bh
	inc bh
	inc bh
	inc bh
	
	mov bl,[si]
	call opnwrt
	mov [di+17],bl
	inc si
	inc bh
	inc bh
	inc bh
	inc bh
	
	rept 20
	mov bl,[si]
	call opnwrt
	inc si
	inc bh
	inc bh
	inc bh
	inc bh
	endm
	
	mov al,10h
	add al,bh
	mov bh,al
	mov bl,[si]
	call opnwrt
	mov al,bl
	and al,7
	mov [di+13],al
	
	pop di
	pop si
	
	ret

s13_2:	push si
	push di
	mov [ext86], 1
	mov si,cx
	
	mov di,offset work
	mov al,[nch]
	add al, 6
	sal al,1
	sal al,1
	sal al,1
	sal al,1
	sal al,1
	xor ah,ah
	add di,ax
	
	
	mov bh,30h
	mov al,[nch]
	add al,bh
	mov bh,al
	
	rept 4
	
	mov bl,[si]
	call opnwrt
	inc si
	inc bh
	inc bh
	inc bh
	inc bh
	
	endm
	
	mov bl,[si]
	call opnwrt
	mov [di+14],bl
	inc si
	inc bh
	inc bh
	inc bh
	inc bh
	
	mov bl,[si]
	call opnwrt
	mov [di+16],bl
	inc si
	inc bh
	inc bh
	inc bh
	inc bh
	
	mov bl,[si]
	call opnwrt
	mov [di+15],bl
	inc si
	inc bh
	inc bh
	inc bh
	inc bh
	
	mov bl,[si]
	call opnwrt
	mov [di+17],bl
	inc si
	inc bh
	inc bh
	inc bh
	inc bh
	
	rept 20
	mov bl,[si]
	call opnwrt
	inc si
	inc bh
	inc bh
	inc bh
	inc bh
	endm
	
	mov al,10h
	add al,bh
	mov bh,al
	mov bl,[si]
	call opnwrt
	mov al,bl
	and al,7
	mov [di+13],al
	
	mov [ext86], 0
	pop di
	pop si
	
	ret


nulvis	db 0,0,0,0,127,127,127,127
	db 31,31,31,31,31,31,31,31
	db 31,31,31,31,255,255,255,255
	db 0,0,0,0,0


pcmstop:
	;PCM停止
	cmp	[pcmplas], 0
	jnz	pcmstop_1
	
	ret
	
pcmstop_1:
	mov	dx, 0a468h
	mov	al, 0
	out	dx, al
	
	mov	[pcmflag], 1
	mov	[pcm_j], 0ffffh
	ret

f_pcmstop:
	;PCM停止(音源部より操作)
	cmp	[pcmplas], 0
	jnz	f_pcmstop_1
	
	jmp	f_pcmstop_ed
	
f_pcmstop_1:
	mov	dx, 0a468h
	mov	al, 0
	out	dx, al
	
	mov	[pcmflag], 1
	mov	[pcm_j], 0ffffh
	
f_pcmstop_ed:
	mov al,[fmfrug]
	or al,al
	cmp al,0
	jz ffsub
	cmp al, 1
	jz psub
	jmp	rth_sub

c_pcmstop:
	;PCM停止(音源部より操作)
	cmp	[pcmplas], 0
	jnz	c_pcmstop_1
	
	jmp	c_pcmstop_ed
	
c_pcmstop_1:
	mov	dx, 0a468h
	mov	al, 0
	out	dx, al
	
	mov	[pcmflag], 1
	mov	[pcm_j], 0ffffh
	
c_pcmstop_ed:
	ret

f_pcmstart:
	;FIFOスタート
	cmp	[pcmplas], 0
	jnz	f_pcmstart_2
	
	jmp	f_pcmstart_ed
	
f_pcmstart_2:
	push	ax
	push	dx
	
	;FIFO割り込みフラグリセット
	mov	dx, 0a468h
	in	al, dx
	
	;トリガOFF
	and	al, 11101111B
	out	dx, al
	
	;トリガON
	or	al, 00010000B
	out	dx, al
	
	;FIFOスタート
	or	al, 10000000B
	out	dx, al
	
	;割り込み許可
	or	al, 00100000B
	out	dx, al

	mov	[pcm_j], 0ffffh
	
	pop	dx
	pop	ax

f_pcmstart_ed:

	mov al,[fmfrug]
	or al,al
	cmp al,0
	jz ffsub
	cmp al, 1
	jz psub
	jmp	rth_sub

r_pcmstart:
	;PCM再生スタート常駐版
	;	es:bx = データ先頭オフセット
	;	dx:cx = 再生サイズ
	
	push	dx
	push	cx
	
	add	cx, 15
	adc	dx, 0
	
	rept	4
	shr	dx, 1
	rcr	cx, 1
	endm
	
	cmp	cx, [pcmbuffersize]
	
	jbe	r_pcm2
	
	;メモリが足りない
	
	pop	cx
	pop	dx
	
	pop bx		;return addr dummy
	pop ax		
	
	pop es
	pop ds
	pop si
	pop di
	pop dx
	pop cx
	pop bx
	
	mov	ax, 0ffffh
	iret
	
r_pcm2:
	push	di
	push	si
		
	push	ds
	push	es
	
	mov	si, bx
	mov	di, 0
	
	mov	ax, es
	mov	ds, ax
	
	mov	es, cs:[pcmbuffer]

r_pcm3:
	cld
	rept	16
	movsb
	endm
	loop	r_pcm3
	
	pop	es
	pop	ds
	pop	si
	pop	di
	
	mov	es, [pcmbuffer]
	mov	bx, 0
	
	pop	cx
	pop	dx
	
	call	pcmstart
	
	pop bx		;return addr dummy
	pop ax		
	
	pop es
	pop ds
	pop si
	pop di
	pop dx
	pop cx
	pop bx
	
	mov	ax, 0
	iret
	
	
pcmstart:
	;PCM再生スタート
	cmp	[pcmplas], 0
	jnz	pcms_1
	
	;PCM機能がない
	ret
	
pcms_1:
	cmp	[pcmflag], 0
	jnz	pcm__p1
	
	;ストップ
	call	pcmstop
	
pcm__p1:

	;入力
	;
	;	es:bx = データ先頭オフセット
	;
	pushf
	cli
	
	mov	si, bx
	mov	al, es:[si]
	mov	bh, al
	and	bh, 7
	mov	bl, al
	and	bl, 0f0h
	
	inc	si
	mov	cx, es:[si]
	add	si, 2
	mov	dx, es:[si]
	add	si, 2
	
	mov	[pcmoffset], si
	mov	[pcmoffset + 2], es
	
	mov	[pcmsize], cx
	mov	[pcmsize + 2], dx

	;割り込みアドレスセット
	push	bx
	
	
	;すでにサウンド割り込みがセットされているか
	mov	ah, 35h
	mov	al, [musicvect]
	add	al, 8
	int	21h
	
	cmp	bx, offset mdrv
	jnz	_init2
	mov	ax, es
	mov	cx, cs
	cmp	ax, cx
	jnz	_init2

	jmp	_init1


_init2:
	;セットされていない
	mov	[vecsoundoff], bx
	mov	[vecsoundseg], es
	mov	[vectsetflag], 1
_init1:
	mov	ah, 25h
	mov	al, [musicvect]
	add	al, 8
	mov	dx, offset mdrv
	int	21h
	
	;割り込み許可
	test	[musicvect], 8
	jnz	_vect8_15
	
	;vect0-7
	in	al, 2
	mov	ah, [maskf]
	not	ah
	and	al, ah
	jmp	$+2
	jmp	$+2
	out	2, al
	jmp	_vectnoed
	
_vect8_15:
	;vect8-15
	in	al, 0ah
	mov	ah, [maskf]
	not	ah
	and	al, ah
	jmp	$+2
	jmp	$+2
	out	0ah, al
	
_vectnoed:
	
	pop	bx
	
	;FIFO割り込み禁止
	mov	dx, 0a468h
	in	al, dx
	
	mov	al, 0	;@@test
	
	and	al, 11011111B
	out	dx, al
	
	;FIFOリセット
	in	al, dx
	or	al, 00001000B
	out	dx, al
	
	and	al, 11110111B
	out	dx, al
	
	;CPU ->FIFO方向設定
	in	al, dx
	and	al, 10111111B
	out	dx, al
	
	;サンプリングレート
	and	al, 11111000B
	and	bh, 00000111B
	or	al, bh
	out	dx, al
	
	;量子化ビット数/パンポットをセット
	mov	dx, 0a46ah
	
	mov	al, bl
	and	al, 01110000B
	or	al, 00000010B
	out	dx, al
	
	
	;割り込みデータ長設定
	pushf
	push	dx
	push	ax
	cli
	mov	ax, TIME_PCMSIZE
	sal	ax, 1
	dec	ah
	
	mov	dx, 0a468h
	in	al, dx
	push	dx
	push	ax
	
	or 	al, 00100000B
	out	dx, al
	
	mov	dx, 0a46ah
	mov	al, ah
	out	dx, al
	
	pop	ax
	pop	dx
	out	dx, al
	
	pop	ax
	pop	dx
	popf
	
	mov	[pcmover], 0
	
	;FIFOにデータ送信
	;バッファフルになるまで
	
	mov	es, [pcmoffset + 2]
	mov	bx, [pcmoffset]
	
	mov	dx, [pcmsize + 2]
	mov	cx, [pcmsize]
	
	mov	ax, TIME_PCMSIZE + 128
pcms_lp:
	push	ax
	;FIFOがフルか?
	push	dx
	mov	dx, 0a466h
	in	al, dx
	
	pop	dx
	test	al, 80h
	jnz	pcms_2	;FIFOフル
	
	cmp	[pcmover], 0
	jz	pcms_5
	push	dx
	mov	dx, 0a46ch
	mov	al, 0
	out	dx, al
	pop	dx
	
	inc	[pcm0send]	;データが無くなってから0を何回送ったか
	
	jmp	pcms_4
	
pcms_5:
	mov	al, es:[bx]
	push	dx
	mov	dx, 0a46ch
	out	dx, al
	pop	dx
	
	inc	bx
	jnz	pcms_3
	mov	ax, es
	add	ax, 1000h
	mov	es, ax
	
pcms_3:
	sub	cx, 1
	sbb	dx, 0
	
	mov	ax, cx
	or	ax, dx
	jnz	pcms_4
	
	mov	[pcmover], 1
	mov	[pcm0send], 0
	
pcms_4:	
	pop	ax
	dec	ax
	jz	pcms_ed
	jmp	pcms_lp
	
pcms_2:
	pop	ax
pcms_ed:

	mov	[pcmoffset + 2], es
	mov	[pcmoffset], bx
	
	mov	[pcmsize + 2], dx
	mov	[pcmsize], cx
	
	
	;ボリューム設定
	mov	dx, 0a466h
	mov	al, 0a0h
	out	dx, al
	
	
	;FIFO割り込みフラグリセット
	mov	dx, 0a468h
	in	al, dx
	
	;トリガOFF
	and	al, 11101111B
	out	dx, al
	
	;トリガON
	or	al, 00010000B
	out	dx, al
	
	
	;サウンドボードのほうの割り込み許可
	
	
	;割り込み許可
	or	al, 00100000B
	out	dx, al
	
	;FIFOスタート
	or	al, 10000000B
	out	dx, al
	
	;TimerA,TimerBフラグOFF
	;mov	bx, 273ch
	;call	opnwrt
	
	mov	[pcmflag], 0
	mov	[pcm_j], 0
	popf
	ret


f_pcm:
	;PCM再生スタート(演奏中からの呼び出し)
	cmp	[pcmplas], 0
	jnz	f_pcms_1
	
	;PCM機能がない
	jmp	f_pcm_end
	
f_pcms_1:
	cmp	[pcmflag], 0
	jnz	f_pcm__p1
	
	;ストップ
	push	dx
	push	ax
	call	c_pcmstop
	pop	ax
	pop	dx
	
f_pcm__p1:
	push	es
	
	push	ax
	push	bx
	push	cx
	push	dx
	push	di
	push	si
	
	
	mov	dx, es
	mov	bx, si
	
	mov	ax, es:[si]
	mov	cx, es:[si + 2]
	
	add	dx, cx
	add	bx, ax
	jnc	f_pcm_notsegov
	add	dx, 1000h

f_pcm_notsegov:
	
	mov	es, dx
	
	;入力
	;
	;	es:bx = データ先頭オフセット
	;	cx:dx = 再生サイズ
	;	bh = 再生周波数(port 0xa468の下位3bit)
	;	bl = パンポット/量子化ビット数(port 0xa46a)
	;
	pushf
	cli
	
	mov	si, bx
	mov	al, es:[si]
	mov	bh, al
	and	bh, 7
	mov	bl, al
	and	bl, 0f0h
	
	inc	si
	mov	cx, es:[si]
	add	si, 2
	mov	dx, es:[si]
	add	si, 2
	
	mov	[pcmoffset], si
	mov	[pcmoffset + 2], es
	
	mov	[pcmsize], cx
	mov	[pcmsize + 2], dx
	
	
	;FIFO割り込み禁止
	mov	dx, 0a468h
	in	al, dx
	
	and	al, 11011111B
	out	dx, al
	
	;FIFOリセット
	in	al, dx
	or	al, 00001000B
	out	dx, al
	
	and	al, 11110111B
	out	dx, al
	
	;CPU ->FIFO方向設定
	in	al, dx
	and	al, 10111111B
	out	dx, al
	
	;サンプリングレート
	and	al, 11111000B
	and	bh, 00000111B
	or	al, bh
	out	dx, al
	
	;量子化ビット数/パンポットをセット
	mov	dx, 0a46ah
	
	mov	al, bl
	and	al, 01110000B
	or	al, 00000010B
	out	dx, al
	
	
	;割り込みデータ長設定
	pushf
	push	dx
	push	ax
	cli
	mov	ax, TIME_PCMSIZE
	sal	ax, 1
	dec	ah
	
	mov	dx, 0a468h
	in	al, dx
	push	dx
	push	ax
	
	or 	al, 00100000B
	out	dx, al
	
	mov	dx, 0a46ah
	mov	al, ah
	out	dx, al
	
	pop	ax
	pop	dx
	out	dx, al
	
	pop	ax
	pop	dx
	popf
	
	mov	[pcmover], 0
	
	;FIFOにデータ送信
	;バッファフルになるまで
	
	mov	es, [pcmoffset + 2]
	mov	bx, [pcmoffset]
	
	mov	dx, [pcmsize + 2]
	mov	cx, [pcmsize]
	
	jmp	f_test	;@@@
	mov	ax, TIME_PCMSIZE + 128
	
f_pcms_lp:
	push	ax
	
	;FIFOがフルか?
	push	dx
	mov	dx, 0a466h
	in	al, dx
	
	pop	dx
	test	al, 80h
	jnz	f_pcms_2	;FIFOフル
	
	cmp	[pcmover], 0
	jz	f_pcms_5
	push	dx
	mov	dx, 0a46ch
	mov	al, 0
	out	dx, al
	pop	dx
	
	inc	[pcm0send]	;データが無くなってから0を何回送ったか
	
	jmp	f_pcms_4
	
f_pcms_5:
	mov	al, es:[bx]
	push	dx
	mov	dx, 0a46ch
	out	dx, al
	pop	dx
	
	inc	bx
	jnz	f_pcms_3
	mov	ax, es
	add	ax, 1000h
	mov	es, ax
	
f_pcms_3:
	sub	cx, 1
	sbb	dx, 0
	
	mov	ax, cx
	or	ax, dx
	jnz	f_pcms_4
	
	mov	[pcmover], 1
	mov	[pcm0send], 0
	
f_pcms_4:	
	pop	ax
	dec	ax
	jz	f_pcms_ed
	
	jmp	f_pcms_lp
	
f_pcms_2:
	pop	ax
	
f_pcms_ed:
f_test:

	mov	[pcmoffset + 2], es
	mov	[pcmoffset], bx
	
	mov	[pcmsize + 2], dx
	mov	[pcmsize], cx
	
	;ボリューム設定
	mov	dx, 0a466h
	mov	al, 0a0h
	out	dx, al
	
	;@@@
	IF 0
	
	;FIFO割り込みフラグリセット
	mov	dx, 0a468h
	in	al, dx
	
	;トリガOFF
	and	al, 11101111B
	out	dx, al
	
	;トリガON
	or	al, 00010000B
	out	dx, al
	
	;FIFOスタート
	or	al, 10000000B
	out	dx, al
	
	;割り込み許可
	or	al, 00100000B
	out	dx, al
	
	ENDIF
	
	mov	[pcm_j], 0
	mov	[pcmflag], 0
	popf
	

	pop	si
	pop	di
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	
	pop	es
	
f_pcm_end:

	add	si, 4
	
	mov al,[fmfrug]
	or al,al
	cmp al,0
	jz ffsub
	cmp al, 1
	jz psub
	jmp	rth_sub
	
	;
	;music start
	;		es:bx<---曲のアドレス先頭
mstart: ;曲が止まっていることが必要
	
	cmp	[bordf], 0
	jnz	mstart_f
	ret
mstart_f:
	cmp [pflag],0
	jnz mss1
	push es
	push bx
	call mend	;演奏中なら一旦止める
	pop bx
	pop es
	
mss1:	
	mov	[opnoffset], bx
	push bx
	;work clear
	mov bx,offset work
	mov cx,20h * 17 ;(FM6+SSG3+RTH6 + INT2)
	xor al,al
ml1:	mov [bx],al
	inc bx
	loop ml1
	
	;内部変数クリア
	
	mov	bx, offset var
	mov	cx, 100
	xor	al, al
ml__1:
	mov	[bx], al
	inc	bx
	loop	ml__1
	
	pop bx
	
	mov di,offset work
	
	rept 3
	call msput1
	add di,20h
	endm
	
	rept 3
	call msput2
	add di,20h
	endm
	
	rept 3
	call msput1
	add	di, 20h
	endm
	
	;RTH
	rept 6
	call msput1
	add	di, 20h
	endm
	
	;FM音源をデフォルトに設定
	mov	[ext86], 0
	
	mov	[nch], 0
	mov	di, offset play1
	call	sc_1
	
	mov	[nch], 1
	mov	di, offset play2
	call	sc_1
	
	mov	[nch], 2
	mov	di, offset play3
	call	sc_1
	
	mov	[ext86], 1
	
	mov	[nch], 0
	mov	di, offset play7
	call	sc_1
	
	mov	[nch], 1
	mov	di, offset play8
	call	sc_1
	
	mov	[nch], 2
	mov	di, offset play9
	call	sc_1
	
	mov	[ext86], 0
	
	
	



	
	;PSG音源をトーンモードに
	mov	di, offset play4
	I = 0
	rept	3
	local	s_pn1, s_pn2, s_pn3, s_pn4, s_pn5
	
	mov	al, 3eh
	mov	byte ptr [di + 10], al
	
	mov 	bh, al
	or 	bh, 11000000B
	mov 	al, I
	or 	al, al
	jz 	s_pn2
s_pn1:	stc
	rcl 	bh, 1
	dec 	al
	jnz 	s_pn1
s_pn2:	mov 	bl, 24h
	mov 	al, I
	or 	al, al
	jnz 	s_pn3
	mov 	bl, 9
	jmp 	s_pn5
s_pn3:	cmp 	al, 1
	jnz 	s_pn4
	mov 	bl, 12h
	jmp 	s_pn5
s_pn4:	
s_pn5:	mov 	al, [tn]
	or 	al, bl
	and 	al, bh
	mov 	[tn], al
	call	opnwrt
	
	I = I + 1
	add	di, 20h
	endm
	
	;PSGボリュームを15に
	mov	di, offset play4 + 9
	rept	3
	mov	byte ptr [di], 15
	add	di, 20h
	endm
	
	;RTHボリュームを63に
	mov	bh, 11h
	mov	bl, 63
	call	opnwrt
	
	
	
	call init
	xor al,al
	mov [pflag],al
	mov [timerbplay],0
	mov	[allstopflag], 0
	ret


se_store:
	;SEデータをストアする
	;	es:bx <-- SEデータ
	;	dx:cx <-- データサイズ
	
	add	cx, 15
	adc	dx, 0
	
	rept	4
	shr	dx, 1
	rcr	cx, 1
	endm
	
	cmp	cx, cs:[sebuffersize]
	jbe	se_store2
	
	;メモリが足りない
	pop bx		;return addr dummy
	pop ax		
	
	pop es
	pop ds
	pop si
	pop di
	pop dx
	pop cx
	pop bx
	
	mov	ax, 0ffffh
	iret
	
se_store2:
	push	di
	push	si
	
	push	ds
	push	es
	
	mov	si, bx
	mov	di, 0
	
	mov	ax, es
	mov	ds, ax
	
	mov	es, cs:[sebuffer]
	
se_store3:
	cld
	rept	16
	movsb
	endm
	loop	se_store3
	
	pop	es
	pop	ds
	pop	si
	pop	di
	
	pop bx		;return addr dummy
	pop ax		
	
	pop bx		;es dummy pop
	pop ds
	pop si
	pop di
	pop dx
	pop cx
	pop bx
	
	mov	cs:[se_storef], 1
	mov	ax, 0
	iret
	
se_start:
	;bl <- SE番号
	cmp	[se_storef], 0
	jz	se_start_end
	
	mov	al, bl
	
	mov	es, [sebuffer]
	mov	bx, 0
	
	;mov	ah, es:[bx + 5]
	;cmp	al, ah
	;jae	se_start_end
	
	add	bx, 6
	mov	ah, 0
	
	; ax = ax * 5 ( = ax * 2 * 2 + ax)
	mov	cx, ax
	sal	ax, 1
	sal	ax, 1
	add	ax, cx
	add	bx, ax
	
	call	sinton2

se_start_end:
	ret
	
	;
	;music start
	;		es:bx<---曲のアドレス先頭
	;		dx:cx<---曲データサイズ
r_mstart: ;曲が止まっていることが必要
	
	cmp	[bordf], 0
	jnz	r_mstart_f
	ret
r_mstart_f:
	cmp [pflag],0
	jnz r_mss1
	push es
	push bx
	push	cx
	push	dx
	call mend	;演奏中なら一旦止める
	pop	dx
	pop	cx
	pop bx
	pop es
	
r_mss1:	
	;メモリ確保
	add	cx, 15
	adc	dx, 0
	
	rept	4
	shr	dx, 1
	rcr	cx, 1
	endm
	
	cmp	cx, [fmbuffersize]
	jbe	r_mss2
	
	;メモリが足りない
	pop bx		;return addr dummy
	pop ax		
	
	pop es
	pop ds
	pop si
	pop di
	pop dx
	pop cx
	pop bx
	
	mov	ax, 0ffffh
	iret
	
r_mss2:
	push	di
	push	si
	
	push	ds
	push	es
	
	mov	si, bx
	mov	di, 0
	
	mov	ax, es
	mov	ds, ax
	
	mov	es, cs:[fmbuffer]
	
r_mss3:
	cld
	rept	16
	movsb
	endm
	loop	r_mss3
	
	pop	es
	pop	ds
	pop	si
	pop	di
	
	mov	es, [fmbuffer]
	mov	bx, 0
	
	
	call	mss1
	
	pop bx		;return addr dummy
	pop ax		
	
	pop bx		;es dummy pop
	pop ds
	pop si
	pop di
	pop dx
	pop cx
	pop bx
	
	mov	ax, 0
	iret

msput1:	mov ax,es:[bx]
	inc bx
	inc bx
	add	ax, [opnoffset]
	mov [di+19],ax
	mov byte ptr [di + 14], 63

	ret
msput2: mov ax,es:[bx]
	inc bx
	inc bx
	add	ax, [opnoffset]
	mov [di+29],ax
	ret
	
sc_1:	
	push	si
	
	mov	si, offset default_fm
	
	;Sound change
	
	;Ver 1.04変更点
	
	;sl/rr を　0xff にする
	mov bh,[nch]
	add bh,80h
	mov bl,0ffh
	call opnwrt
	add bh,4
	call opnwrt
	add bh,4
	call opnwrt
	add bh,4
	call opnwrt
	
	;変更ここまで
	
	mov bl,[nch]
	mov bh,28h
	call opnwrt



	mov bh,30h
	mov al,[nch]
	add al,bh
	mov bh,al
	
	rept 4
	
	mov bl,[si]
	call opnwrt
	inc si
	inc bh
	inc bh
	inc bh
	inc bh
	
	endm
	
	mov bl,[si]
	call opnwrt
	mov [di+14],bl
	inc si
	inc bh
	inc bh
	inc bh
	inc bh
	
	mov bl,[si]
	call opnwrt
	mov [di+16],bl
	inc si
	inc bh
	inc bh
	inc bh
	inc bh
	
	mov bl,[si]
	call opnwrt
	mov [di+15],bl
	inc si
	inc bh
	inc bh
	inc bh
	inc bh
	
	mov bl,[si]
	call opnwrt
	mov [di+17],bl
	inc si
	inc bh
	inc bh
	inc bh
	inc bh
	
	rept 20
	mov bl,[si]
	call opnwrt
	inc si
	inc bh
	inc bh
	inc bh
	inc bh
	endm
	
	mov al,10h
	add al,bh
	mov bh,al
	mov bl,[si]
	call opnwrt
	mov al,bl
	and al,7
	mov [di+13],al
	
	;mov bh,28h
	;mov al,[nch]
	;mov bl,al
	;call opnwrt
	
	mov byte ptr [di+1],0
	
	;悪いが、パッチ当てで逃げるぞーーーー
	;mov al,0a4h
	;add al,[nch]
	;mov bh,al
	;mov bl,0
	;call opnwrt
	;
	;sub bh,4
	;call opnwrt
	
	pop	si
	ret
	
	
	
	
	
	
	;music stop
	;		強制ストップ

mstop:	
	cmp	[bordf], 0
	jnz	mstop_f
	ret
mstop_f:
	mov al,1
	mov [pflag],al
	mov bx,2700h
	call opnwrt
	
	ret
	
	;music pause
	
mescon: 
	cmp [bordf], 0
	jnz	mescon_f
	ret
mescon_f:
	cmp [musicp],0
	jz mesc1
	;既にポーズがかかっている
	ret
mesc1:
	mov al,1
	mov [pflag],al
	
	mov al,[s3work]	;ret code
	mov ah,byte ptr [s3]
	mov byte ptr [s3],al
	mov [s3work],ah
	
	mov bx,407fh
	
	rept 15
	call opnwrt
	inc bh
	endm
	
	mov bx,73fh
	call opnwrt
	
	mov [musicp],1
	
	ret
	
	;music restart
	
mescof: 
	cmp [bordf], 0
	jnz	mescof_f
	ret
mescof_f:

	cmp [musicp],0
	jnz meso1
	;ポーズがかかっていないのに呼んだ
	ret
meso1:
	mov di,offset play1
	mov bh,40h
	call volc
	mov di,offset play2
	mov bh,41h
	call volc
	mov di,offset play3
	mov bh,42h
	call volc
	
	mov di,offset play7
	mov bh,40h
	call volc
	mov di,offset play8
	mov bh,41h
	call volc
	mov di,offset play9
	mov bh,42h
	call volc
	
	xor al,al
	mov [pflag],al
	mov al,[s3work]	;ret code
	mov ah,byte ptr [s3]
	mov byte ptr [s3],al
	mov [s3work],ah
	
	mov [musicp],0
	ret
	
volc:	mov bl,[di+14]
	call opnwrt
	inc bh
	inc bh
	inc bh
	inc bh
	mov bl,[di+16]
	call opnwrt
	inc bh
	inc bh
	inc bh
	inc bh
	mov bl,[di+15]
	call opnwrt
	inc bh
	inc bh
	inc bh
	inc bh
	mov bl,[di+17]
	call opnwrt
	ret
	;
	
	;tn set
	;	ジョイポートのＡ、Ｂの入出力を決める
	;
	;	引数：ＣＬ；アドレス７のデータ（上位２ｂｉｔのみ有効）

tnset:	
	mov al,[tn]
	and al,00111111B
	and cl,11000000B
	or al,cl
	mov [tn],al
	
	ret
	
	
	
	;init start
	;		<--cx;割り込みデータ格納アドレス
	;		<--dl;0---ch2 1---ch3
minton: 
	cmp [bordf], 0
	jnz	minton_f
	ret
minton_f:
	mov di,offset int1
	mov bx,offset play5+2
	or dl,dl
	jz m1
	mov di,offset int2
	mov bx,offset play6+2
m1:	mov al,1
	mov [bx],al
	mov byte ptr [di+0],0
	mov [di+29],cx
	ret

minton2: 
	cmp	[bordf], 0
	jnz	minton2_f
	ret
minton2_f:
	;mintonのesセグメントバージョン
	;esセグメントを設定する
	mov	[seseg], es
	mov di,offset int1
	mov bx,offset play5+2
	or dl,dl
	jz mm_m1
	mov di,offset int2
	mov bx,offset play6+2
mm_m1:	mov al,1
	mov [bx],al
	mov byte ptr [di+0],0
	mov [di+29],cx
	ret

midiread	PROC	NEAR
		;MIDIの演奏状況を返す
		;
		;戻り値:	al <- 演奏状況 0-play 1-stop
		;		ah <- fade情報 (0 - 128)
		mov	ax, cs
		mov	es, ax

		mov	al, es:[stop]
		mov	ah, byte ptr es:[velo]
		
		;直接ここから戻ってしまう
		;ちょっときたないけど
		
		pop bx		;return addr dummy
		pop bx		;ax dummy pop 
		
		pop es
		pop ds
		pop si
		pop di
		pop dx
		pop cx
		pop bx
		
		iret

midiread	ENDP

fmstat:
	;ボード状態などステータス読み出し
	;
	;
	;	戻り値: al
	;
	;	ビット0: FM音源再生 (0:再生中,1:停止中)
	;	ビット1: PCM音源再生(0:再生中,1:停止中)
	;	ビット4: ポート番号が(0:188h, 1:288h)
	;	ビット5: PCM音源が(0:存在しない,1:存在する)
	;	ビット6: YM2608が(0:存在しない,1:存在する)
	;	ビット7: FM音源ボードが(0:存在しない,1:存在する)
	
	mov	al, 0
	or	al, [bordf]
	sal	al, 1
	or	al, [bord86]
	sal	al, 1
	or	al, [pcmplas]
	sal	al, 1
	
	cmp	[ioport1], 188h
	jz	fm_188
	or	al, 1
	
fm_188:
	sal	al, 1
	
	sal	al, 1
	sal	al, 1
	or	al, [pcmflag]
	sal	al, 1
	or	al, [pflag]
	
	pop bx		;return addr dummy
	pop bx		;ax dummy pop 
	
	pop es
	pop ds
	pop si
	pop di
	pop dx
	pop cx
	pop bx
	
	iret

addrread:
	;read FM音源アドレス
	;
	;	dl:アドレス番号
	;	dh:ポート(0:188h, 1:18ah)
	
	;	戻り値: al
	
	mov	di, offset buf188
	cmp	dh, 0
	jnz	p_188
	mov	di, offset buf18a
p_188:
	mov	al, dl
	mov	ah, 0
	add	di, ax
	mov	al, [di]
	
	
	pop bx		;return addr dummy
	pop bx		;ax dummy pop 
	
	pop es
	pop ds
	pop si
	pop di
	pop dx
	pop cx
	pop bx
	
	iret
	
	
varread:
	;内部変数読み込み
	;dl <- 変数番号(0～99)
	mov	di, offset var
	mov	dh, 0
	add	di, dx
	mov	al, [di]
	
	pop bx		;return addr dummy
	pop bx		;ax dummy pop 
	
	pop es
	pop ds
	pop si
	pop di
	pop dx
	pop cx
	pop bx
	
	iret
	
	
	;read status
	;		dl<--チャンネル番号(1-11)
	;		dh<--初めから何番目のワークを読むか
	;		ah<--返す値
	
mread:	mov di,offset work-20h
	mov cl,dl
	mov ch,0
	
mrlp:	add di,20h
	loop mrlp
	
	xchg dl,dh
	mov dh,0
	
	add di,dx
	mov ah,[di]
	
	pop bx		;return addr dummy
	pop bx		;ax dummy pop 
	
	pop es
	pop ds
	pop si
	pop di
	pop dx
	pop cx
	pop bx
	
	iret
	
i80seg	dw ?
i80addr dw ?

sysf	dw ?
sax	dw ?

	;system
	;	ベクターを戻す
	;	割り込みを戻す
system:	
	
	call	midistop
	call	closemidi
	call	mend
	call	pcmstop
	
	mov	ax, cs
	mov	ds, ax
	mov	es, ax
	
	cmp	[bordf], 0
	jz	system_e

	cmp	[vectsetflag], 0
	jz	notsetsoundvect
	
	test [musicvect],8
	jnz evect8_15
	;vect 0-7
	mov al,[invmask]
	out 2,al
	jmp evectnoed
	
evect8_15:
	;vect 8-15
	mov al,[invmask]
	out 0ah,al
evectnoed:
	
system_e:
	;サウンドベクタを戻す
	
	push ds
	mov al,[musicvect]
	add al,8	
	mov dx,[vecsoundoff]
	mov ds,[vecsoundseg]
	mov ah,25h
	int 21h
	pop ds
	
notsetsoundvect:
	
	;制御ベクタを戻す
	
	push ds
	mov dx,[i80addr]
	mov ds,[i80seg]
	mov cs:[sysf],ds
	mov ah,25h
	mov al,vect
	int 21h
	
	pop ds
	
	
	;バッファ解放
	push	es
	mov	ah, 49h
	mov	es, [fmbuffer]
	int	21h
	pop	es
	jnc	rr_1
	mov	ah, 9
	mov	dx, offset msgfree
	int	21h
	mov	ax, 4cfeh
	int	21h
rr_1:
	push	es
	mov	ah, 49h
	mov	es, [midibuffer]
	int	21h
	pop	es
	jnc	rr_2
	mov	ah, 9
	mov	dx, offset msgfree
	int	21h
	mov	ax, 4cfeh
	int	21h

rr_2:
	push	es
	mov	ah, 49h
	mov	es, [pcmbuffer]
	int	21h
	pop	es
	jnc	rr_3
	mov	ah, 9
	mov	dx, offset msgfree
	int	21h
	mov	ax, 4cfeh
	int	21h
	
rr_3:
	
	push	es
	mov	ah, 49h
	mov	es, [sebuffer]
	int	21h
	pop	es
	jnc	rr_4
	mov	ah, 9
	mov	dx, offset msgfree
	int	21h
	mov	ax, 4cfeh
	int	21h
	
rr_4:
	
	push	cs
	pop	es
	
	
	pop bx		;return addr dummy
	pop ax		
	
	pop bx		;es dummy pop
	pop ds
	pop si
	pop di
	pop dx
	pop cx
	pop bx
	
	iret
	
	;

VERS	db ver
ID	db 'OPNDRV'
	
ent80:	;制御ベクタ　エントリ
	;
	;		ah<--機能番号
	
	push bx
	push cx
	push dx
	push di
	push si
	push ds
	push es
	push ax
	
	push cs
	pop ds
	
	cld
	
	mov [sysf],es
	
ent80a:	
	push bx		;これを忘れたために３日悩んだ
	
	xchg ah,al
	xor ah,ah
	sal ax,1
	mov bx,offset rnum
	add bx,ax
	
	mov ax,[bx]
	pop bx
	
	call ax
	
entret:	
	pop ax
	pop es
	pop ds
	pop si
	pop di
	pop dx
	pop cx
	pop bx
	
	iret
	
rnum	dw offset mstart
	dw offset mend
	dw offset se_start
	dw offset mescon
	dw offset mescof
	dw offset minton
	dw offset mfout
	dw offset system
	dw offset mread
	dw offset sstart
	dw offset send
	dw offset sinton
	dw offset tnset
	dw offset	minton2	;Ver1.11で追加 esセグメント設定あり
	dw offset	sinton2 ;Ver1.11で追加 esセグメント設定あり
	dw	offset tempochange
	dw	offset addrread
	dw	offset fmstat
	dw	offset r_mstart
	dw	offset se_store
	
	;MIDI + 
	;機能コード20から
	dw	offset midi		;midi init
	dw	offset closemidi	;midi close
	dw	offset midiplay		;play
	dw	offset midistop		;stop
	dw	offset midicont		;cotinue
	dw	offset midifade		;fade
	dw	offset midiread		;midi status
	dw	offset miditem		;tempo補正
	dw	offset midicall		;midiサブルーチンコール
	dw	offset midiextend	;midi jump extender on!
	dw	offset midisend		;midi command send
	dw	offset r_midiplay
	dw	?
	dw	?
	dw	?
	dw	?
	dw	?
	dw	?
	dw	?
	dw	?
	
	;PCM +
	;機能コード 40から
	dw	offset pcmstart
	dw	offset pcmstop
	dw	offset r_pcmstart
	dw	?
	dw	?
	dw	?
	dw	?
	dw	?
	dw	?
	dw	?
	
	;拡張
	dw	offset varread	;内部変数読み込み
	
progend:

start:	;プログラムスタート
	;int セット
	;
	;	int num ==vect
	push cs
	pop ds
	;リリースオプションチェック
	mov cl,byte ptr cs:[80h]
	cmp cl,0
	jz st2
	xor ch,ch
	mov bx,81h
stlp:	;オプション解析
	
	;スペースをカット
	cmp byte ptr [bx],' '
	jz ste
	cmp byte ptr [bx],9	;tab code
	jz ste
	cmp byte ptr [bx],13
	jz ste
	cmp byte ptr [bx],'-'
	jz	opt_all
	cmp byte ptr [bx],'/'
	jnz parae
	
opt_all:
	; /k  カナキーによる音の抑制
	cmp byte ptr [bx+1],'k'
	jz opt_k
	cmp byte ptr [bx+1],'K'
	jz opt_k
	; /c　ＣＡＰＳキーによる音の抑制
	cmp byte ptr [bx+1],'c'
	jz opt_c
	cmp byte ptr [bx+1],'C'
	jz opt_c
	; /r  開放オプション
	cmp byte ptr [bx+1],'R'
	jz opt_r
	cmp byte ptr [bx+1],'r'
	jz opt_r
	; /?  オプション
	cmp byte ptr [bx+1],'?'
	jz	helpmes
	cmp byte ptr [bx+1],'h'
	jz	helpmes
	cmp byte ptr [bx+1],'H'
	jz	helpmes
	; /D オプション
	cmp	byte ptr [bx + 1], 'd'
	jz	opt_d
	cmp	byte ptr [bx + 1], 'D'
	jz	opt_d
	
	; /X  オプション
	cmp	byte ptr [bx + 1], 'f'
	jz	opt_f
	cmp	byte ptr [bx + 1], 'F'
	jz	opt_f
	
	cmp	byte ptr [bx + 1], 'm'
	jz	opt_m
	cmp	byte ptr [bx + 1], 'M'
	jz	opt_m
	
	cmp	byte ptr [bx + 1], 'p'
	jz	opt_p
	cmp	byte ptr [bx + 1], 'P'
	jz	opt_p
	
	cmp	byte ptr [bx + 1], 's'
	jz	opt_s
	cmp	byte ptr [bx + 1], 'S'
	jz	opt_s
	
	jmp parae
	
opt_f:
	;FM音源バッファサイズ
	inc	bx
	inc	bx
	dec	cx
	dec	cx
	call	getval
	;ここでaxに1/16された値が入ってくる
	mov	cs:[fmbuffersize], ax
	
	jmp	ste
opt_s:
	;SEバッファサイズ
	inc	bx
	inc	bx
	dec	cx
	dec	cx
	call	getval
	;ここでaxに1/16された値が入ってくる
	mov	cs:[sebuffersize], ax
	
	jmp	ste
	
opt_m:
	;MIDI音源バッファサイズ
	inc	bx
	inc	bx
	dec	cx
	dec	cx
	call	getval
	;ここでaxに1/16された値が入ってくる
	mov	cs:[midibuffersize], ax
	
	jmp	ste
	
opt_p:
	;PCM音源バッファサイズ
	inc	bx
	inc	bx
	dec	cx
	dec	cx
	call	getval
	;ここでaxに1/16された値が入ってくる
	mov	cs:[pcmbuffersize], ax
	
	jmp	ste
	
getval:
	;[bx]より数値を取り出す
	;値はパラグラフでaxに返す
	
	mov	ax, 0
getval_lp:
	cmp	byte ptr [bx], '0'
	jb	getval_ret
	cmp	byte ptr [bx], '9'
	ja	getval_ret
	
	;axを10倍( ax = (ax * 2 * 2 + ax) * 2)
	push	dx
	mov	dx, ax
	sal	ax, 1
	sal	ax, 1
	add	ax, dx
	sal	ax, 1
	pop	dx
	
	add	al, byte ptr [bx]
	adc	ah, 0
	sub	ax, '0'
	inc	bx
	dec	cx
	jmp	getval_lp
	
getval_ret:
	;ax = ax * 1024(KB単位に) / 16(パラグラフ単位に)
	;   = ax * 64(=2^6)
	rept	6
	sal	ax, 1
	jc	getval_err
	endm
	
	dec	bx
	inc	cx
	ret
	
getval_err:
	mov	ah, 9
	mov	dx, offset msgerrbuf
	int	21h
	mov	ax, 4cfeh
	int	21h
	
opt_d:
	mov	[debug], 1
	inc	bx
	dec	cx
	jmp	ste
	
opt_c:  or [ston],2
	inc bx
	dec cx
	jmp ste
opt_k:  or [ston],4
	inc bx
	dec cx
	jmp ste
	
opt_r:
	mov [rrf],1
	inc bx
	dec cx
	
ste:
	inc bx
	loop stlp
	
st2:
	mov ah,35h
	mov al,vect
	int 21h
	;既に常駐しているかチェック
	;初めの50ｂｙｔｅでいいじゃないでしょうか？
	cld
	mov di,bx
	mov si,offset ent80
	mov cx,50
	repe cmpsb
	jne start1
	;既に常駐している
	cmp [rrf],0
	jnz release
	
	mov dx,offset msgq
	mov ah,09h
	int 21h
	
	mov ah,4ch
	mov al,[bordf]
	or al,080h
	int 21h		;常駐せずに終了
	
release:
	;開放する
	
	;音をとめる
	cmp	[bordf], 0
	jz	notkillpcm
	mov ah,1
	int vect
	;拡張機能が有効ならば、それを無効にする
	cmp	[bord86], 0
	jz	notkill86
	mov	bl, 0
	mov	bh, 29h
	call	opnwrt
notkill86:
	;PCM機能が有効ならば無効にする
	cmp	[pcmplas], 0
	jz	notkillpcm
	mov	dx, 0a460h
	mov	al, 0
	out	dx, al
notkillpcm:

	
	mov ah,7	;ワークエリア戻し
	int vect
	;
	;ここで、ｅｓに常駐セグメントが返ってくる
	;
	
	
	push es
	
	mov es,es:[002ch]	;環境文字列の削除
	
	
	mov ax,4900h
	int 21h
	
	pop es
	
	mov ax,4900h
	int 21h
	
	mov dx,offset msg1
	mov ah,09h
	int 21h
	
	sti
	mov ah,4ch
	mov al,0ffh	;開放コード
	int 21h		;常駐せずに終了
	
start1:	
	cmp [rrf],0
	jz sty1
	mov dx,offset msge1
	mov ah,09h
	int 21h
	mov ah,4ch
	mov al,0fdh
	int 21h
	
sty1:
	mov [i80seg],es
	mov [i80addr],bx
	mov ah,25h
	mov al,vect
	mov dx,offset ent80
	int 21h
	
	;メッセージを表示
	mov dx,offset msg2
	mov ah,09h
	int 21h
	
	;86ボードが付いているか?
	
	mov	[bord86], 0
	mov	[pcmplas], 0
	
	cmp	[debug], 0
	jnz	not86
	
	mov	dx, 0a460h
	in	al, dx
	cmp	al, 0ffh
	jz	not86
	;86ボードらしきものがついているようだ
	
	;拡張機能ON
	mov	al, 1
	out	dx, al
	
	;メッセージ表示
	mov	dx, offset msgpcm
	mov	ah, 9
	int	21h
	
	mov	[bord86], 1
	mov	[pcmplas], 1
	
	
	shr	ax, 1
	shr	ax, 1
	shr	ax, 1
	shr	ax, 1
	
	cmp	ax, 3
	jnz	notp2xx
	cmp	ax, 5
	jnz	notp2xx
p2xx:	
	;ポートが288/28a/28c/28e
	mov	[ioport1], 288h
	mov	[ioport2], 28ah
	mov	[ioport3], 28ch
	mov	[ioport4], 28eh
	
	jmp	fm46on
	
notp2xx:
	;ポートが188/18a/18c/18e
	mov	[ioport1], 188h
	mov	[ioport2], 18ah
	mov	[ioport3], 18ch
	mov	[ioport4], 18eh
	
fm46on:
	;拡張部分使用可能にする
	mov	bh, 29h
	mov	bl, 83h
	call	opnwrt
	
	;リズムボリュームを最大にする
	mov	bh, 11h
	mov	bl, 63
	call	opnwrt
	
not86:
	;サウンドボードが付いているかのチェック
	cli
	mov dx,[ioport1]
	mov al,6
	out dx,al
	jmp $+2
	jmp $+2
	jmp $+2
	jmp $+2
	mov dx,[ioport2]
	out dx,al
	jmp $+2
	jmp $+2
	jmp $+2
	jmp $+2
	jmp $+2
	in al,dx
	sti
	cmp al,6
	jz slp3
	;ＰＳＧに関してはＷＡＩＴ不要
	;付いていない
	mov [bordf],0
	mov dx,offset msg3
	mov ah,09h
	int 21h
	jmp slp4
slp3:	mov [bordf],1
	
	
	;YM2608が付いているかどうか
	cmp	[debug], 0
	jnz	not__86
	
	mov	dx, [ioport1]
	mov	al, 0ffh
	out	dx, al
	
	;ウェイト
	;2.1μs
	rept	4
	out	5fh, al
	endm
	
	mov	dx, [ioport2]
	in	al, dx
	cmp	al, 1
	jnz	not__86
	
	;YM2608がある
	
	mov	[bord86], 1
	
	;メッセージ表示
	mov	dx, offset msg86
	mov	ah, 9
	int	21h
	
not__86:

	;ボードはついている
	;サウンド割り込みベクタの入手
	cli
	mov dx,[ioport1]
	mov al,0eh
	out dx,al
	mov dx,[ioport2]
	jmp $+2
	jmp $+2
	jmp $+2
	jmp $+2
	in al,dx
	sti
	and al,11000000B	;上位２ＢＩＴのみ
	cmp al,01000000B
	jnz vc1
	mov [musicvect],13
	mov [maskf],00100000B
	jmp vcted
vc1:	cmp al,11000000B
	jnz vc2
	mov [musicvect],12
	mov [maskf],00010000B
	jmp vcted
vc2:	cmp al,10000000B
	jnz vc3
	mov [musicvect],10
	mov [maskf],00000100B
	jmp vcted
vc3:	;cmp al,00000000B
	;jnz vc4
	mov [musicvect],3
	mov [maskf],00001000B
vcted:
	mov dx,[ioport1]
	mov al,7
	out dx,al
	jmp $+2
	jmp $+2
	jmp $+2
	jmp $+2
	mov dx,[ioport2]
	in al,dx
	or al,00111111B
	mov [tn],al
slp4:	
	;キーププロセス
	
	;バッファ用メモリの確保
	
	;メモリの切り詰め
	
	mov	ax, cs
	mov	es, ax
	mov	bx, offset progendaddr
	add	bx, 200h
	mov	sp, bx
	
	add	bx, 15
	
	shr	bx, 1
	shr	bx, 1
	shr	bx, 1
	shr	bx, 1
	
	mov	ah, 4ah
	int	21h
	jnc	slp4_ok
	;メモリ縮小失敗
	mov	dx, offset msgcut
	mov	ah, 9
	int	21h
	mov	ax, 4cfeh
	int	21h
	
slp4_ok:
	
	;メモリ割り当て
	mov	ah, 48h
	mov	bx, [fmbuffersize]
	int	21h
	jnc	slp4_m1
	;メモリ割り当て失敗
	mov	dx, offset msgalloc
	mov	ah, 9
	int	21h
	mov	ax, 4cfeh
	int	21h
slp4_m1:
	mov	[fmbuffer], ax
	
	mov	ah, 48h
	mov	bx, [midibuffersize]
	int	21h
	jnc	slp4_m2
	;メモリ割り当て失敗
	mov	dx, offset msgalloc
	mov	ah, 9
	int	21h
	mov	ax, 4cfeh
	int	21h
slp4_m2:
	mov	[midibuffer], ax
	
	mov	ah, 48h
	mov	bx, [pcmbuffersize]
	int	21h
	jnc	slp4_m3
	;メモリ割り当て失敗
	mov	dx, offset msgalloc
	mov	ah, 9
	int	21h
	mov	ax, 4cfeh
	int	21h
slp4_m3:
	mov	[pcmbuffer], ax
	
	mov	ah, 48h
	mov	bx, [sebuffersize]
	int	21h
	jnc	slp4_m4
	;メモリ割り当て失敗
	mov	dx, offset msgalloc
	mov	ah, 9
	int	21h
	mov	ax, 4cfeh
	int	21h
slp4_m4:
	mov	[sebuffer], ax
	
slp4_me:

	mov ah,31h
	mov al,0
	mov dx,offset cs:start
	;
	shr dx,1
	shr dx,1
	shr dx,1
	shr dx,1
	;
	inc dx
	mov al,[bordf]	;常駐終了コード
	
	int 21h

parae:	;パラメーターエラー
	mov dx,offset msge
	mov ah,09h
	int 21h
	
	mov ax,4cfeh
	int 21h

helpmes:
	;ヘルプメッセージの表示
	mov	dx, offset hmes
	mov	ah, 9
	int	21h
	mov	ax, 4cfch
	int	21h
	
rrf	db 0			;リリースフラグ

	
hmes	db 'ＯＰＮＤＲＶ '
	db 1bh, '[s', 1bh, '[33m'
	db 'MIDI+'
	db 1bh, '[u', 1bh, '[5C'
	db '   with  expander & Psg voice speaking system  Ver ',ver,13,10
	db '		               Original programmed  by T.Horikawa(PC-8801)',13,10
	db '		               reprogrammed & MIDI+ by F.Osawa 　(PC-9801)',13,10
	db 13,10
	db 'Options:',13,10
	IF 0
	db '        /k              カナキーによる音の抑制をon',13,10
	db '        /c              CAPSキーによる音の抑制をon',13,10
	ENDIF
	
	db '        /fxx            FM音源用バッファをxx(KB)にする(Default 0)', 13, 10
	db '        /mxx            MIDI音源用バッファをxx(KB)にする(Default 0)', 13, 10
	db '        /pxx            PCM音源用バッファをxx(KB)にする(Default 0)', 13, 10
	db '        /sxx            SE用バッファをxx(KB)にする(Default 0)', 13, 10
	
	db '        /h              このメッセージを表示',13,10
	db '        /r              常駐解除',13,10
	db '$'
	
msg1	db 'ＯＰＮＤＲＶを開放しました',13,10,'$'
msg2	db 'ＯＰＮＤＲＶ '
	db 1bh,'[s',1bh,'[33m'	;黄色で表示
	db 'MIDI+'
	db 1bh,'[u',1bh,'[5C'	;元に戻す
	db '   with  expander & PSG voice speaking system  Ver ',ver,13,10
	db '		               Original programmed  by T.Horikawa(PC-8801)',13,10
	db '		               reprogrammed & MIDI+ by F.Osawa 　(PC-9801)',13,10
	db 'システム常駐しました。ベクタ'
	IF ((vect/16) LT 10)
		db (vect/16)+30h
		ELSE
		db (vect/16)-10+'A'
		ENDIF
	IF ((vect MOD 16) LT 10)
		db (vect MOD 16)+30h
		ELSE
		db (vect MOD 16)-10+'A'
		ENDIF
	db 'H を使用します。',13,10,'$'
msg3	db 'このシステムにはサウンドボードは装備されていません。',13,10,'$'

msgq	db 'ＯＰＮＤＲＶはすでに常駐しています',13,10,'$'
msge	db 'パラメータエラーです(/? でヘルプを表示)',13,10,'$'
msge1	db 'ＯＰＮＤＲＶは常駐していません',13,10,'$'
msgpcm	db 'PCM機能が使用可能です', 13, 10, '$'
msg86	db 'YM2608拡張機能が使用可能です', 13, 10, '$'
msgalloc	db 'サウンドバッファ用メモリが確保できません', 13, 10, '$'
msgcut	db 'メモリブロックの縮小に失敗しました', 13, 10, '$'
msgfree	db 'サウンドバッファのメモリ解放に失敗しました', 13, 10, '$'
msgerrbuf	db 'バッファサイズが大きすぎます', 13, 10, '$'
progendaddr	LABEL UNKNOWN
	end progstart
	
	
	