pub main
  coginit(0, @entry, 0)
dat
	org	0
entry

_blah
	mov	result1, #3
_blah_ret
	ret

_getbar
	add	arg01, ptr__dat__
	rdbyte	result1, arg01
_getbar_ret
	ret

ptr__dat__
	long	@@@_dat_
result1
	long	0
COG_BSS_START
	fit	496
	long
_dat_
	byte	$01, $02, $03, $00
	org	COG_BSS_START
arg01
	res	1
	fit	496
