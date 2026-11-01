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

/* stdio's storage (lc02), for the same reasons: a wolf var holds a
 * scalar only and has no address (wolf-lang#597), and a module cannot
 * export data (wolf-lang#516).
 *
 * __muzzle_std_files: the three standard streams' FILE records, one
 *   256-byte slot each (stdio's ready() traps if its record outgrows a
 *   slot).
 * __muzzle_stage: 4096 bytes an unbuffered stream's printf output
 *   collects in, so it reaches the descriptor in one write.
 * __muzzle_termios: 64 bytes isatty's TCGETS fills (struct termios is
 *   36 bytes on x86-64).
 * __muzzle_stat: 144 bytes fstat fills (x86-64's struct stat). */
	.balign	16
	.globl	__muzzle_std_files
	.type	__muzzle_std_files,@object
	.size	__muzzle_std_files, 768
__muzzle_std_files:
	.zero	768
	.globl	__muzzle_stage
	.type	__muzzle_stage,@object
	.size	__muzzle_stage, 4096
__muzzle_stage:
	.zero	4096
	.globl	__muzzle_termios
	.type	__muzzle_termios,@object
	.size	__muzzle_termios, 64
__muzzle_termios:
	.zero	64
	.globl	__muzzle_stat
	.type	__muzzle_stat,@object
	.size	__muzzle_stat, 144
__muzzle_stat:
	.zero	144

/* C17 7.21.1p3's stdin, stdout and stderr: FILE pointers C reads, each
 * at its standard stream's slot. The __muzzle_ names are the same
 * objects, for wolf's `extern "c" let`. */
	.data
	.balign	8
	.globl	stdin
	.globl	__muzzle_stdin
	.type	stdin,@object
	.size	stdin, 8
stdin:
__muzzle_stdin:
	.quad	__muzzle_std_files
	.globl	stdout
	.globl	__muzzle_stdout
	.type	stdout,@object
	.size	stdout, 8
stdout:
__muzzle_stdout:
	.quad	__muzzle_std_files+256
	.globl	stderr
	.globl	__muzzle_stderr
	.type	stderr,@object
	.size	stderr, 8
stderr:
__muzzle_stderr:
	.quad	__muzzle_std_files+512

/* The texts glibc prints for a null %s and a null %p: a str literal's
 * bytes cannot reach C as a *u8 yet (wolf-lang#604). */
	.section .rodata
	.globl	__muzzle_text_null
	.type	__muzzle_text_null,@object
	.size	__muzzle_text_null, 6
__muzzle_text_null:
	.ascii	"(null)"
	.globl	__muzzle_text_nil
	.type	__muzzle_text_nil,@object
	.size	__muzzle_text_nil, 5
__muzzle_text_nil:
	.ascii	"(nil)"

	.section .note.GNU-stack,"",@progbits
