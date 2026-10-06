typedef unsigned long size_t;
long write(int, const void *, size_t);
size_t strlen(const char *);
void exit(int);
int __errno_value(void);
int main(void) {
    const char *m = "q2 hello\n";
    write(1, m, strlen(m));
    long r = write(-1, m, 1);
    char b[2] = { (char)('0' + __errno_value()), '\n' };
    write(1, "errno=", 6); write(1, b, 2);
    return r == -1 ? 7 : 1;
}
