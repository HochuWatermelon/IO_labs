#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("German, Alexandr");
MODULE_DESCRIPTION("Sum of ints in string");
MODULE_VERSION("0.1");

int atoi(const char* S) {
    int num = 0;
    int i = 0;

    while (S[i] && (S[i] >= '0' && S[i] <= '9')) {
        num = num * 10 + (S[i] - '0');
        i++;
    }
    return num;
}

static struct proc_dir_entry *proc_entry;

const size_t mem_size = 1024;
dev_t dev = 0;
static struct class *dev_class;
static struct cdev dev_cdev;
uint8_t *input_buffer;
uint8_t *result_str_buffer;

// Хранилище последних результатов
#define RESULTS_BUF_SIZE 5
int results[RESULTS_BUF_SIZE];
int result_sp = 0;

static ssize_t proc_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos) {
    return -1;
}

static ssize_t proc_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos) {
    int i, offset = 0;

    for (i = 0; i < RESULTS_BUF_SIZE; i++)
        if (results[i] != INT_MIN)
            offset += sprintf(result_str_buffer + offset, "result #num=%d #value=%d\n", i, results[i]);

    if (*ppos > 0 || count < offset)
        return 0;

    if (copy_to_user(ubuf, result_str_buffer, offset) != 0)
        return -EFAULT;

    *ppos = offset;
    return offset;
}

static struct proc_ops fops = {
        .proc_read = proc_read,
        .proc_write = proc_write,
};



static int dev_open(struct inode *inode, struct file *file) {
    return 0;
}

static int dev_release(struct inode *inode, struct file *file) {
    return 0;
}

static ssize_t dev_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {
    int i, offset = 0;

    for (i = 0; i < RESULTS_BUF_SIZE; i++)
        if (results[i] != INT_MIN)
            offset += sprintf(result_str_buffer + offset, "result #num=%d #value=%d\n", i, results[i]);

    if (*off > 0 || len < offset)
        return 0;

    if (copy_to_user(buf, result_str_buffer, offset) != 0)
        return -EFAULT;
    *off = offset;
    return offset;
}

static ssize_t dev_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
    size_t i;
    int sum = 0;
    int last_pos_start = -1;

    if (copy_from_user(input_buffer, buf, len)) {
        pr_err("Data Write : Err!\n");
    }

    for (i = 0; i < len; i++) {
        if (input_buffer[i] >= '0' && input_buffer[i] <= '9' && last_pos_start == -1) {
            last_pos_start = i;
        }
        if ((input_buffer[i] < '0' || input_buffer[i] > '9') && last_pos_start != -1) {
            input_buffer[i] = '\0';
            sum += atoi(input_buffer + last_pos_start);
            last_pos_start = -1;
        }
    }
    pr_info("Data wrote: result %d!\n", sum);
    if (result_sp == RESULTS_BUF_SIZE) {
        for (i = 0; i < RESULTS_BUF_SIZE - 1; i++) {
            results[i] = results[i + 1];
        }
        results[RESULTS_BUF_SIZE - 1] = sum;
        result_sp--;
    } else {
        results[result_sp++] = sum;
    }
    return len;
}

static struct file_operations devfops = {
        .owner      = THIS_MODULE,
        .read       = dev_read,
        .write      = dev_write,
        .open       = dev_open,
        .release    = dev_release,
};


static int __init proc_example_init(void) {
    int i;
    proc_entry = proc_create("var3", 0444, NULL, &fops);

    for (i = 0; i < RESULTS_BUF_SIZE; i++)
        results[i] = INT_MIN;

    if ((alloc_chrdev_region(&dev, 0, 1, "var3_Dev")) < 0) {
        pr_err("Cannot allocate major number\n");
        return -1;
    }
    pr_info("Major = %d Minor = %d \n", MAJOR(dev), MINOR(dev));
    cdev_init(&dev_cdev, &devfops);
    if ((cdev_add(&dev_cdev, dev, 1)) < 0) {
        pr_err("Cannot add the device to the system\n");
        goto r_class;
    }
    if ((dev_class = class_create(THIS_MODULE, "var_class")) == NULL) {
        pr_err("Cannot create the struct class\n");
        goto r_class;
    }
    if ((device_create(dev_class, NULL, dev, NULL, "var3")) == NULL) {
        pr_err("Cannot create the Device 1\n");
        goto r_device;
    }
    if ((input_buffer = (uint8_t *) kmalloc(mem_size, GFP_KERNEL)) == 0) {
        pr_info("Cannot allocate memory in kernel\n");
        goto r_device;
    }
    if ((result_str_buffer = (uint8_t *) kmalloc(mem_size, GFP_KERNEL)) == 0) {
        pr_info("Cannot allocate memory in kernel\n");
        goto r_device;
    }
    return 0;

    r_device:
        class_destroy(dev_class);
    r_class:
        unregister_chrdev_region(dev, 1);
    return -1;
}

static void __exit proc_example_exit(void) {
    proc_remove(proc_entry);
    device_destroy(dev_class, dev);
    class_destroy(dev_class);
    cdev_del(&dev_cdev);
    unregister_chrdev_region(dev, 1);
}

module_init(proc_example_init);
module_exit(proc_example_exit);