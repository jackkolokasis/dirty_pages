#include "dirtyPages.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <bitset>
#include <sys/types.h>
#include <unistd.h>

void DirtyPages::open_proc_files(void) {


}

void DirtyPages::close_proc_files(void) {
  close(mappes_fd);
  close(page_map_fd);
}


using namespace std;

// Function to check if a page is dirty
bool DirtyPages::isPageDirty(unsigned long long pagemap_entry) {
    return (pagemap_entry & (1ULL << 55)) != 0;
}

// Function to extract page frame number from pagemap entry
unsigned long long DirtyPages::getPageFrameNumber(unsigned long long pagemap_entry) {
    return pagemap_entry & ((1ULL << 55) - 1);
}

//void DirtyPages::print_dirty_pages() {
//  // Get the process ID of the current process
//  pid_t pid = getpid();
//
//  // Build the paths for maps and pagemap files
//  // Maps contains the virtual address space while the pagemaps
//  // contains the physical pages
//  stringstream maps_path;
//  maps_path << "/proc/" << pid << "/maps";
//
//  stringstream pagemap_path;
//  pagemap_path << "/proc/" << pid << "/pagemap";
//
//  // Open maps file for reading
//  ifstream maps_file(maps_path.str());
//  if (!maps_file.is_open()) {
//    cerr << "Error: Unable to open " << maps_path.str() << endl;
//    return;
//  }
//
//  // Open pagemap file for reading
//  ifstream pagemap_file(pagemap_path.str(), ios::binary);
//  if (!pagemap_file.is_open()) {
//    cerr << "Error: Unable to open " << pagemap_path.str() << endl;
//    return;
//  }
//
//  // Read maps file line by line
//  string line;
//  while (getline(maps_file, line)) {
//    istringstream iss(line);
//
//    // Extract start and end address of the mapped region
//    unsigned long long start_addr, end_addr;
//    char dash;
//    iss >> hex >> start_addr >> dash >> end_addr;
//
//    size_t num_pages = (end_addr - start_addr) / getpagesize();
//    // Calculate the number of pages in the range
//
//    // Read corresponding pagemap entries
//    for (size_t i = 0; i < num_pages; ++i) {
//      unsigned long long pagemap_entry;
//      // Seek to the position in the pagemap file corresponding to the current page
//      pagemap_file.seekg((start_addr / getpagesize() + i) * sizeof(pagemap_entry), ios::beg);
//      pagemap_file.read(reinterpret_cast<char*>(&pagemap_entry), sizeof(pagemap_entry));
//
//      // Check if the page is dirty
//      if (isPageDirty(pagemap_entry)) {
//        cout << "Page at address " << hex << start_addr + i * getpagesize() << " is dirty." << endl;
//      }
//    }
//  }
//}
//
  
void DirtyPages::get_virtual_address_range(void) {
  page_map_fd = open()
}
