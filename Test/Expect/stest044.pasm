DAT
	org	0

_strlen
	neg	_strlen_r, #1
L_025_
	rdbyte	_strlen_c, arg1_ wz
	add	arg1_, #1
	add	_strlen_r, #1
 if_ne	jmp	#L_025_
	mov	result_, _strlen_r
_strlen_ret
	ret

_strlen_c
	long	0
_strlen_r
	long	0
arg1_
	long	0
arg2_
	long	0
arg3_
	long	0
result_
	long	0
