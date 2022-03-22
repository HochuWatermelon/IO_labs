#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/uaccess.h>

ssize_t input_char_len = 0;
char *input_buff;
static struct proc_dir_entry *entry;

static ssize_t my_read(struct file *f, char __user *buf, size_t len,
                       loff_t *off) {

  int count = strlen(input_buff);
  if (*off > 0 || len < count) {
    return 0;
  }

  if (copy_to_user(buf, input_buff, input_char_len) != 0) {
    return -EFAULT;
  }

  printk(KERN_DEBUG "Characters from /proc/var1: %s\n", input_buff);
  *off = count;
  return count;
}

static ssize_t my_write(struct file *f, const char __user *buf, size_t len,
                        loff_t *off) {

  copy_from_user(input_buff, buf, len);
  input_char_len += len - 1;
  printk(KERN_DEBUG "Input characters amount: %ld\n", input_char_len);

  return len;
}

static struct file_operations mychdev_fops = {
    .owner = THIS_MODULE,
    .read = my_read,
    .write = my_write
};

static int __init ch_drv_init(void) {
  input_buff = (char *)kmalloc(1024, GFP_KERNEL);
  input_char_len = 0;
  memset(input_buff, 0, 1024);

  entry = proc_create("var1", 0666, NULL, &mychdev_fops);
  printk(KERN_INFO "%s: proc file is created\n", THIS_MODULE->name);
  return 0;
}

static void __exit ch_drv_exit(void) {
  kfree(input_buff);
  proc_remove(entry);
  printk(KERN_INFO "%s: proc file is deleted\n", THIS_MODULE->name);
  printk(KERN_INFO "Bye!!!\n");
}

module_init(ch_drv_init);
module_exit(ch_drv_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Danila Gorelko, Boris Novoselov");
MODULE_DESCRIPTION("The first kernel module");
