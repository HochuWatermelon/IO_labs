#include <linux/module.h>  /* essential for modules' macros */
#include <linux/kernel.h>  /* essential for KERNEL_INFO */

#include <linux/proc_fs.h> /* essential for procfs */
#include <linux/slab.h>    /* essential for kmalloc, kfree */
#include <linux/cdev.h>    /* essential for dev_t */


MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("German");
MODULE_AUTHOR("Alexandr");

#define MOD_NAME "chr_comp"

#define VAR_NAME "var3"
#define DEV_NAME  "/dev/"  VAR_NAME
#define PROC_NAME "/proc/" VAR_NAME

#define OUTCOMES_LENGTH (5)

static int outcomes[OUTCOMES_LENGTH];
static size_t dev_idx = 0;
static size_t proc_idx = 0;

static bool dev_create(dev_t *first_dev_id, int major, int minor, u32 count, struct cdev *cdev, const char *name,
                       const struct file_operations *fops);

static bool dev_remove(struct cdev *cdev, dev_t *first_dev_id, u32 count);

static int dev_var3_open(struct inode *ptr_inode, struct file *ptr_file);

static ssize_t dev_var3_read(struct file *ptr_file, char __user *usr_buf, size_t length, loff_t *ptr_offset);

static ssize_t dev_var3_write(struct file *ptr_file, const char __user *usr_buf, size_t length, loff_t *ptr_offset);

static int dev_var3_release(struct inode *ptr_inode, struct file *ptr_file);


static dev_t dev_var3_first_device_id;
static u32 dev_var3_count = 1;
static int dev_var3_major = 410, dev_var3_minor = 0;
static struct cdev *dev_var3_cdev = NULL;
static const struct file_operations dev_var3_fops = {
        .owner   = THIS_MODULE,
        .open    = dev_var3_open,
        .read    = dev_var3_read,
        .write   = dev_var3_write,
        .release = dev_var3_release
};


static int proc_var3_open(struct inode *ptr_inode, struct file *ptr_file);

static ssize_t proc_var3_read(struct file *ptr_file, char __user *usr_buf, size_t length, loff_t *ptr_offset);

static ssize_t proc_var3_write(struct file *ptr_file, const char __user *usr_buf, size_t length, loff_t *ptr_offset);

static int proc_var3_release(struct inode *ptr_inode, struct file *ptr_file);

static struct proc_dir_entry *proc_var3_entry = NULL;
static const struct proc_ops proc_var3_ops = {
        .proc_open    = proc_var3_open,
        .proc_read    = proc_var3_read,
        .proc_write   = proc_var3_write,
        .proc_release = proc_var3_release
};


static int __init init_chr_comp(void) {
    printk(KERN_INFO ": module inited");

    proc_var3_entry = proc_create(VAR_NAME, 0444, NULL, &proc_var3_ops); // 0444 -> r--r--r--
    if (proc_var3_entry == NULL)
        return -EINVAL;

    if (dev_create(&dev_var3_first_device_id, dev_var3_major, dev_var3_minor, dev_var3_count, dev_var3_cdev, VAR_NAME,
                   &dev_var3_fops) == false)
        return -EINVAL;

    return 0;
}

static void __exit cleanup_chr_comp(void) {
    printk(KERN_INFO ": module cleaned up");

    proc_remove(proc_var3_entry);

    dev_remove(dev_var3_cdev, &dev_var3_first_device_id, dev_var3_count);
}

module_init(init_chr_comp);
module_exit(cleanup_chr_comp)

static int atoi(const char *string);;


static int proc_var3_open(struct inode *ptr_inode, struct file *ptr_file) {
    printk(KERN_INFO ": file " PROC_NAME " opened\n");
    return 0;
}

static ssize_t proc_var3_read(struct file *ptr_file, char __user *usr_buf, size_t length, loff_t *ptr_offset) {
    if (*ptr_offset > 0) return 0;
    *ptr_offset += length;

    printk(KERN_INFO ": proc_var3_read: %d\n", outcomes[proc_idx]);
    proc_idx = (proc_idx + 1) % OUTCOMES_LENGTH;
    return length;
}

static ssize_t proc_var3_write(struct file *ptr_file, const char __user *usr_buf, size_t length, loff_t *ptr_offset) {
    return -EINVAL;
}

static int proc_var3_release(struct inode *ptr_inode, struct file *ptr_file) {
    printk(KERN_INFO ": file " PROC_NAME " closed\n");
    return 0;
}


static bool dev_create(dev_t *first_dev_id, int major, int minor, u32 count, struct cdev *cdev, const char *name,
                       const struct file_operations *fops) {
    *first_dev_id = MKDEV(major, minor);
    if (register_chrdev_region(*first_dev_id, count, name)) {
        // unregister_chrdev_region( *first_dev_id, count ); // void can't check for the errors
        return false;
    }

    cdev = cdev_alloc();
    if (cdev == NULL) {
        unregister_chrdev_region(*first_dev_id, count); // void can't check for the errors
        return false;
    }

    cdev_init(cdev, fops);
    if (cdev_add(cdev, *first_dev_id, count) == -1) {
        unregister_chrdev_region(*first_dev_id, count); // void can't check for the errors
        cdev_del(cdev);
        return false;
    }

    return true;
}

static bool dev_remove(struct cdev *cdev, dev_t *first_dev_id, u32 count) {
    if (cdev)
        cdev_del(cdev); // void can't check for errors

    unregister_chrdev_region(*first_dev_id, count); // void can't check for the errors
    return true;
}

static int dev_var3_open(struct inode *ptr_inode, struct file *ptr_file) {
    printk(KERN_INFO ": file " DEV_NAME " opened\n");
    return 0;
}

static ssize_t dev_var3_read(struct file *ptr_file, char __user *usr_buf, size_t length, loff_t *ptr_offset) {
    // size_t count = 0;
    // if ( *ptr_offset > 0 ) return 0;

    // count = snprintf( usr_buf, length, "%d\n", outcomes[ dev_idx ] );
    // *ptr_offset += count;
    // dev_idx = ( dev_idx + 1 ) % OUTCOMES_LENGTH;
    // printk( KERN_INFO MODULE_NAME ": dev_var3_read: dev_idx = %zu, count = %zu, length = %zu\n", dev_idx, count, length );
    // return count;
    if (*ptr_offset > 0) return 0;
    *ptr_offset += length;

    printk(KERN_INFO ": dev_var3_read: %d\n", outcomes[(dev_idx - 1) % OUTCOMES_LENGTH]);
    dev_idx = (dev_idx + 1) % OUTCOMES_LENGTH;
    return length;
}

static ssize_t dev_var3_write(struct file *ptr_file, const char __user *usr_buf, size_t length, loff_t *ptr_offset) {
    int sum = 0;
    int last_pos_start = -1;
    char *cp = kmalloc(length * sizeof(char), GFP_USER);
    for (int j = 0; j < length; j++) {
        cp[j] = usr_buf[j];
    }
    printk(KERN_INFO ": dev_var3_write: length = %zu\n", length);

    for (int i = 0; i < length; i++) {
        if (usr_buf[i] >= '0' && usr_buf[i] <= '9' && last_pos_start == -1) {
            last_pos_start = i;
        }
        if ((usr_buf[i] < '0' || usr_buf[i] > '9') && last_pos_start != -1) {
            last_pos_start = -1;
            cp[i] = '\0';
            sum += atoi(cp + last_pos_start);
        }
    }
    outcomes[dev_idx] = sum;
    dev_idx = (dev_idx + 1) % OUTCOMES_LENGTH;
    return length;
}

static int atoi(const char *string) {
    int k = 0;
    while (*string) {
        k = (k << 3) + (k << 1) + (*p) - '0';
        string++;
    }
    return k;
}

static int dev_var3_release(struct inode *ptr_inode, struct file *ptr_file) {
    printk(KERN_INFO ": file " DEV_NAME " closed\n");
    return 0;
}