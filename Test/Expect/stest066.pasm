PUB main
  coginit(0, @entry, 0)
DAT
	org	0
entry

_set
	add	objptr, #8
	mov	_var_02, objptr
	sub	objptr, #4
	rdlong	_tmp002_, objptr
	sub	objptr, #4
	add	_var_02, _tmp002_
	wrbyte	arg1, _var_02
_set_ret
	ret

objptr
	long	@@@objmem
result1
	long	0
COG_BSS_START
	fit	496
objmem
	res	3
	org	COG_BSS_START
_tmp002_
	res	1
_var_02
	res	1
arg1
	res	1
arg2
	res	1
arg3
	res	1
arg4
	res	1
	fit	496
