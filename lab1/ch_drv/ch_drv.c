#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>

static dev_t first;
static struct cdev c_dev; 
static struct class *cl;

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
  printk(KERN_INFO "Driver: open()\n");
  return 0;
}

static int my_close(struct inode *i, struct file *f)
{
  printk(KERN_INFO "Driver: close()\n");
  return 0;
}

static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
  curr = list_head;
  while (curr != NULL) {
  	printk(KERN_INFO "%d, ", curr->value);
  	curr = curr->next;
  }
  printk(KERN_INFO "Driver: read()\n");
  return 0;
}

static ssize_t my_write(struct file *f, const char __user *buf,  size_t len, loff_t *off)
{
  list_head = push_front(list_head, len);
  printk(KERN_INFO "Driver: write()\n");
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

static int __init ch_drv_init(void)
{
    printk(KERN_INFO "Hello!\n");
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

    if (device_create(cl, NULL, first, NULL, "mychdev") == NULL)
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
 
static void __exit ch_drv_exit(void)
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
    printk(KERN_INFO "Bye!!!\n");
}
 
module_init(ch_drv_init);
module_exit(ch_drv_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Author");
MODULE_DESCRIPTION("The first kernel module");

