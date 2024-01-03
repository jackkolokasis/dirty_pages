#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

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
#define TRACE_DIRTY_PAGES _IOW(TRACE_DIRTY_PAGES_MAGIC, 1, unsigned long[2])
#define MY_MAX_MINORS  1

static dev_t dev_number;
static struct cdev c_dev;
static struct class *cl;                    /* Global variable for the device class */

static long trace_dirty_pages_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
  unsigned long va_array[2];
  printk(KERN_INFO "trace_dirty_pages_ioctl\n");

  switch(cmd) {
    case TRACE_DIRTY_PAGES:
      if (copy_from_user(va_array, (unsigned long *)arg, sizeof(va_array))) {
        printk(KERN_INFO "trace_dirty_pages_ioctl: copy_from_user failed\n");
        return -EFAULT;
      }

      printk(KERN_INFO "trace_dirty_pages_ioctl: %lu %lu\n", va_array[0], va_array[1]);
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

  if ((cl = class_create("chardev")) == NULL) {
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

  return 0;
}

void trace_dirty_pages_exit(void)
{
  // unload kernel module
  cdev_del(&c_dev);
  device_destroy(cl, dev_number);
  class_destroy(cl);
  unregister_chrdev_region(dev_number, MY_MAX_MINORS);

  printk(KERN_INFO "trace_dirty_pages_exit\n");
}

module_init(trace_dirty_pages_init);
module_exit(trace_dirty_pages_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Iacovos Kolokasis");
MODULE_DESCRIPTION("A kernel module that trace dirty pages");
