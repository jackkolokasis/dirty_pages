#include "dirtyPages.hpp"
#include <pwd.h>
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

// The pagemap format specifies that for each page, there is a 64-bit entry in
// /proc/<PID>/pagemap, with the lower 54 bits indicating the page frame number
// (PFN). To get this PFN, we first seek to the correct position, then we read
// 64 bits and extract the PFN from it
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
    unsigned long long page_index = start_virt_addr / PAGE_SIZE + i;
    // Each entry in pagemap is 64 bits, i.e, 8 bytes.
    unsigned long long offset = page_index * sizeof(uint64_t);
    int res = lseek(pagemap_fd, offset, SEEK_SET);

    if (res == -1) {
      perror("Error seeking in pagemap file");
      close(pagemap_fd);
      return;
    }


    uint64_t pagemap_entry;
    ssize_t read_size = read(pagemap_fd, &pagemap_entry, sizeof(pagemap_entry));

    if (read_size != sizeof(pagemap_entry)) {
      fprintf(stderr, "Error reading pagemap entry for address %llx\n", start_virt_addr + i * PAGE_SIZE);
      close(pagemap_fd);
      return;
    }

    if (!is_page_in_dram(pagemap_entry)) {
      continue;
    }

#ifdef KFLAGS
    // Create a mask that has ones in the lowest 54 bits, and use that to
    // extract the PFN.
    uint64_t pfn = get_page_frame_number(pagemap_entry);
    printf("Pagemap entry = %lx\n", pagemap_entry);
    printf("PFN = %lu\n", pfn);
    uint64_t pf = get_pflags(pfn);

    // Check if the page is dirty
    if (is_page_dirty(pf)) {
      printf("Page at address %llx is dirty.\n", start_virt_addr + i * PAGE_SIZE);
    }
#else
    // Check if the page is dirty
    if (is_pte_dirty(pagemap_entry)) {
      printf("Page at address %llx is dirty.\n", start_virt_addr + i * PAGE_SIZE);
    }
#endif
  }

  close(pagemap_fd);
}
  
// Check if the page table entry is dirty
bool DirtyPages::is_pte_dirty(uint64_t pagemap_entry) {
  return (pagemap_entry >> 55) & 0x1;
}

// Function to check if a page is dirty
bool DirtyPages::is_page_dirty(uint64_t page_flags) {
  return (page_flags >> 4) & 0x1;
}

// Check if page is present (bit 63). Otherwise, there is no PFN.
bool DirtyPages::is_page_in_dram(uint64_t page_info) {
  return (page_info & (static_cast<uint64_t>(1) << 63));
}

// Create a mask that has ones in the lowest 54 bits, and use that to
// extract the page frame number (PFN).
uint64_t DirtyPages::get_page_frame_number(uint64_t page_info) {
  return page_info & ((static_cast<uint64_t>(1) << 55) - 1);
}
  
//  Having the page frame number (pfn) in hands, we can use that as index into
//  /proc/kpageflags to get to our page’s flags. Each entry is 64 bits
//  long, so we must skip ahead in 8-Byte-steps.
uint64_t DirtyPages::get_pflags(uint64_t pfn) {
  int fd = open("/proc/kpageflags", O_RDONLY);
  if (fd == -1) {
    perror("Error opening pagemap file");
    return 0;
  }
    
  lseek(fd, pfn * 8, SEEK_SET);

  uint64_t pflags = 0;
  ssize_t read_size = read(fd, &pflags, sizeof(pflags));
    
  if (read_size != sizeof(pflags)) {
    fprintf(stderr, "Error reading pflag entry\n");
    close(pflags);
    return 0;
  }

  close(fd);
  return pflags;
}
