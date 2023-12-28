#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cstdlib>
#include <string.h>
#include "dirtyPages.hpp"

#define FILE_NAME "/mnt/fmap/file.txt"
#define FILE_SIZE (2 * 1024 * 1024 * 1024LU)
#define NUM_DIRTY_PAGES 10

int fd = 0;
char* mapped_data = NULL;

void create_file() {
  // Create a file and open it for read and write
  fd = open(FILE_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    std::cerr << "Error opening file." << std::endl;
    exit(EXIT_FAILURE);
  }

  // Allocate space for the file using posix_fallocate
  if (posix_fallocate(fd, 0, FILE_SIZE) != 0) {
    std::cerr << "Error allocating space for the file." << std::endl;
    close(fd);
    exit(EXIT_FAILURE);
  }

  // Map the file into memory
  mapped_data = (char *) mmap(nullptr, FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (mapped_data == MAP_FAILED) {
    std::cerr << "Error mapping file into memory." << std::endl;
    close(fd);
    exit(EXIT_FAILURE);
  }
}

void delete_file() {
  // Unmap the file from memory
  if (munmap(mapped_data, FILE_SIZE) == -1) {
    std::cerr << "Error unmapping file from memory." << std::endl;
  }

  // Close the file
  close(fd);
}

void create_dirty_pages(int num_pages) {
  // Fill the pages with data to make them dirty
  for (int i = 0; i < num_pages; ++i) {
    memset(mapped_data + i * PAGE_SIZE, i, PAGE_SIZE);
  }
    
  // Access each page to make them resident in memory
  for (int i = 0; i < num_pages; ++i) {
    volatile char value = mapped_data[i * PAGE_SIZE];
    (void)value; // Avoid compiler optimizations for the unused variable
  }
}

int main() {
  create_file();
  DirtyPages *dpages = new DirtyPages();

  create_dirty_pages(NUM_DIRTY_PAGES);
  dpages->print_dirty_pages();

  delete_file();
  return 0;
}
