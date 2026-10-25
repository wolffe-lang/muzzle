# The ledger

Every case in `tests/cases/` agrees with glibc on stdout and exit
status. A row here names where the C standard or POSIX leaves the
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

## How muzzle's link differs (lc01)

| what | why | until |
|---|---|---|
| every case builds with `-fno-stack-protector`, in both columns | the protector's canary is read at `%fs:0x28`, and muzzle sets up no TLS block, so `%fs` is 0 | lc05 (the main thread's TLS block) |
| `exit` is `_Exit` | no `atexit` (C function pointers, wolf-lang#520) and no streams to flush (lc08) | lc04, lc08 |
| errno is one process-wide int in `sys.S` | ruling R4's single-threaded first cut; it is not a wolf `var` because a `var` has no address (wolf-lang#597) | #597 for the `var`; lc05 (#517) per thread |
| the package root calls a never-used `reach()` | a module reached only by C is not compiled without an import, and an import nothing uses is E0305 (wolf-lang#599) | #599 |
| the heap state is loaded and stored once per `carve` | wolf 0.2.24 forwards a module var's value across a call that writes it (wolf-lang#598) | #598 |
| the trap hook writes its fixed text from packed `u64` constants | a `str` literal's bytes cannot reach the kernel as a `*u8` (wolf-lang#604) | #604 |

## printf acceptance slice (s263; not full stdio)

Only bare `%d`, `%s`, `%c`, `%x` and `%%` are supported. `%d` reads
promoted `int`, `%x` reads `unsigned int`, `%c` reads `int` then converts
to unsigned char, and `%s` reads a pointer to a null-terminated byte
string. There are no flags, widths, precisions, length modifiers, wide
characters, floating conversions, FILE objects, or buffering. Unsupported
conversions return -1 with EINVAL (22), possibly after writing a prefix;
this is a subset boundary, not a claim of full C printf conformance.

The formatter writes fd 1 directly, retries EINTR and short writes,
returns -1 on a write error, and rejects a return count above INT_MAX
with EOVERFLOW (75). Its 16-byte digit/character scratch buffer is
process-wide, separate from the trap buffer: single-threaded only, not
reentrant or async-signal-safe. A null `%s` is outside its precondition;
no test relies on glibc's optional `(null)` spelling. The corpus uses a
regular file for stdout. Additional muzzle-only tests cover invalid formats
and EBADF; EINTR, short writes and count overflow are not dynamically
exercised by this acceptance. The lc02 draft requires those tests.

`tests/cases/printf.c` contains fifty primary calls plus count metadata.
Every call checks a separately spelled expected byte count; differential
comparison includes embedded NUL bytes, promotions, INT_MIN, UINT_MAX,
and integer/pointer overflow from the GP save area onto the stack.
The toolchain pin remains 0.2.34 until a released variadic compiler exists.
