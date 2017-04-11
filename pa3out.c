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
#include <asm/uaccess.h>

#define DEVICE_NAME "pa3outchar"
#define CLASS_NAME  "pa3out"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johnathan Sattler, Michael Slater, Christian Theriot");
MODULE_DESCRIPTION("COP 4600 - Programming Assignment 2");
MODULE_VERSION("1.0");

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);

static DEFINE_MUTEX(pa3out_mutex);

static struct file_operations fops = {
	.read    = device_read,
	.open    = device_open,
	.release = device_release
};

static int Major;
static int numberOpens = 0;

#define BUF_LEN 1024
extern char * msg;

static struct class*  pa3charClass  = NULL;
static struct device* pa3charDevice = NULL;

static int __init pa3_init(void) {

	msg = (char *) vmalloc(sizeof(char) * BUF_LEN);

	printk(KERN_INFO "PA3 Output Module: Initialized.\n");

	Major = register_chrdev(0, DEVICE_NAME, &fops);
	if (Major < 0) {
		printk(KERN_ALERT "PA3 Output Module: Registering char device failed with %d.\n", Major);
		return Major;
	}
	printk(KERN_INFO "PA3 Output Module: Assigned major number - %d.\n", Major);

	pa3charClass = class_create(THIS_MODULE, CLASS_NAME);
   	if (IS_ERR(pa3charClass)){
      		unregister_chrdev(Major, DEVICE_NAME);
      		printk(KERN_ALERT "PA3 Output Module: Failed to register device class.\n");
      		return PTR_ERR(pa3charClass);
   	}
   	printk(KERN_INFO "PA3 Output Module: Device class registered correctly.\n");

	pa3charDevice = device_create(pa3charClass, NULL, MKDEV(Major, 0), NULL, DEVICE_NAME);
   	if (IS_ERR(pa3charDevice)){
      		class_destroy(pa3charClass);
      		unregister_chrdev(Major, DEVICE_NAME);
      		printk(KERN_ALERT "PA3 Output Module: Failed to create the device.\n");
      		return PTR_ERR(pa3charDevice);
   	}
   	printk(KERN_INFO "PA3 Output Module: Class created correctly.\n");

	mutex_init(&pa3out_mutex);

	return 0;
}

static void __exit pa3_exit(void) {

	mutex_destroy(&pa3out_mutex);

	device_destroy(pa3charClass, MKDEV(Major, 0));
   	class_unregister(pa3charClass);
   	class_destroy(pa3charClass);
   	unregister_chrdev(Major, DEVICE_NAME);

	printk(KERN_INFO "PA3 Output Module: De-initialized.\n");
}

static int device_open(struct inode *inode, struct file *file) {

	if (!mutex_trylock(&pa3out_mutex)) {
		printk(KERN_ALERT "PA3 Output Module: Device in use by another process.\n");
		return -EBUSY;
	}

	printk(KERN_INFO "PA3 Output Module: Character device opened %d times.\n", ++numberOpens);

	return 0;
}

static int device_release(struct inode *inode, struct file *file) {

	mutex_unlock(&pa3out_mutex);

	printk(KERN_INFO "PA3 Output Module: Character device closed.\n");

	return 0;
}

static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t * offset) {

	int errorCount = 0;

	errorCount = copy_to_user(buffer, msg, strlen(msg));
	
	if (errorCount == 0) {
			printk(KERN_INFO "PA3 Output Module: Sent %d characters to the user [%s].\n", strlen(msg), msg);
			return (msg[0] = '\0');
	}
		
	printk(KERN_INFO "PA3 Output Module: Failed to send %d characters to the user.\n", errorCount);
	return -EFAULT;
}

module_init(pa3_init);
module_exit(pa3_exit);

