/* muzzle: _start, the process entry (crt0.o; not part of libmuzzle.a).
 *
 * Written from the x86-64 psABI 3.4.1 (the initial process stack: argc
 * at 0(%rsp), then argv[0..argc-1], a null, envp[...], a null, auxv),
 * which also fixes %rsp 16-byte aligned at entry and %rbp unspecified,
 * to be zeroed to mark the outermost frame. Calls main(argc, argv,
 * envp), then exit with its result (C17 5.1.2.2.3). */

	.text
	.globl	_start
	.type	_start,@function
_start:
	xor	%ebp, %ebp
	mov	(%rsp), %rdi		/* argc */
	lea	8(%rsp), %rsi		/* argv */
	lea	8(%rsi,%rdi,8), %rdx	/* envp: past argv's null */
	and	$-16, %rsp		/* the call below pushes 8: main sees the psABI's alignment */
	call	main
	mov	%eax, %edi
	call	exit
	hlt
	.size	_start, .-_start

	.section .note.GNU-stack,"",@progbits
