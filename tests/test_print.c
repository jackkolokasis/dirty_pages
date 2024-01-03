#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define TRACE_DIRTY_PAGES_MAGIC 'T'
#define TRACE_DIRTY_PAGES _IOW(TRACE_DIRTY_PAGES_MAGIC, 1, unsigned long[2])

int main() {
    int fd = open("/dev/trace_dirty_pages", O_RDWR);
    if (fd < 0) {
        perror("Failed to open /dev/virtual_address");
        exit(EXIT_FAILURE);
    }
    /* Example virtual addresses */
    unsigned long va_array[2] = {0x12345678, 0xABCD5678};

    /* Request virtual addresses from the kernel */
    if (ioctl(fd, TRACE_DIRTY_PAGES, va_array) < 0) {
        perror("IOCTL VIRT_ADDR_GET failed");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd); /* Close file */

    return 0;
}
