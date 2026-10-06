	.text
	.globl	_start
_start:
	xor	%rbp, %rbp
	mov	(%rsp), %rdi
	lea	8(%rsp), %rsi
	lea	8(%rsi,%rdi,8), %rdx
	and	$-16, %rsp
	call	main
	mov	%eax, %edi
	call	exit
	hlt
	.section .note.GNU-stack,"",@progbits
