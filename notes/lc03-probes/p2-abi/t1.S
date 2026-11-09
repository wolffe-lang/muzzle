/* p2: int __p2_call2(void *f, const void *a, const void *b): calls f(a, b). */
	.text
	.globl	__p2_call2
	.type	__p2_call2,@function
__p2_call2:
	mov	%rdi, %rax
	mov	%rsi, %rdi
	mov	%rdx, %rsi
	jmp	*%rax
	.size	__p2_call2, .-__p2_call2
	.section .note.GNU-stack,"",@progbits
