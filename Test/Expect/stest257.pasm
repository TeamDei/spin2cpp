pub main
  coginit(0, @entry, 0)
dat
	org	0
entry

_fetch1
	rdlong	result1, arg01
	add	arg01, #4
	rdlong	_var03, arg01
	mov	_var02, _var03
	mov	result2, _var02
_fetch1_ret
	ret

_fetch2
	rdlong	result1, arg01
	add	arg01, #4
	rdlong	_var03, arg01
	mov	_var02, _var03
	mov	result2, _var02
_fetch2_ret
	ret

result1
	long	0
result2
	long	0
COG_BSS_START
	fit	496
	org	COG_BSS_START
_var01
	res	1
_var02
	res	1
_var03
	res	1
arg01
	res	1
	fit	496
