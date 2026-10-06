	.text
	.globl	q_syscall3
	.type	q_syscall3,@function
q_syscall3:
	mov	%rdi, %rax
	mov	%rsi, %rdi
	mov	%rdx, %rsi
	mov	%rcx, %rdx
	syscall
	ret
	.size	q_syscall3, .-q_syscall3
	.section .note.GNU-stack,"",@progbits
