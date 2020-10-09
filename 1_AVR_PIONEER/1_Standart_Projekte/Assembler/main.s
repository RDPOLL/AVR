	.file	"main.c"
__SREG__ = 0x3f
__SP_H__ = 0x3e
__SP_L__ = 0x3d
__CCP__  = 0x34
__tmp_reg__ = 0
__zero_reg__ = 1
	.text
.global	main
	.type	main, @function
main:
/* prologue: function */
/* frame size = 0 */
	ldi r24,lo8(-1)
	out 36-32,r24
	out 42-32,__zero_reg__
.L2:
	in r24,41-32
	out 37-32,r24
	rjmp .L2
	.size	main, .-main
