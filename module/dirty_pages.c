#include <linux/module.h> /* Needed by all kernel modules */
#include <linux/kernel.h> /* Needed for KERN_INFO */

static int __init dirty_pages_init(void) {
  printk(KERN_INFO "Hello World\n");

  /* A non 0 return means init_module failed; module can not be loaded. */
  return 0;
}

static void __exit dirty_pages_exit (void) {
  printk(KERN_INFO "Unload module!\n");
}

module_init(dirty_pages_init);
module_exit(dirty_pages_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Iacovos G. Kolokasis");
MODULE_DESCRIPTION("A simple kernel module to print dirty pages in the page cache");
