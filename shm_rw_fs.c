#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>

/* For FS mode - use /dev/mem to access physical memory
 * This approach works in full system mode with proper permissions
 */
#define SHARE_PADDR 0x3FC0000UL
#define SHARE_SIZE  1024              /* Only test 1 kB */

static inline void write_mem_fence(void) { __asm__ volatile("mfence" ::: "memory"); }
static inline void read_mem_fence(void)  { __asm__ volatile("mfence" ::: "memory"); }

int main(void)
{
    int mem_fd;
    volatile uint64_t *share;
    
    printf("[shm_rw_fs] Opening /dev/mem for physical memory access...\n");
    
    /* Open /dev/mem for physical memory access */
    mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (mem_fd < 0) {
        perror("Failed to open /dev/mem");
        printf("[shm_rw_fs] ERROR: Make sure to run this program as root (sudo)\n");
        printf("[shm_rw_fs] You may also need to: modprobe memdev or echo 0 > /proc/sys/kernel/mmap_min_addr\n");
        return 1;
    }
    
    /* Map physical memory into our address space */
    share = (volatile uint64_t *)mmap(
        NULL,                    /* Let kernel choose virtual address */
        SHARE_SIZE,              /* Size to map */
        PROT_READ | PROT_WRITE,  /* Read/write access */
        MAP_SHARED,              /* Shared mapping */
        mem_fd,                  /* File descriptor for /dev/mem */
        SHARE_PADDR              /* Physical address to map */
    );
    
    if (share == MAP_FAILED) {
        perror("mmap failed");
        close(mem_fd);
        return 1;
    }
    
    printf("[shm_rw_fs] Successfully mapped physical address 0x%lX to virtual address %p\n", 
           SHARE_PADDR, share);
    
    /* 1. Write test pattern */
    printf("[shm_rw_fs] Writing test pattern...\n");
    for (size_t i = 0; i < SHARE_SIZE / sizeof(uint64_t); ++i) {
        share[i] = 0xDEADBEEF00000000ULL | i;
    }
    write_mem_fence();
    
    /* 2. Read back and verify */
    printf("[shm_rw_fs] Reading back and verifying...\n");
    int ok = 1;
    read_mem_fence();
    for (size_t i = 0; i < SHARE_SIZE / sizeof(uint64_t); ++i) {
        uint64_t v = share[i];
        if (v != (0xDEADBEEF00000000ULL | i)) {
            printf("[shm_rw_fs] Mismatch at index %zu: expected 0x%016llX, got 0x%016llX\n", 
                   i, (unsigned long long)(0xDEADBEEF00000000ULL | i), (unsigned long long)v);
            ok = 0;
            break;
        }
    }
    
    /* 3. Print result */
    if (ok) {
        printf("[shm_rw_fs] PASS: all %d bytes match\n", SHARE_SIZE);
    } else {
        printf("[shm_rw_fs] FAIL: data mismatch\n");
    }
    
    /* Cleanup */
    munmap((void*)share, SHARE_SIZE);
    close(mem_fd);
    
    return ok ? 0 : 1;
}
