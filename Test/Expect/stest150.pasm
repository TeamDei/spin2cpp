pub main
  coginit(0, @entry, 0)
dat
	org	0
entry

_foo
	mov	result1, #0
_foo_ret
	ret

result1
	long	0
COG_BSS_START
	fit	496
	org	COG_BSS_START
	fit	496
