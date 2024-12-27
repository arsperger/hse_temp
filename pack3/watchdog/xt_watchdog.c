#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/timer.h>
#include <linux/mutex.h>
#include <linux/sched/signal.h>
#include <linux/slab.h>
#include <linux/list.h>

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
static struct cdev watchdog_cdev;
static struct class *watchdog_class;

static DEFINE_MUTEX(watchdog_mtx);
static LIST_HEAD(process_list);

struct process_entry {
    pid_t pid;
    struct task_struct *task;
    struct timer_list timer;
    struct list_head list;
};

// timer cb
static void watchdog_timer_callback(struct timer_list *t) {
    struct process_entry *entry = from_timer(entry, t, timer);

    pr_info("watchdog timer expired\n");

    mutex_lock(&watchdog_mtx);

    if (entry->task) {
        pr_info("Sending signal %d to PID %d\n", signal_num, entry->pid);
        send_sig(signal_num, entry->task, 0);
    }

    mutex_unlock(&watchdog_mtx);
}

// jiffy - The time unit of Linux kernel. One jiffy equals (1/HZ) second.
static int watchdog_open(struct inode *inode, struct file *file) {
    struct process_entry *new_entry;

    pr_info("device opened\n");

    mutex_lock(&watchdog_mtx);

    list_for_each_entry(new_entry, &process_list, list) {
        if (new_entry->pid == current->pid) {
            pr_info("process PID:%d already exists\n", current->pid);
            mutex_unlock(&watchdog_mtx);
            return 0;
        }
    }

    // add new process
    new_entry = kmalloc(sizeof(*new_entry), GFP_KERNEL);
    if (!new_entry) {
        pr_err("new_entry kmalloc error\n");
        mutex_unlock(&watchdog_mtx);
        return -ENOMEM;
    }

    new_entry->pid = current->pid;
    new_entry->task = current;
    timer_setup(&new_entry->timer, watchdog_timer_callback, 0);
    mod_timer(&new_entry->timer, jiffies + timeout * HZ);
    list_add(&new_entry->list, &process_list);

    mutex_unlock(&watchdog_mtx);

    return 0;
}

static int watchdog_release(struct inode *inode, struct file *file) {
    struct process_entry *entry, *tmp;

    pr_info("device closed\n");

    mutex_lock(&watchdog_mtx);

    list_for_each_entry_safe(entry, tmp, &process_list, list) {
        if (entry->pid == current->pid) {
            del_timer_sync(&entry->timer);
            list_del(&entry->list);
            kfree(entry);
            pr_info("removed PID:%d\n", current->pid);
            break;
        }
    }

    mutex_unlock(&watchdog_mtx);
    return 0;
}

// write ops update timer
static ssize_t watchdog_write(struct file *file, const char __user *buf, size_t len, loff_t *off) {
    struct process_entry *entry;

    pr_info("device write pid:%d\n", current->pid);

    mutex_lock(&watchdog_mtx);

    list_for_each_entry(entry, &process_list, list) {
        if (entry->pid == current->pid) {
            mod_timer(&entry->timer, jiffies + timeout * HZ);
            pr_info("reset timer for PID %d\n", current->pid);
            break;
        }
    }

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

    pr_info("watchdog initialized\n");
    return 0;
}

static void __exit watchdog_exit(void) {
    struct process_entry *entry, *tmp;

    pr_info("watchdog exiting...\n");

    mutex_lock(&watchdog_mtx);

    list_for_each_entry_safe(entry, tmp, &process_list, list) {
        del_timer_sync(&entry->timer);
        list_del(&entry->list);
        kfree(entry);
    }

    mutex_unlock(&watchdog_mtx);

    device_destroy(watchdog_class, dev_number);
    class_destroy(watchdog_class);
    cdev_del(&watchdog_cdev);
    unregister_chrdev_region(dev_number, 1);

    pr_info("watchdog exited\n");
}

module_init(watchdog_init);
module_exit(watchdog_exit);


