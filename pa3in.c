/*
 * COP 4600
 * Group 1
 * Johnathan Sattler
 * Michael Slater
 * Christian Theriot
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/mutex.h>
#include <asm/uaccess.h>

#define DEVICE_NAME "pa3inchar"
#define CLASS_NAME  "pa3in"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johnathan Sattler, Michael Slater, Christian Theriot");
MODULE_DESCRIPTION("COP 4600 - Programming Assignment 2");
MODULE_VERSION("1.0");

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

static DEFINE_MUTEX(pa3in_mutex);

static struct file_operations fops = {
	.write   = device_write,
	.open    = device_open,
	.release = device_release
};

static int Major;
static int numberOpens = 0;

#define BUF_LEN 1024
static char * msg;

EXPORT_SYMBOL(msg);

static struct class*  pa3charClass  = NULL;
static struct device* pa3charDevice = NULL;

static int __init pa3_init(void) {

	msg = (char *) vmalloc(sizeof(char) * BUF_LEN);
	msg[0] = '\0';

	printk(KERN_INFO "PA3 Input Module: Initialized.\n");

	Major = register_chrdev(0, DEVICE_NAME, &fops);
	if (Major < 0) {
		printk(KERN_ALERT "PA3 Input Module: Registering char device failed with %d.\n", Major);
		return Major;
	}
	printk(KERN_INFO "PA3 Input Module: Assigned major number - %d.\n", Major);

	pa3charClass = class_create(THIS_MODULE, CLASS_NAME);
   	if (IS_ERR(pa3charClass)){
      		unregister_chrdev(Major, DEVICE_NAME);
      		printk(KERN_ALERT "PA3 Module: Failed to register device class.\n");
      		return PTR_ERR(pa3charClass);
   	}
   	printk(KERN_INFO "PA3 Input Module: Device class registered correctly.\n");

	pa3charDevice = device_create(pa3charClass, NULL, MKDEV(Major, 0), NULL, DEVICE_NAME);
   	if (IS_ERR(pa3charDevice)){
      		class_destroy(pa3charClass);
      		unregister_chrdev(Major, DEVICE_NAME);
      		printk(KERN_ALERT "PA3 Input Module: Failed to create the device.\n");
      		return PTR_ERR(pa3charDevice);
   	}
   	printk(KERN_INFO "PA3 Input Module: Class created correctly.\n");

	mutex_init(&pa3in_mutex);

	return 0;
}

static void __exit pa3_exit(void) {

	mutex_destroy(&pa3in_mutex);

	device_destroy(pa3charClass, MKDEV(Major, 0));
   	class_unregister(pa3charClass);
   	class_destroy(pa3charClass);
   	unregister_chrdev(Major, DEVICE_NAME);

	printk(KERN_INFO "PA3 Input Module: De-initialized.\n");
}

static int device_open(struct inode *inode, struct file *file) {

	if (!mutex_trylock(&pa3in_mutex)) {
		printk(KERN_ALERT "PA3 Input Module: Device in use by another process.\n");
		return -EBUSY;
	}

	printk(KERN_INFO "PA3 Input Module: Character device opened %d times.\n", ++numberOpens);

	return 0;
}

static int device_release(struct inode *inode, struct file *file) {

	mutex_unlock(&pa3in_mutex);
	
	printk(KERN_INFO "PA3 Input Module: Character device closed.\n");

	return 0;
}

static ssize_t device_write(struct file *filp, const char *buffer, size_t len, loff_t * off) {

	strcat(msg, buffer);
		
	strcat(msg, "\0");

   	printk(KERN_INFO "PA3 Input Module: Received %zu characters from the user [%s].\n", len, buffer);

   	return len;
}

module_init(pa3_init);
module_exit(pa3_exit);
