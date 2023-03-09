#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>


static dev_t first;
static struct cdev c_dev; 
static struct class *cl;
#define BUF_SIZE 100
static char buffer[BUF_SIZE];

struct ListNode {
  int value;
  struct ListNode* next;
};

struct ListNode* push_front(struct ListNode* head, int value) {
  struct ListNode* new_node = (struct ListNode*) kmalloc(sizeof(struct ListNode), GFP_KERNEL);
  new_node->value = value;
  new_node->next = head;
  return new_node;
}

static struct ListNode* list_head = NULL;
static struct ListNode* curr = NULL; 


static int my_open(struct inode *i, struct file *f)
{
  printk(KERN_INFO "ch_drv Driver: open()\n");
  return 0;
}

static int my_close(struct inode *i, struct file *f)
{
  printk(KERN_INFO "ch_drv Driver: close()\n");
  return 0;
}

static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
  curr = list_head;
  while (curr != NULL) {
  	printk(KERN_INFO "%d, ", curr->value);
  	curr = curr->next;
  }
  printk(KERN_INFO "ch_drv Driver: read()\n");

  int sz = strlen(buffer);
  if (*off > 0 || len < sz)
  {
	return 0;
  }
  if (copy_to_user(buf, buffer, sz) != 0)
  {
	return -EFAULT;
  }

  *off = sz;
  return sz;
}

static ssize_t my_write(struct file *f, const char __user *buf,  size_t len, loff_t *off)
{
  if (len >= BUF_SIZE) {
    return -EFAULT;
  }
  if (copy_from_user(buffer, buf, len) != 0) {
    return -EFAULT;
  }
  buffer[len] = 0;
  list_head = push_front(list_head, len);
  printk(KERN_INFO "ch_drv Driver: write()\n");
  return len;
}

static int my_dev_uevent(struct device *dev, struct kobj_uevent_env *env)
{
    add_uevent_var(env, "DEVMODE=%#o", 0666);
    return 0;
}

static struct file_operations mychdev_fops =
{
  .owner      = THIS_MODULE,
  .open       = my_open,
  .release    = my_close,
  .read       = my_read,
  .write      = my_write
};

static int ch_drv_init(void)
{
    printk(KERN_INFO "ch_drv Device: init()\n");
    if (alloc_chrdev_region(&first, 0, 1, "ch_dev") < 0)
	  {
		return -1;
	  }
    if ((cl = class_create(THIS_MODULE, "chardrv")) == NULL)
	  {
		unregister_chrdev_region(first, 1);
		return -1;
	  }

    cl->dev_uevent = my_dev_uevent;

    if (device_create(cl, NULL, first, NULL, "var1") == NULL)
	  {
		class_destroy(cl);
		unregister_chrdev_region(first, 1);
		return -1;
	  }
    cdev_init(&c_dev, &mychdev_fops);
    if (cdev_add(&c_dev, first, 1) == -1)
	  {
		device_destroy(cl, first);
		class_destroy(cl);
		unregister_chrdev_region(first, 1);
		return -1;
	  }
    return 0;
}
 
static void ch_drv_exit(void)
{
    cdev_del(&c_dev);
    device_destroy(cl, first);
    class_destroy(cl);
    unregister_chrdev_region(first, 1);
    while (list_head != NULL) {
    	curr = list_head;
    	list_head = list_head->next;
    	kfree(curr);
    }
    printk(KERN_INFO "ch_drv Device: exit()\n");
}

static struct proc_dir_entry* entry;
#define PROC_BUF_SIZE 1024
static char proc_buf[PROC_BUF_SIZE];

static ssize_t proc_write(struct file *file, const char __user * ubuf, size_t count, loff_t* ppos)
{
	printk(KERN_DEBUG "ch_drv Attempt to write proc file");
	return -1;
}

static ssize_t proc_read(struct file *file, char __user * ubuf, size_t count, loff_t* ppos)
{
    int i = 0;	
    for (i = 0; i < PROC_BUF_SIZE; i++) {
      proc_buf[i] = 0;
    }

    curr = list_head;
    while (curr != NULL) {
      char mini_buf[10];
      sprintf(mini_buf, "%d, ", curr->value);
      strcat(proc_buf, mini_buf);
      curr = curr->next;
    }
    printk(KERN_INFO "ch_drv Proc: read()\n");

	size_t len = strlen(proc_buf);
	if (*ppos > 0 || count < len)
	{
		return 0;
	}
	if (copy_to_user(ubuf, proc_buf, len) != 0)
	{
		return -EFAULT;
	}
	*ppos = len;
	return len;
}

static struct proc_ops fops = {
	.proc_read = proc_read,
	.proc_write = proc_write,
};


static int proc_example_init(void)
{
	entry = proc_create("var1", 0444, NULL, &fops);
	printk(KERN_INFO "ch_drv Proc: init()\n");
	return 0;
}

static void proc_example_exit(void)
{
	proc_remove(entry);
	printk(KERN_INFO "ch_drv Proc: exit()\n");
}


static int __init drv_init(void) {
  return ch_drv_init() || proc_example_init();
}

static void __exit drv_exit(void) {
  ch_drv_exit();
  proc_example_exit();
}
 
module_init(drv_init);
module_exit(drv_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Selfofly");
MODULE_DESCRIPTION("Oblepikha");

