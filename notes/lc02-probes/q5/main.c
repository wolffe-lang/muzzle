#include "t.h"
unsigned long q5_cycle(unsigned long);
int main(void) { t_kv("q5.cycle", (long)q5_cycle(41)); t_kv("q5.ok", q5_cycle(41) == 42 && q5_cycle(99) == 100); return 0; }
