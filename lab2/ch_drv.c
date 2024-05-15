#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>


#include <linux/slab.h>
#include <linux/string.h>

#define MAX_BUFFER_SIZE 1024

static dev_t first;
static struct cdev c_dev; 
static struct class *cl;
static char buffer[MAX_BUFFER_SIZE];
static int buffer_size = 0;
static int num_characters_written = 0; // Variable to store the count of characters written

#define MAX_SIZE 100 // Максимальное количество строк


struct StringArray {
    char* strings[MAX_SIZE];
    int count;
};

struct StringArray arr;

void initializeStringArray(struct StringArray *arr) {
    arr->count = 0;
}



void addString(struct StringArray *arr, const char* str) {
    if (arr->count < MAX_SIZE) {
        arr->strings[arr->count] = kmalloc(strlen(str) + 1, GFP_KERNEL);
        if (arr->strings[arr->count] == NULL) {
            printk(KERN_ERR "Ошибка выделения памяти\n");
            return;
        }
        strcpy(arr->strings[arr->count], str);
        arr->count++;
    } else {
        printk(KERN_ERR "Превышено максимальное количество строк\n");
    }
}

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
    ssize_t bytes = 0; 

    if (*off >= buffer_size && *off >= arr.count)
        return 0;

    if (*off < buffer_size) {
        size_t to_copy = min(buffer_size - *off, len); 
        if (copy_to_user(buf, buffer + *off, to_copy)) {
            return -EFAULT;
        }
        buf += to_copy;
        bytes += to_copy;
        len -= to_copy;
        *off += to_copy;
    }

    
    if (len > 0 && *off >= buffer_size) {
      
        int index = *off - buffer_size;
        for (; index < arr.count && len > 0; index++) {
            size_t str_len = strlen(arr.strings[index]) + 1; 
            if (str_len > len)
                break;
            if (copy_to_user(buf, arr.strings[index], str_len)) {
                return -EFAULT; 
            }
            buf += str_len;
            bytes += str_len;
            len -= str_len;
            *off += str_len;
        }
    }

    return bytes; 
}

static ssize_t my_write(struct file *f, const char __user *buf, size_t len, loff_t *off) {
    printk(KERN_INFO "Driver: write()\n");

    if (buffer_size + len > MAX_BUFFER_SIZE)
        return -ENOSPC;

    if (copy_from_user(buffer + buffer_size, buf, len) != 0)
        return -EFAULT;

    buffer_size += len;
    num_characters_written += len; 


    char temp_str[100];
    snprintf(temp_str, 100, "Number of characters written: %d\n", len);
    addString(&arr, temp_str);

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
    initializeStringArray(&arr);
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
    if (device_create(cl, NULL, first, NULL, "var5") == NULL)
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
    printk(KERN_INFO "Bye!!!\n");
}
 
module_init(ch_drv_init);
module_exit(ch_drv_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Author");
MODULE_DESCRIPTION("The first kernel module");
