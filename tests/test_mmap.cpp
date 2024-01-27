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
#define PAGE_SIZE 4096

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

long long convertToBytes(char *sizeStr) {
  long long multiplier = 1;
  size_t len = strlen(sizeStr);
  char unit = sizeStr[len - 1];

  switch (unit) {
    case 'G':
    case 'g':
      multiplier *= 1024 * 1024 * 1024;
      break;
    case 'M':
    case 'm':
      multiplier *= 1024 * 1024;
      break;
    case 'K':
    case 'k':
      multiplier *= 1024;
      break;
    default:
      break;
  }

  sizeStr[len - 1] = '\0'; // Remove the unit from the string
  return atoll(sizeStr) * multiplier;
}

void create_file(char *file_name, long long file_size) {
  // Create a file and open it for read and write
  fd = open(file_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    std::cerr << "Error opening file." << std::endl;
    exit(EXIT_FAILURE);
  }

  // Allocate space for the file using posix_fallocate
  if (posix_fallocate(fd, 0, file_size) != 0) {
    std::cerr << "Error allocating space for the file." << std::endl;
    close(fd);
    exit(EXIT_FAILURE);
  }

  // Map the file into memory
  mapped_data = (char *) mmap(nullptr, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (mapped_data == MAP_FAILED) {
    std::cerr << "Error mapping file into memory." << std::endl;
    close(fd);
    exit(EXIT_FAILURE);
  }

  printf("mapped_data = %p\n", mapped_data);
  start_address = (unsigned long) mapped_data;
  end_address = start_address + file_size;
}

void delete_file(long long file_size) {
  // Unmap the file from memory
  if (munmap(mapped_data, file_size) == -1) {
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

int main(int argc, char* argv[]) {
  if (argc != 4) {
    printf("Usage: %s <filename> <size> <num_dirty_pages>\n", argv[0]);
    printf("Example: %s /mnt/fmap/file.txt 2G 10\n", argv[0]);
    return 1;
  }
  
  char *file_name  = strdup(argv[1]);
  long long file_size = convertToBytes(argv[2]);
  int num_dirty_pages = atoi(argv[3]);

  create_file(file_name, file_size);
  create_dirty_pages(num_dirty_pages);

  int trace_fd = open("/dev/trace_dirty_pages", O_RDWR);
  if (trace_fd < 0) {
    perror("Failed to open /dev/virtual_address");
    exit(EXIT_FAILURE);
  }

  struct ioctl_data data;
  // Initialize the struct with data
  data.start_address = start_address;
  data.end_address = end_address;
  data.page_array_size = num_dirty_pages;
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

  delete_file(file_size);
  return 0;
}
