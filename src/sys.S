/* muzzle: the kernel boundary in assembly (ruling R2: muzzle makes
 * syscalls through a linked .S until wolf has inline asm, KWC kw13).
 *
 * Written from the x86-64 psABI (3.2.3 parameter passing, A.2.1 the
 * Linux kernel convention) and syscall(2)'s register table: the number
 * in rax, arguments in rdi rsi rdx r10 r8 r9, the result in rax, rcx
 * and r11 clobbered by the instruction. */

	.text

/* long __muzzle_syscall6(long n, long a1, long a2, long a3, long a4,
 *                        long a5, long a6)
 * C convention in: n=rdi a1=rsi a2=rdx a3=rcx a4=r8 a5=r9 a6=8(%rsp).
 * Returns the kernel's rax as is: -4095..-1 is -errno (sys.lu). */
	.globl	__muzzle_syscall6
	.type	__muzzle_syscall6,@function
__muzzle_syscall6:
	mov	%rdi, %rax
	mov	%rsi, %rdi
	mov	%rdx, %rsi
	mov	%rcx, %rdx
	mov	%r8, %r10
	mov	%r9, %r8
	mov	8(%rsp), %r9
	syscall
	ret
	.size	__muzzle_syscall6, .-__muzzle_syscall6

/* Static storage wolf names with `extern "c" let` ([abi.link.extern]).
 *
 * __muzzle_errno: errno's int. It belongs in a wolf module `var`
 *   (ruling R4's first cut), but a `var` has no address to hand C
 *   (wolf-lang#597), so it lives here until that lands.
 * __muzzle_bins: the allocator's 16 free-list heads; module state holds
 *   scalars only ([mem.static.3]), so a table is reserved here.
 * __muzzle_scratch: 32 bytes the trap hook formats a number into. */
	.bss
	.balign	16
	.globl	__muzzle_errno
	.type	__muzzle_errno,@object
	.size	__muzzle_errno, 4
__muzzle_errno:
	.zero	16
	.globl	__muzzle_bins
	.type	__muzzle_bins,@object
	.size	__muzzle_bins, 128
__muzzle_bins:
	.zero	128
	.globl	__muzzle_scratch
	.type	__muzzle_scratch,@object
	.size	__muzzle_scratch, 32
__muzzle_scratch:
	.zero	32

	.section .note.GNU-stack,"",@progbits
