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

/* lc03. C function pointers (qsort's comparator, atexit's handlers):
 * wolf has no C function-pointer type yet (wolf-lang#520), so a call
 * through one goes by a trampoline that moves the arguments down one
 * register and tail-jumps to the pointer (psABI 3.2.3). */
	.text
/* int __muzzle_call2(void *f, const void *a, const void *b): f(a, b). */
	.globl	__muzzle_call2
	.type	__muzzle_call2,@function
__muzzle_call2:
	mov	%rdi, %rax
	mov	%rsi, %rdi
	mov	%rdx, %rsi
	jmp	*%rax
	.size	__muzzle_call2, .-__muzzle_call2

/* void __muzzle_call0(void (*f)(void)): f(). */
	.globl	__muzzle_call0
	.type	__muzzle_call0,@function
__muzzle_call0:
	jmp	*%rdi
	.size	__muzzle_call0, .-__muzzle_call0

/* double strtod(const char *nptr, char **endptr), C17 7.22.1.3. The
 * conversion is wolf's (stdlib), and answers the double's bits in %rax:
 * wolf has no floating point type on this target. The psABI returns a
 * double in %xmm0 (3.2.3), so the entry moves the bits there. The
 * sub keeps %rsp 16-byte aligned at the call. */
	.globl	strtod
	.type	strtod,@function
strtod:
	sub	$8, %rsp
	call	__muzzle_strtod
	add	$8, %rsp
	movq	%rax, %xmm0
	ret
	.size	strtod, .-strtod

/* POSIX.1-2024 XBD 8.1's environ: _start (crt0.S) stores the initial
 * environment's address here; setenv may point it at a list of its own.
 * __muzzle_environ is the same object, for wolf's `extern "c" let`. */
	.data
	.balign	8
	.globl	environ
	.globl	__muzzle_environ
	.type	environ,@object
	.size	environ, 8
environ:
__muzzle_environ:
	.quad	0

/* lc03's storage, for the reasons above (a var holds a scalar only and
 * has no address, wolf-lang#597):
 * __muzzle_atexit: the first 32 atexit handlers (C17 7.22.4.2p3's
 *   minimum); more move the table to the heap.
 * __muzzle_limbs: strtod's three big integers, 160 32-bit limbs each.
 * __muzzle_errbuf: strerror's "Unknown error N" (C17 7.24.6.2p2 lets a
 *   later call overwrite it).
 * __muzzle_sigbuf: abort's signal set and its struct sigaction. */
	.bss
	.balign	16
	.globl	__muzzle_atexit
	.type	__muzzle_atexit,@object
	.size	__muzzle_atexit, 256
__muzzle_atexit:
	.zero	256
	.globl	__muzzle_limbs
	.type	__muzzle_limbs,@object
	.size	__muzzle_limbs, 1920
__muzzle_limbs:
	.zero	1920
	.globl	__muzzle_errbuf
	.type	__muzzle_errbuf,@object
	.size	__muzzle_errbuf, 32
__muzzle_errbuf:
	.zero	32
	.globl	__muzzle_sigbuf
	.type	__muzzle_sigbuf,@object
	.size	__muzzle_sigbuf, 32
__muzzle_sigbuf:
	.zero	32

/* <time.h>'s storage and texts (lc03, the second part):
 * __muzzle_tm: the struct tm gmtime and localtime answer (C17 7.27.3p1:
 *   the broken-down time they share may be overwritten by the next call).
 * __muzzle_ts: a struct timespec time() asks the kernel to fill.
 * __muzzle_day_names, __muzzle_mon_names: the C locale's names
 *   (C17 7.27.3.5p7), as tables of pointers; the abbreviations are their
 *   first three letters.
 * __muzzle_text_gmt, __muzzle_text_utc: tm_zone for gmtime_r and for
 *   localtime_r (the local zone is UTC). */
	.bss
	.balign	16
	.globl	__muzzle_tm
	.type	__muzzle_tm,@object
	.size	__muzzle_tm, 64
__muzzle_tm:
	.zero	64
	.globl	__muzzle_ts
	.type	__muzzle_ts,@object
	.size	__muzzle_ts, 16
__muzzle_ts:
	.zero	16

	.section .data.rel.ro,"aw"
	.balign	8
	.globl	__muzzle_day_names
	.type	__muzzle_day_names,@object
	.size	__muzzle_day_names, 56
__muzzle_day_names:
	.quad	.Lday0
	.quad	.Lday1
	.quad	.Lday2
	.quad	.Lday3
	.quad	.Lday4
	.quad	.Lday5
	.quad	.Lday6
	.globl	__muzzle_mon_names
	.type	__muzzle_mon_names,@object
	.size	__muzzle_mon_names, 96
__muzzle_mon_names:
	.quad	.Lmon0
	.quad	.Lmon1
	.quad	.Lmon2
	.quad	.Lmon3
	.quad	.Lmon4
	.quad	.Lmon5
	.quad	.Lmon6
	.quad	.Lmon7
	.quad	.Lmon8
	.quad	.Lmon9
	.quad	.Lmon10
	.quad	.Lmon11

	.section .rodata.str1.1,"aMS",@progbits,1
.Lday0:	.asciz	"Sunday"
.Lday1:	.asciz	"Monday"
.Lday2:	.asciz	"Tuesday"
.Lday3:	.asciz	"Wednesday"
.Lday4:	.asciz	"Thursday"
.Lday5:	.asciz	"Friday"
.Lday6:	.asciz	"Saturday"
.Lmon0:	.asciz	"January"
.Lmon1:	.asciz	"February"
.Lmon2:	.asciz	"March"
.Lmon3:	.asciz	"April"
.Lmon4:	.asciz	"May"
.Lmon5:	.asciz	"June"
.Lmon6:	.asciz	"July"
.Lmon7:	.asciz	"August"
.Lmon8:	.asciz	"September"
.Lmon9:	.asciz	"October"
.Lmon10:	.asciz	"November"
.Lmon11:	.asciz	"December"

	.section .rodata
	.globl	__muzzle_text_gmt
	.type	__muzzle_text_gmt,@object
	.size	__muzzle_text_gmt, 4
__muzzle_text_gmt:
	.asciz	"GMT"
	.globl	__muzzle_text_utc
	.type	__muzzle_text_utc,@object
	.size	__muzzle_text_utc, 4
__muzzle_text_utc:
	.asciz	"UTC"

	.section .note.GNU-stack,"",@progbits
