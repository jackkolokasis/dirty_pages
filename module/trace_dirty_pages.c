#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/pagemap.h>
  
unsigned long *my_pages = NULL;

/**
 * This line defines a unique magic number associated with the ioctl commands of
 * your module. A magic number is a unique identifier used to distinguish the
 * ioctl commands of a specific module from those of other modules. It's a good
 * practice to choose a character or number that is less likely to conflict with
 * magic numbers used by other modules.
 *
 * In this case, 'T' is chosen as the magic number for the virtual address
 * module. It's just a character to help uniquely identify ioctl commands
 * related to virtual addresses in your module.
 **/
#define TRACE_DIRTY_PAGES_MAGIC 'T'
/**
 * This line defines an ioctl command named VIRT_ADDR_GET using the _IOW macro.
 * The _IOW macro is a helper macro that constructs an ioctl command number
 * based on the provided parameters.
 */
#define TRACE_DIRTY_PAGES _IOW(TRACE_DIRTY_PAGES_MAGIC, 1, struct ioctl_data)
#define MY_MAX_MINORS  1

struct ioctl_data {
  unsigned long va_start;
  unsigned long va_end;
  unsigned long *pages;
  size_t array_size;
};

static dev_t dev_number;
static struct cdev c_dev;
static struct class *cl;            /* Global variable for the device class */

// This function is used to print dirty pages of a process
// va_start: starting virtual address
// va_end: ending virtual address
// return: 0 if success, -EFAULT if failed
static long print_dirty_pages(unsigned long va_start, unsigned long va_end,
                              size_t array_size, struct ioctl_data data) 
{
  struct address_space *mapping;
  struct file *file;
  struct page *page;
  struct vm_area_struct *vma;
  unsigned long starting_offset, ending_offset;
  //unsigned long i = 0;
  size_t i = 0;
  size_t count = 0;
  size_t num_pages = 0;

  memset(my_pages, 0, 512 * sizeof(unsigned long));

  // Need a read lock to have a consistent view of the vma area data structure
  mmap_read_lock(current->mm);
  // while loop in case of madvise multiple vmas
  vma = find_vma(current->mm, va_start);
  if (vma == NULL) {
    printk(KERN_INFO "find_vma_area: find_vma failed\n");
    return -EFAULT;
  }

  // Page cache
  file = vma->vm_file;
  mapping = file->f_mapping;

  // starting offset with linear_page_index
  starting_offset = linear_page_index(vma, vma->vm_start);
  ending_offset = linear_page_index(vma, vma->vm_end);

  XA_STATE(xas, &mapping->i_pages, starting_offset);

  rcu_read_lock();
  xas_for_each_marked(&xas, page, ending_offset, PAGECACHE_TAG_DIRTY) {
    unsigned long dirty_virtual_address = ((page->index - 0) << PAGE_SHIFT) + vma->vm_start;
    //printk(KERN_INFO "device offset: %lu\n", page->index);
    //printk(KERN_INFO "dirty virtual address: 0x%lx\n", dirty_virtual_address);
    //i++;
    if (num_pages == array_size) {
      rcu_read_unlock();
      mmap_read_unlock(current->mm);
      return -EFAULT;
    }

    if (i == 512) {
      if (copy_to_user(data.pages + (count * 512), my_pages, 512 * sizeof(unsigned long))) {
        printk(KERN_INFO "trace_dirty_pages_ioctl: copy_to_user failed\n");
        return -EFAULT;
      }
      printk(KERN_INFO "Here1 | NUM pages = %lu | Count = %ld\n", num_pages, count);
      memset(my_pages, 0, 512 * sizeof(unsigned long));
      i = 0;
      count++;
    }

    my_pages[i] = dirty_virtual_address;
    i++;
    num_pages++;
  }

  rcu_read_unlock();
  mmap_read_unlock(current->mm);

  //my_pages[0] = i;
    
  //if (copy_to_user(data.pages, my_pages, 512 * sizeof(unsigned long))) {
  //  printk(KERN_INFO "trace_dirty_pages_ioctl: copy_to_user failed\n");
  //  return -EFAULT;
  //}


  if (i != 0 && (count * 512 + 512) < array_size) {
    printk(KERN_INFO "Here | NUM pages = %lu\n", num_pages);
    if (copy_to_user(data.pages + (count * 512), my_pages, 512 * sizeof(unsigned long))) {
      printk(KERN_INFO "trace_dirty_pages_ioctl: copy_to_user failed\n");
      return -EFAULT;
    }
  }

  return 0;
}

static long trace_dirty_pages_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
  struct ioctl_data data;

  switch(cmd) {
    case TRACE_DIRTY_PAGES:
      if (copy_from_user(&data, (unsigned long *)arg, sizeof(struct ioctl_data))) {
        printk(KERN_INFO "trace_dirty_pages_ioctl: copy_from_user failed\n");
        return -EFAULT;
      }
      
      if (data.array_size == 0 || data.array_size > SIZE_MAX / sizeof(unsigned long)) {
        printk(KERN_INFO "trace_dirty_pages_ioctl: array_size is 0\n");
        return -EFAULT;
      }

      //printk(KERN_INFO "trace_dirty_pages_ioctl: va_start: %lu\n", data.va_start);
      //printk(KERN_INFO "trace_dirty_pages_ioctl: va_end: %lu\n", data.va_end);
      //printk(KERN_INFO "trace_dirty_pages_ioctl: array_size: %lu\n", data.array_size);

      if (print_dirty_pages(data.va_start, data.va_end, data.array_size, data)) {
        printk(KERN_INFO "trace_dirty_pages_ioctl: print_dirty_pages failed\n");
        // free the allocated memory
        //kfree(my_pages);
        return -EFAULT;
      }

      //// copy the array to user space
      //if (copy_to_user(data.pages, my_pages, data.array_size * sizeof(unsigned long))) {
      //  // free the allocated memory
      //  kfree(my_pages);
      //  printk(KERN_INFO "trace_dirty_pages_ioctl: copy_to_user failed\n");
      //  return -EFAULT;
      //}

      // free the allocated memory
      //kfree(my_pages);

      break;
    default:
      return -ENOTTY;
  }
  return 0;
}

static struct file_operations fops = {
  .owner = THIS_MODULE,
  .unlocked_ioctl = trace_dirty_pages_ioctl,
};

int trace_dirty_pages_init(void)
{
  int err;

  err = alloc_chrdev_region(&dev_number, 0, MY_MAX_MINORS, "trace_dirty_pages");
  if (err != 0) {
    printk(KERN_INFO "trace_dirty_pages_init: register_chrdev_region failed with %d\n", err);
    return err;
  }

  if ((cl = class_create(THIS_MODULE, "chardev")) == NULL) {
    printk(KERN_INFO "trace_dirty_pages_init: class_create failed\n");
    unregister_chrdev_region(dev_number, MY_MAX_MINORS);
    return -1;
  }

  if (device_create(cl, NULL, dev_number, NULL, "trace_dirty_pages") == NULL) {
    printk(KERN_INFO "trace_dirty_pages_init: device_create failed\n");
    class_destroy(cl);
    unregister_chrdev_region(dev_number, MY_MAX_MINORS);
    return -1;
  }

  cdev_init(&c_dev, &fops);

  if (cdev_add(&c_dev, dev_number, MY_MAX_MINORS) == -1) {
    printk(KERN_INFO "trace_dirty_pages_init: cdev_add failed\n");
    device_destroy(cl, dev_number);
    class_destroy(cl);
    unregister_chrdev_region(dev_number, MY_MAX_MINORS);
    return -1;
  }

  // We allocate an array of 512 indices which indicates 512 pages
  // 512 * 8 bytes = 4096 = 4KB = 1 physical page
  my_pages = kcalloc(512, sizeof(unsigned long), GFP_KERNEL);

  if (my_pages == NULL) {
    printk(KERN_INFO "trace_dirty_pages_ioctl: kcalloc failed\n");
    return -EFAULT;
  }

  printk(KERN_INFO "trace_dirty_pages_init: Module has been loaded!\n");
  return 0;
}

void trace_dirty_pages_exit(void)
{
  // free the allocated memory
  kfree(my_pages);
  cdev_del(&c_dev);
  device_destroy(cl, dev_number);
  class_destroy(cl);
  unregister_chrdev_region(dev_number, MY_MAX_MINORS);

  printk(KERN_INFO "trace_dirty_pages_exit: Module has been removed!\n");
}

module_init(trace_dirty_pages_init);
module_exit(trace_dirty_pages_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Iacovos Kolokasis");
MODULE_DESCRIPTION("A kernel module that trace dirty pages");
