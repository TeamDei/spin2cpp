DAT
	org	0

sum2
	rdlong	result_, arg1_
	add	arg1_, #4
	rdlong	sum2_tmp003_, arg1_
	add	result_, sum2_tmp003_
sum2_ret
	ret

arg1_
	long	0
arg2_
	long	0
arg3_
	long	0
result_
	long	0
sum2_tmp003_
	long	0
