/* p2: void __p2_call0(void *f): calls f(). And a data word C and wolf share. */
	.text
	.globl	__p2_call0
	.type	__p2_call0,@function
__p2_call0:
	jmp	*%rdi
	.size	__p2_call0, .-__p2_call0
	.data
	.balign	8
	.globl	__p2_env
	.type	__p2_env,@object
	.size	__p2_env, 8
__p2_env:
	.quad	0
	.section .note.GNU-stack,"",@progbits
