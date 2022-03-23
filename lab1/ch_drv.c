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

static dev_t first;
static struct cdev c_dev;
static struct class *cl;

ssize_t input_char_len = 0;
char *input_buff;
static struct proc_dir_entry *entry;

static ssize_t proc_read(struct file *f, char __user *buf, size_t len,
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

static ssize_t proc_write(struct file *f, const char __user *buf, size_t len,
                          loff_t *off) {

  if (copy_from_user(input_buff, buf, len) != 0) {
    return -EFAULT;
  }
  input_char_len += len - 1;
  printk(KERN_DEBUG "Input characters amount: %ld\n", input_char_len);

  return len;
}

static ssize_t dev_read(struct file *f, char __user *buf, size_t len,
                        loff_t *off) {

  // TODO: read from /dev/var1

  printk(KERN_INFO "This is %s \n", __FUNCTION__);
  return 0;
}

static ssize_t dev_write(struct file *f, const char __user *buf, size_t len,
                         loff_t *off) {
  // TODO: write to /dev/var1

  printk(KERN_INFO "This is %s \n", __FUNCTION__);
  return len;
}

static struct file_operations proc_fops = {
    .owner = THIS_MODULE, .read = proc_read, .write = proc_write};

static struct file_operations mychdev_fops = {.read = dev_read,
                                              .write = dev_write};

static int mychardev_uevent(struct device *dev, struct kobj_uevent_env *env) {
  add_uevent_var(env, "DEVMODE=%#o", 0666);
  return 0;
}

static int __init ch_drv_init(void) {
  input_buff = (char *)kmalloc(1024, GFP_KERNEL);
  if (!input_buff) {
    printk(KERN_ERR "%s:impossible to allocate memory\n", THIS_MODULE->name);
    return -1;
  }
  input_char_len = 0;
  if (!memset(input_buff, 0, 1024)) {
    printk(KERN_ERR "%s:something went wrong\n", THIS_MODULE->name);
    return -1;
  }

  // TODO: - init dev/var1
  //       - alloc_chrdev_region
  //       - class create
  //       - device create
  //       - device init
  // |
  // | I guess it's done, but some review needed
  // v

  if (alloc_chrdev_region(&first, 0, 1, "ch_dev") < 0) {
    return -1;
  }
  if ((cl = class_create(THIS_MODULE, "chardrv")) == NULL) {
    unregister_chrdev_region(first, 1);
    return -1;
  }
  cl->dev_uevent = mychardev_uevent;
  if (device_create(cl, NULL, first, NULL, "var1") == NULL) {
    class_destroy(cl);
    unregister_chrdev_region(first, 1);
    return -1;
  }
  cdev_init(&c_dev, &mychdev_fops);
  if (cdev_add(&c_dev, first, 1) == -1) {
    device_destroy(cl, first);
    class_destroy(cl);
    unregister_chrdev_region(first, 1);
    return -1;
  }

  entry = proc_create("var1", 0666, NULL, &proc_fops);
  if (entry == NULL) {
    device_destroy(cl, first);
    class_destroy(cl);
    unregister_chrdev_region(first, 1);
    return -1;
  }

  printk(KERN_INFO "%s: dev file is created\n", THIS_MODULE->name);
  printk(KERN_INFO "%s: proc file is created\n", THIS_MODULE->name);
  return 0;
}

static void __exit ch_drv_exit(void) {

  // TODO: - del dev
  //       - device destroy
  //       - class destroy
  // |
  // | I guess it's done, but some review needed
  // v

  cdev_del(&c_dev);
  device_destroy(cl, first);
  class_destroy(cl);
  unregister_chrdev_region(first, 1);
  printk(KERN_INFO "%s: dev file is deleted\n", THIS_MODULE->name);

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
