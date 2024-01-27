#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cstdlib>
#include <string.h>
#include "dirtyPages.hpp"

int fd = 0;
char* mapped_data = NULL;

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

int main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("Usage: %s <filename> <size> <num_dirty_pages>\n", argv[0]);
    printf("Example: %s /mnt/fmap/file.txt 2G 10\n", argv[0]);
    return 1;
  }

  char *file_name  = strdup(argv[1]);
  long long file_size = convertToBytes(argv[2]);
  int num_dirty_pages = atoi(argv[3]);

  create_file(file_name, file_size);
  DirtyPages *dpages = new DirtyPages();

  create_dirty_pages(num_dirty_pages);
  dpages->print_dirty_pages();

  delete_file(file_size);
  return 0;
}
