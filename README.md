# Trace dirty pages in Linux kernel page cache

Monitor and report dirty pages in Linux Kernel for a specific process. We
provide two implementations:

1. Reading maps, pagemaps, and kpageflags files from sysfs 
2. Linux kernel module that read the page cache for the specific file. This
   implementation is for Linux kernel v.6.1.

## Build example that uses sysfs
```
cd sysfs
make
./example <filename> <size> <num_dirty_pages>
```

## Build kernel module
```
cd module
# Edit the version of the kernel inside the Makefile
# Edit the username for /dev/trace_dirty_pages
# You need sudo permissions
make
```

## Test the kernel module
```
cd tests
# Edit the username for /dev/trace_dirty_pages
# Replace the path /mnt/fmap/file.txt with your own path
make
./example <filename> <size> <num_dirty_pages>
```
