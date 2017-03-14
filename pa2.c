/*
 * COP 4600
 * Group 1
 * Johnathan Sattler
 * Michael Slater
 * Christian Theriot
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#define DRIVER_AUTHOR "Johnathan Sattler, Michael Slater, Christian Theriot"
#define DRIVER_DESC "COP 4600 - Programming Assignment 2"

MODULE_LICENSE("GPL");

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define DEVICE_NAME "cop4600pa2"
#define BUF_LEN 80

static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};

static int Major;
static int Device_Open = 0;

static char msg[BUF_LEN];
static char *msg_Ptr;

static int __init pa2_init(void) {

	Major = register_chrdev(0, DEVICE_NAME, &fops);

	if (Major < 0) {
		printk(KERN_ALERT "Registering char device failed with %d\n", Major);
		return Major;
	}

	printk(KERN_INFO "PA2 Module assigned major number: %d\n", Major);
	printk(KERN_INFO "PA2 Module initialized.\n");

	return 0;
}

static void __exit pa2_exit(void) {

	unregister_chrdev(Major, DEVICE_NAME);

	printk(KERN_INFO "PA2 Module de-initialized.\n");
}

module_init(pa2_init);
module_exit(pa2_exit);

static int device_open(struct inode *inode, struct file *file)
{
	static int counter = 0;

	if (Device_Open)
		return -EBUSY;

	Device_Open++;
	sprintf(msg, "Device opened %d times.\n", counter++);
	msg_Ptr = msg;
	try_module_get(THIS_MODULE);

	printk(KERN_INFO "PA2 Module character device opened.\n");

	return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
	Device_Open--;

	module_put(THIS_MODULE);

	printk(KERN_INFO "PA2 Module character device closed.\n");

	return 0;
}

static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t * offset) {

	int bytes_read = 0;

	if (*msg_Ptr == 0) {
		return 0;
	}

	while (length && *msg_Ptr) {
		put_user(*(msg_Ptr++), buffer++);

		length--;
		bytes_read++;
	}

	printk(KERN_INFO "PA2 Module character device read.\n");

	return bytes_read;
}

static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t * off) {

	printk(KERN_ALERT "Sorry, this operation isn't supported.\n");

	printk(KERN_INFO "PA2 Module character device written.\n");

	return -EINVAL;
}

