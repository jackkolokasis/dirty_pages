#ifndef DIRTY_PAGES_HPP
#define DIRTY_PAGES_HPP

#include <unistd.h>
#include <stdint.h>
#include <assert.h>

#define PAGE_SIZE 4096

class DirtyPages {
private:
  pid_t pid;
  unsigned long long start_virt_addr;
  unsigned long long end_virt_addr;

  // Check if the page is dirty
  bool is_page_dirty(uint64_t page_flags);

  // Open maps file to find the start and the end virtual address range of the
  // mapped file.
  // Remember in the JVM to change this function to use the counters from the
  // allocator 
  void get_virtual_addr_range(void);

  // Check if page is present (bit 63). Otherwise, there is no PFN.
  bool is_page_in_dram(uint64_t page_info);

  // Create a mask that has ones in the lowest 54 bits, and use that to
  // extract the page frame number (PFN).
  uint64_t get_page_frame_number(uint64_t page_info);

  //  Having the page frame number (pfn) in hands, we can use that as index into
  //  /proc/kpageflags to get to our page’s flags. Each entry is 64 bits
  //  long, so we must skip ahead in 8-Byte-steps.
  size_t get_pflags(uint64_t pfn);

public:
  // Constructor
  DirtyPages(); 

  // Read the corresponding entries in the /proc/pid/pagemap file for the
  // given virtual address range.
  void print_dirty_pages(void);
};

#endif // DIRTY_PAGES_HPP
