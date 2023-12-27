#ifndef DIRTY_PAGES_HPP
#define DIRTY_PAGES_HPP

#include <unistd.h>

class DirtyPages {
private:
  int page_map_fd;
  int mappes_fd;
  pid_t pid;

  bool isPageDirty(unsigned long long pagemap_entry);
  unsigned long long getPageFrameNumber(unsigned long long pagemap_entry);


public:
  void open_proc_files(void);

  void close_proc_files(void);

  //void print_dirty_pages();
  
  void get_virtual_address_range(void);

};

#endif // !DIRTY_PAGES_HPP

