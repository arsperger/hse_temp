#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/timer.h>
#include <linux/mutex.h>
#include <linux/sched/signal.h>

#define DEVICE_NAME "watchdog_timer"

static int timeout = 5;
static int signal_num = SIGTERM;

module_param(timeout, int, 0644);
module_param(signal_num, int, 0644);

MODULE_PARM_DESC(timeout, "timeout");
MODULE_PARM_DESC(signal_num, "send signal");

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("simple watchdog timer");

static dev_t dev_number;
static pid_t watchdog_pid = -1;

static struct cdev watchdog_cdev;
static struct class *watchdog_class;
static struct timer_list watchdog_timer;
static struct task_struct *watchdog_task = NULL;

static DEFINE_MUTEX(watchdog_mtx);


// timer cb
// TODO: handle multiple processes in a linked-list
static void watchdog_timer_callback(struct timer_list *timer) {
    pr_info("watchdog timer expired\n");
    mutex_lock(&watchdog_mtx);
    if (watchdog_task) {
        pr_info("send signal %d to pid:%d\n", signal_num, watchdog_pid);
        send_sig(signal_num, watchdog_task, 0);
    }
    mutex_unlock(&watchdog_mtx);
}

// jiffy - The time unit of Linux kernel. One jiffy equals (1/HZ) second.
// 2*HZ      2 seconds in jiffies
// HZ        1 second in jiffies
// foo * HZ  foo seconds in jiffies
// HZ/10     100 milliseconds in jiffies
// HZ/100    10 milliseconds in jiffies
// bar*HZ/1000  bar milliseconds in jiffies
static int watchdog_open(struct inode *inode, struct file *file) {
    pr_info("device opened\n");
    mutex_lock(&watchdog_mtx);
    watchdog_task = current;
    watchdog_pid = current->pid;
    mod_timer(&watchdog_timer, jiffies + timeout * HZ);
    mutex_unlock(&watchdog_mtx);
    return 0;
}

static int watchdog_release(struct inode *inode, struct file *file) {
    pr_info("device closed\n");
    mutex_lock(&watchdog_mtx);
    watchdog_task = NULL;
    watchdog_pid = -1;
    mutex_unlock(&watchdog_mtx);
    return 0;
}

static ssize_t watchdog_write(struct file *file, const char __user *buf, size_t len, loff_t *off) {
    pr_info("device write\n");
    mutex_lock(&watchdog_mtx);
    mod_timer(&watchdog_timer, jiffies + timeout * HZ);
    mutex_unlock(&watchdog_mtx);
    return len;
}

// TODO: implement watchdog_read
/*
static ssize_t watchdog_read(struct file *file, char __user *buf, size_t len, loff_t *off) {
    pr_info("device read\n");
    mutex_lock(&watchdog_mtx);
    mod_timer(&watchdog_timer, jiffies + timeout * HZ);
    mutex_unlock(&watchdog_mtx);
    return len;
}
*/

// file ops
static struct file_operations watchdog_fops = {
    .owner = THIS_MODULE,
    .open = watchdog_open,
    .release = watchdog_release,
    .write = watchdog_write,
    //.read = watchdog_read, // temp. excluded
    // TODO:
    // .ioctl = watchdog_ioctl,
    // .llseek = watchdog_llseek,
};

static int __init watchdog_init(void) {
    int ret;

    pr_info("initializing...\n");

    ret = alloc_chrdev_region(&dev_number, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        pr_err("error alloc_chrdev_region\n");
        return ret;
    }

    cdev_init(&watchdog_cdev, &watchdog_fops);
    ret = cdev_add(&watchdog_cdev, dev_number, 1);
    if (ret < 0) {
        pr_err("error cdev_add\n");
        unregister_chrdev_region(dev_number, 1);
        return ret;
    }

    watchdog_class = class_create(THIS_MODULE, DEVICE_NAME);
    if (IS_ERR(watchdog_class)) {
        pr_err("error class_create\n");
        cdev_del(&watchdog_cdev);
        unregister_chrdev_region(dev_number, 1);
        return PTR_ERR(watchdog_class);
    }

    if (!device_create(watchdog_class, NULL, dev_number, NULL, DEVICE_NAME)) {
        pr_err("error device_create\n");
        class_destroy(watchdog_class);
        cdev_del(&watchdog_cdev);
        unregister_chrdev_region(dev_number, 1);
        return -EINVAL;
    }

    timer_setup(&watchdog_timer, watchdog_timer_callback, 0);

    pr_info("watchdog initialized\n");
    return 0;
}

static void __exit watchdog_exit(void) {
    pr_info("watchdog exiting...\n");

    del_timer_sync(&watchdog_timer);
    device_destroy(watchdog_class, dev_number);
    class_destroy(watchdog_class);
    cdev_del(&watchdog_cdev);
    unregister_chrdev_region(dev_number, 1);

    pr_info("watchdog exited\n");
}

module_init(watchdog_init);
module_exit(watchdog_exit);


