#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cstdlib>

#define FILE_NAME "/mnt/fmap/file.txt"
#define FILE_SIZE (10 * 1024 * 1024 * 1024LU)

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

int main() {
  create_file();
  // create a new file
  delete_file();
  return 0;
}

