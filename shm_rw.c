#include <stdio.h>
#include <stdint.h>

/* Assume gem5 system has 1 GB physical memory, we use the last 4 MB as "shared area"
 * 0x3FC_0000 to 0x3FF_FFFF (physical address)
 * Works in both bare metal/FS mode, as long as this physical address is not occupied by other devices.
 * Note: Direct physical address access here, so either run on bare metal (--kernel),
 *       or in FS mode use mmap /dev/mem with MMU disabled, or manually do ioremap.
 */
#define SHARE_PADDR 0x3FC0000UL
#define SHARE_SIZE  1024              /* Only test 1 kB */

static inline void write_mem_fence(void) { __asm__ volatile("mfence" ::: "memory"); }
static inline void read_mem_fence(void)  { __asm__ volatile("mfence" ::: "memory"); }

int main(void)
{
    volatile uint64_t *share = (volatile uint64_t *)SHARE_PADDR;

    /* 1. Write */
    for (size_t i = 0; i < SHARE_SIZE / sizeof(uint64_t); ++i)
        share[i] = 0xDEADBEEF00000000ULL | i;

    write_mem_fence();

    /* 2. Read back and verify */
    int ok = 1;
    read_mem_fence();
    for (size_t i = 0; i < SHARE_SIZE / sizeof(uint64_t); ++i) {
        uint64_t v = share[i];
        if (v != (0xDEADBEEF00000000ULL | i)) {
            ok = 0;
            break;
        }
    }

    /* 3. Print result */
    if (ok)
        printf("[shared_bmk] PASS: all %d bytes match\n", SHARE_SIZE);
    else
        printf("[shared_bmk] FAIL: data mismatch\n");

    return ok ? 0 : 1;
}
