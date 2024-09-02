/*
 * chardev.c : A read-only char device driver that tells how many times you have
 * read from the dev file
 *
 */
#include <asm/errno.h>
#include <linux/atomic.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/version.h>
#define DEVICE_NAME "chardev" // Device names which appears in /proc/devices
#define BUF_LEN 80

// Prototypes of file operations
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char __user *, size_t,
                            loff_t *);

static int major; // Major number assigned to this device driver
static struct file_operations chardev_fops = {.write = device_write,
                                              .read = device_read,
                                              .release = device_release,
                                              .open = device_open};
static char msg[BUF_LEN];
static struct class *cls;
enum {
  CDEV_NOT_USED = 0,
  CDEV_EXCLUSIVE_OPEN = 1,
};
static atomic_t already_open = ATOMIC_INIT(CDEV_NOT_USED);

static int __init chardev_init(void) {
  major = register_chrdev(0, DEVICE_NAME, &chardev_fops);
  if (major < 0) {
    pr_alert("Register device failded with %d\n", major);
    return major;
  }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
  cls = class_create(DEVICE_NAME);
#else
  cls = class_create(THIS_MODULE, DEVICE_NAME);
#endif
  device_create(cls, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
  pr_info("Assigned major number: %d\n", major);
  pr_info("Device created on /dev/%s\n", DEVICE_NAME);
  return 0;
}

static void __exit chardev_exit(void) {
  device_destroy(cls, MKDEV(major, 0));
  class_destroy(cls);
  unregister_chrdev(major, DEVICE_NAME);

  pr_info("Chardev exited\n");
}

static int device_open(struct inode *inode, struct file *file) {
  static int count = 0;
  if (atomic_cmpxchg(&already_open, CDEV_NOT_USED, CDEV_EXCLUSIVE_OPEN))
    return -EBUSY;
  sprintf(msg, "This shit has been opened %d times", count++);
  try_module_get(THIS_MODULE);
  return 0;
}
static int device_release(struct inode *inode, struct file *file) {
  atomic_set(&already_open, CDEV_NOT_USED);
  module_put(THIS_MODULE);
  return 0;
}
static ssize_t device_read(struct file *filp, char __user *buffer,
                           size_t length, loff_t *offset) {
  int bytes_read = 0;
  const char *msg_ptr = msg;
  if (!*(msg_ptr + *offset)) {
    *offset = 0;
    return 0;
  }
  msg_ptr += *offset;
  while (length && *msg_ptr) {
    put_user(*(msg_ptr++), buffer++);
    length--;
    bytes_read++;
  }
  *offset += bytes_read;
  return bytes_read;
}
static ssize_t device_write(struct file *filp, const char __user *buffer,
                            size_t length, loff_t *offset) {
  pr_alert("Sorry, this operation is not supported");
  return -EINVAL;
}
module_init(chardev_init);
module_exit(chardev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hoang Minh");
MODULE_DESCRIPTION("A read-only character device driver");
