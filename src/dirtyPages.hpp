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
  bool isPageDirty(unsigned long long pagemap_entry);

  // Open maps file to find the start and the end virtual address range of the
  // mapped file.
  // Remember in the JVM to change this function to use the counters from the
  // allocator 
  void get_virtual_addr_range(void);

public:
  // Constructor
  DirtyPages(); 

  // Read the corresponding entries in the /proc/pid/pagemap file for the
  // given virtual address range.
  void print_dirty_pages(void);
};

#endif // DIRTY_PAGES_HPP
