#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/cdev.h>
static dev_t first;
static struct cdev c_dev; 
static struct class * cl;
static int* numbers;
static size_t numbers_sz;


static int my_open(struct inode *i, struct file *f)
{
  printk(KERN_INFO "Driver: open()\n");
  return 0;
}

static int my_close(struct inode *i, struct file *f)
{
  printk(KERN_INFO "Driver: close()\n");
  return 0;
}

static int copy_to_user(char __user *buf, int count) {
  printk(KERN_INFO "Driver: copy_to_user()\n");
  int j = 0;
  void* i;
  for (i = buf; j < count; j++) {
    i += sprintf(i, "%d, ", numbers[j]);
  }
  i += sprintf(i, "\n");
  *((char*)i) = 0;
  printk(KERN_INFO "RESULTING STR: %d\n", numbers_sz);
  return 0;
}

static int get_sum(char* buf, int count) {
  int curpowten = 1;
  int sum = 0;
  char* i;
  for (i = buf+count-1; i >= buf; i--){
    if (*i >= '0' && *i <= '9'){
      sum += ((int)(*i - '0')) * curpowten;
      curpowten *= 10;
    } else{
      curpowten = 1;
    }
  }
  return sum;
}

static int copy_from_user(char __user *buf, int count) {
  printk(KERN_INFO "Driver: copy_from_user()\n");
  int sum = get_sum(buf, count);
  numbers = krealloc(numbers, sizeof(int)*(++numbers_sz), 0);
  if (!numbers) {
    printk(KERN_INFO "Driver: cant realloc numbers array!");
    return 1;
  }
  numbers[numbers_sz-1] = sum;
  
  return 0;
}


static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
  printk(KERN_INFO "Driver: read()\n");
  
  if (*off > 0 || len < numbers_sz) {
      return 0;
  }
  printk(KERN_INFO "Driver: 1\n");

  if (copy_to_user(buf, numbers_sz) != 0) {
      return -EFAULT;
  }
  printk(KERN_INFO "Driver: 2\n");
  
  int l = strlen(buf);
  *off = l;
  
  return l;
}

static ssize_t my_write(struct file *f, const char __user *buf,  size_t len, loff_t *off)
{
  printk(KERN_INFO "Driver: write()\n");
  
  if (copy_from_user(buf, len) != 0) {
      return -EFAULT;
  }
  
  return len;
}

static struct file_operations mychdev_fops =
{
  .owner = THIS_MODULE,
  .open = my_open,
  .release = my_close,
  .read = my_read,
  .write = my_write
};
 
static int __init ch_drv_init(void)
{
    printk(KERN_INFO "Hello new!\n");
    numbers_sz = 0;
    numbers = 0;
    if (alloc_chrdev_region(&first, 0, 1, "ch_dev") < 0)
	  {
		return -1;
	  }
    if ((cl = class_create(THIS_MODULE, "chardrv")) == NULL)
	  {
		unregister_chrdev_region(first, 1);
		return -1;
	  }
      
      if (device_create(cl, NULL, first, NULL, "var3") == NULL)
      {
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
    return 0;
}
 
static void __exit ch_drv_exit(void)
{
    cdev_del(&c_dev);
    device_destroy(cl, first);
    class_destroy(cl);
    unregister_chrdev_region(first, 1);
    if (numbers)
      kfree(numbers);
    printk(KERN_INFO "Bye!!!\n");
}
 
module_init(ch_drv_init);
module_exit(ch_drv_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vlad & Ivan");
MODULE_DESCRIPTION("IO Lab 1");

