# The ledger

Every case in `tests/cases/` agrees with glibc on stdout, stderr, the
exit status and the files it writes. A row here names where the C standard or POSIX leaves the
behaviour open, so the agreement is with glibc's choice, not a
requirement, or where muzzle's link differs from an ordinary C program's.
lc00's rule: the standard is the oracle; glibc is the default where the
standard leaves the behaviour to the implementation.

## Behaviour the standards leave open (agrees with glibc)

| case line | the standard | muzzle and glibc |
|---|---|---|
| `malloc.c` `malloc0.nonnull` | C17 7.22.3p1, `malloc(3p)`: a size of zero answers a null pointer or a unique pointer, implementation-defined | a unique block of the smallest class, which `free` accepts |
| `errno.c` `errno.kept_on_success` | `errno(3p)`: after a successful call errno is unspecified unless the function's page says it is not modified | a successful `write` leaves errno alone |
| `write.c` `write.zero` | `write(3p)`: nbyte 0 on a file that is not regular is unspecified | the harness sends stdout to a regular file, so the specified case (answers 0) is the one run |
| (no case) `realloc(p, 0)` | `realloc(3p)`: implementation-defined | muzzle frees and answers a null pointer, as `malloc(3)` describes; no case relies on it |

### stdio and the printf family (lc02)

Seen as a black box first (`notes/lc02-probes/g1.c` … `g5.c`), then held by
the cases named.

| case line | the standard | muzzle and glibc |
|---|---|---|
| `stdio_buf.c` `full16.after16`, `after17` | C17 7.21.3p3: a fully buffered stream's bytes are "intended to be transmitted ... as a block when a buffer is filled" | a full buffer is written when the next byte arrives, not when it fills |
| `stdio_buf.c` `full256.*`, `default.*`, `full16.fwrite*` | the same | a write that fits the room left joins the buffer, even when it fills it; one that does not fit fills the room, sends the full buffer, sends the rest's whole multiples of the buffer size straight to the file, and keeps the remainder; a buffer under 128 bytes sends the whole rest |
| `stdio_buf.c` `full16.fwrite16_exact` (a buffer of the program's own, first written by fwrite) | the same | until a byte has gone into a buffer, a write of several bytes finds no room in it |
| `stdio_buf.c` `default.putc5000`, `setbuf.putc9000` | C17 7.21.5.6: with a null buffer "the setvbuf function [may] allocate" one; its size is the implementation's | the file's preferred size, `st_blksize` (4096 for these files); `setvbuf`'s size is ignored when buf is null; `setbuf` uses BUFSIZ (8192) |
| `stdio_buf.c` `line*.*`, `stdio_tty.c` | C17 7.21.3p3: a line-buffered stream transmits "when a new-line character is encountered" | the bytes through the last newline go out; what follows stays, except in a buffer under 128 bytes, which sends it too (`line16.fputs_nl`) |
| `stdio_tty.c` `[8]`–`[9]` | C17 7.21.3p3: a read that needs the host flushes line-buffered output when the input stream is unbuffered or line buffered | a read from a fully buffered stdin (a file) does not flush stdout |
| `stdio_buf.c` `full.fclose` | C17 7.21.5.2 says only that fflush fails | a failed write drops the pending bytes, so fclose afterwards has nothing to retry and answers 0 |
| `stdio_buf.c` `setvbuf.badmode` | C17 7.21.5.6: nonzero for an invalid mode | EOF, errno unchanged |
| `stdio_char.c` `fputs.*` | C17 7.21.7.4: "a nonnegative value" on success | 1 |
| `stdio_exit.c` (puts) | C17 7.21.7.9: "a nonnegative value" on success | the bytes written, the newline included |
| `stdio_char.c` `fgets.n1`, `fgets.n0_null` | C17 7.21.7.2 reads at most n-1 bytes; n below 1 is not described | n of 1 stores only the null and answers s; n of 0 answers a null pointer |
| `stdio_ungetc.c` `unget.Y`, `unget.X` | C17 7.21.7.10p3: one byte of pushback is guaranteed | more are taken, last in first out: muzzle takes 8, glibc more; no case pushes more than 2 |
| `stdio_ungetc.c` `offset.with_pushback` | C17 7.21.5.2: fflush of an input stream is undefined; POSIX sets the descriptor's offset to the stream's position and drops pushed-back bytes | with the buffer empty, a pushed-back byte moves the offset back one and is dropped, in both |
| `stdio_open.c` `mode.rw`, `mode.r+bx`, `mode.rbe` | C17 7.21.5.3p3: a mode not in the list is undefined | the first byte decides r, w or a; after it `+`, `x` and `e` count and any other byte is ignored, so "rw" opens for reading |
| `stdio_open.c` `fdopen.w_on_rdonly` | POSIX fdopen "may fail" with EINVAL for a mode the descriptor's access does not allow | it fails, EINVAL |
| `stdio_open.c` `fdopen.a`, `fdopen.w_does_not_truncate` | POSIX fdopen: the mode does not truncate | the descriptor is left as it is (no O_APPEND added) |
| `printf.c` `[%p]` | C17 7.21.6.1p8: `%p` is implementation-defined | `0x` and lower-case hex; a null pointer is `(nil)`; `+`, space, `0` and a precision apply as to a signed conversion, but not to `(nil)` |
| `printf.c` `[%s]` of a null pointer | undefined (C17 7.21.6.1p8 asks for a pointer to an array) | `(null)`, or nothing when a precision below 6 cannot hold it; `%ls` the same |
| `printf.c` `[%k]` … | C17 7.21.6.1p9: an invalid specification is undefined | printed back from what was parsed: `%`, the flags in the order `# ' + - 0`, the width, the precision, the character; the length dropped |
| `printf.c` `[%5%]` | C17 7.21.6.1p8: "the complete conversion specification shall be %%" | flags, width and precision are ignored |
| `printf.c` `[%`, `abc%lcdef` … | C17 7.21.6.1p14: a negative value "if an output or encoding error occurred"; POSIX leaves the output unspecified | the call writes none of its output to a stream; to memory, what it formatted before the failure stays, with its null |
| `printf.c` `[%lc]` of 0xe9, `[%ls]` | the C locale's character set is implementation-defined (C17 7.11.1.1) | ASCII: a wide character above 127 fails with EILSEQ |
| `printf.c` `big.3` … `big.7` | POSIX fprintf: EOVERFLOW when the count would exceed INT_MAX | the same for a width or precision past INT_MAX, a `*` width of INT_MIN included |
| `printf.c` `[%qd]`, `[%Zu]`, `[%Lx]` | not C17 lengths (undefined) | glibc's: `q` and `L` are `ll`, `Z` is `z` |

### Not provided yet, by name

| what | the standard | muzzle |
|---|---|---|
| the floating conversions `f F e E g G a A` and the `L` length with them | C17 7.21.6.1 | **R-lc02-1 = B**: lc03 owns them. A call that meets one answers -1 with errno EINVAL, as any failed call does: a stream receives none of its output; in memory what came before it stays, with its null (`tests/own/printf_refusals.c`) |
| positional arguments `%n$` and `*m$` | POSIX fprintf (XSI) | refused the same way (`tests/own/printf_refusals.c`) |
| `%m` | a glibc extension | not one: printed back as an unknown conversion; no case uses it |

### Where glibc departs from the standard (muzzle follows the standard)

| what | the standard | glibc | muzzle |
|---|---|---|---|
| fflush of an input stream with read-ahead and a pushed-back byte both pending | POSIX XSH fflush: the offset becomes the stream's position; pushed-back bytes are dropped "without further changing the file offset" | moves the offset back over the pushback alone and keeps the read-ahead (`abcdef` read to `c`, `ungetc`, `fflush`: offset 5, next byte `d`; build6's `stdio_ungetc`) | the stream's position (offset 2, next byte `c`): `tests/own/fflush_pushback.c`; no differential case has this shape |

### Known to differ, not exercised

| what | glibc | muzzle |
|---|---|---|
| a printf that fails after its output has passed the stream's buffer size | writes one byte more than its buffer had sent (4097 of 5000, 8193 of 9000: `notes/lc02-evidence/g5-glibc.out`, `pf.fail.*`) | writes what its buffer had sent (4096, 8192) |
| a long write to a line-buffered stream whose newline lies in the part that went out whole | the observed rule (`g5`: `lbf.long_nl_*`) | the same rule; only the two shapes probed are known to agree |

## How muzzle's link differs (lc01, lc02)

| what | why | until |
|---|---|---|
| every case builds with `-fno-stack-protector`, in both columns | the protector's canary is read at `%fs:0x28`, and muzzle sets up no TLS block, so `%fs` is 0 | lc05 (the main thread's TLS block) |
| `exit` flushes the streams but runs no `atexit` handler | C function pointers (wolf-lang#520) | lc04 |
| `stdin`, `stdout`, `stderr` and the standard streams' records live in `sys.S` | a module cannot export data (wolf-lang#516), and a `var` has no address (wolf-lang#597) | #516, #597 |
| the texts `(null)` and `(nil)` live in `sys.S` | a `str` literal's bytes cannot reach C as a `*u8` (wolf-lang#604) | #604 |
| errno is one process-wide int in `sys.S` | ruling R4's single-threaded first cut; it is not a wolf `var` because a `var` has no address (wolf-lang#597) | #597 for the `var`; lc05 (#517) per thread |
| the package root calls a never-used `reach()` | a module reached only by C is not compiled without an import, and an import nothing uses is E0305 (wolf-lang#599) | #599 |
| the heap state is loaded and stored once per `carve` | wolf 0.2.24 forwards a module var's value across a call that writes it (wolf-lang#598) | #598 |
| the trap hook writes its fixed text from packed `u64` constants | a `str` literal's bytes cannot reach the kernel as a `*u8` (wolf-lang#604) | #604 |
