#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cstdlib>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#define TRACE_DIRTY_PAGES_MAGIC 'T'
#define TRACE_DIRTY_PAGES _IOW(TRACE_DIRTY_PAGES_MAGIC, 1, struct ioctl_data)

#define FILE_NAME "/mnt/fmap/file.txt"
#define FILE_SIZE (2 * 1024 * 1024 * 1024LU)
#define PAGE_SIZE 4096
#define NUM_DIRTY_PAGES 262144

int fd = 0;
char* mapped_data = NULL;
unsigned long start_address, end_address;

struct ioctl_data {
  unsigned long start_address;    //< Start virtual address of mmaped space
  unsigned long end_address;      //< End virtual address of mmaped space
  unsigned long *page_array;      //< Page array to be filled by the kernel
  size_t page_array_size;         //< Size of the page array
  unsigned long *num_dirty_pages;  //< Number of dirty pages
};

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

  printf("mapped_data = %p\n", mapped_data);
  start_address = (unsigned long) mapped_data;
  end_address = start_address + FILE_SIZE;
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
  create_dirty_pages(NUM_DIRTY_PAGES);

  int trace_fd = open("/dev/trace_dirty_pages", O_RDWR);
  if (trace_fd < 0) {
    perror("Failed to open /dev/virtual_address");
    exit(EXIT_FAILURE);
  }

  struct ioctl_data data;
  // Initialize the struct with data
  data.start_address = start_address;
  data.end_address = end_address;
  data.page_array_size = NUM_DIRTY_PAGES;
  data.page_array = (unsigned long *) calloc(data.page_array_size, sizeof(unsigned long));
  data.num_dirty_pages = (unsigned long *) calloc(1, sizeof(unsigned long));

  if (data.page_array == NULL) {
    perror("Failed to allocate memory for page array");
    close(trace_fd);
    exit(EXIT_FAILURE);
  }

  /* Request virtual addresses from the kernel */
  if (ioctl(trace_fd, TRACE_DIRTY_PAGES, &data) < 0) {
    perror("IOCTL VIRT_ADDR_GET failed");
    close(trace_fd);
    exit(EXIT_FAILURE);
  }

  // print filled arrray
  for (size_t i = 0; i < data.page_array_size && data.page_array[i] != 0; ++i) {
    //print unsigned long as address in hex
    printf("data.page_array[%lu] = 0x%lx\n", i, data.page_array[i]);
  }

  printf("data.num_dirty_pages = %lu\n", *data.num_dirty_pages);

  close(trace_fd); /* Close file */

  delete_file();
  return 0;
}
