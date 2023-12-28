#include "dirtyPages.hpp"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define LEN 150
#define BUFFER 4096
  
DirtyPages::DirtyPages() {
  pid = getpid();
  start_virt_addr = 0;
  end_virt_addr = 0;

  get_virtual_addr_range();
  assert(start_virt_addr != 0 && end_virt_addr != 0);

  printf("start_virt_addr = %#llx\n", start_virt_addr);
  printf("end_virt_addr = %#llx\n", end_virt_addr);
}

void DirtyPages::get_virtual_addr_range(void) {
  char maps_file[LEN];
  FILE *fd;
  snprintf(maps_file, LEN, "/proc/%d/maps", pid);

  fd = fopen(maps_file, "r");
  if (fd == NULL) {
    perror("Error opening file");
    return;
  }

  char *line = NULL;
  size_t len = 0;
  ssize_t nread;

  // Read the file line by line
  while ((nread = getline(&line, &len, fd)) != -1) {
    // Check if the substring is present in the line
    if (strstr(line, "/mnt/fmap/") != NULL) {
      printf("Found: %s", line);
      fclose(fd);
      break;
    }
  }

  if (line == NULL) {
    perror("String pattern not found");
    return;
  }

  // The line is in the form
  // 555786a3e000-555786a40000 r--p 00000000 103:02 30933618 /mnt/fmap/.uxzs
  char *token = strtok(line, "-"); // Tokenize based on "-"

  if (token != NULL) {
    // Convert the hexadecimal string to an unsigned long long
    start_virt_addr = strtoull(token, NULL, 16);
  } else {
    perror("Error parsing the line (start address)\n");
    return;
  }

  // Tokenize based on " "
  token = strtok(NULL, " ");
  if (token != NULL) {
    // Convert the hexadecimal string to an unsigned long long
    end_virt_addr = strtoull(token, NULL, 16);
  } else {
    perror("Error parsing the line (end address)\n");
    return;
  }

  if (line != NULL) {
    free(line);
  }
}

void DirtyPages::print_dirty_pages(void) {
  char pagemap_path[LEN];
  snprintf(pagemap_path, sizeof(pagemap_path), "/proc/%d/pagemap", pid);

  int pagemap_fd = open(pagemap_path, O_RDONLY);
  if (pagemap_fd == -1) {
    perror("Error opening pagemap file");
    return;
  }

  // Calculate the number of pages in the range
  size_t num_pages = (end_virt_addr - start_virt_addr) / PAGE_SIZE;

  // Read corresponding pagemap entries
  for (size_t i = 0; i < num_pages; ++i) {
    unsigned long long offset = (start_virt_addr / PAGE_SIZE + i) * sizeof(uint64_t);
    lseek(pagemap_fd, offset, SEEK_SET);

    uint64_t pagemap_entry;
    ssize_t read_size = read(pagemap_fd, &pagemap_entry, sizeof(pagemap_entry));

    if (read_size != sizeof(pagemap_entry)) {
      fprintf(stderr, "Error reading pagemap entry for address %llx\n", start_virt_addr + i * PAGE_SIZE);
      close(pagemap_fd);
      return;
    }

    // Check if the page is dirty
    if (isPageDirty(pagemap_entry)) {
      printf("Page at address %llx is dirty.\n", start_virt_addr + i * PAGE_SIZE);
    }
  }

  close(pagemap_fd);
}

// Function to check if a page is dirty
bool DirtyPages::isPageDirty(unsigned long long pagemap_entry) {
    return (pagemap_entry & (1ULL << 55)) != 0;
}
