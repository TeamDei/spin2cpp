pub main
  coginit(0, @entry, 0)
dat
	org	0
entry

_test1
	mov	_var01, arg01 wz
 if_ne	jmp	#LR__0001
	cmp	arg02, #0 wz
 if_e	jmp	#LR__0002
LR__0001
	mov	result1, arg03
	jmp	#_test1_ret
LR__0002
	neg	result1, #1
_test1_ret
	ret

__lockreg
	long	0
ptr___lockreg_
	long	@@@__lockreg
result1
	long	0
COG_BSS_START
	fit	496
	org	COG_BSS_START
_var01
	res	1
arg01
	res	1
arg02
	res	1
arg03
	res	1
	fit	496
