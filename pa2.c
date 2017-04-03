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

#define DEVICE_NAME "pa2char"
#define CLASS_NAME  "pa2"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johnathan Sattler, Michael Slater, Christian Theriot");
MODULE_DESCRIPTION("COP 4600 - Programming Assignment 2");
MODULE_VERSION("1.0");

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations fops = {
	.read    = device_read,
	.write   = device_write,
	.open    = device_open,
	.release = device_release
};

static int Major;
static int numberOpens = 0;

#define BUF_LEN 1024
static char * msg;
static short msgSize;

static struct class*  pa2charClass  = NULL;
static struct device* pa2charDevice = NULL;

static int __init pa2_init(void) {

	msg = (char *) vmalloc(sizeof(char) * BUF_LEN);

	printk(KERN_INFO "PA2 Module: Initialized.\n");

	Major = register_chrdev(0, DEVICE_NAME, &fops);
	if (Major < 0) {
		printk(KERN_ALERT "PA2 Module: Registering char device failed with %d.\n", Major);
		return Major;
	}
	printk(KERN_INFO "PA2 Module: Assigned major number - %d.\n", Major);

	pa2charClass = class_create(THIS_MODULE, CLASS_NAME);
   	if (IS_ERR(pa2charClass)){
      		unregister_chrdev(Major, DEVICE_NAME);
      		printk(KERN_ALERT "PA2 Module: Failed to register device class.\n");
      		return PTR_ERR(pa2charClass);
   	}
   	printk(KERN_INFO "PA2 Module: Device class registered correctly.\n");

	pa2charDevice = device_create(pa2charClass, NULL, MKDEV(Major, 0), NULL, DEVICE_NAME);
   	if (IS_ERR(pa2charDevice)){
      		class_destroy(pa2charClass);
      		unregister_chrdev(Major, DEVICE_NAME);
      		printk(KERN_ALERT "PA2 Module: Failed to create the device.\n");
      		return PTR_ERR(pa2charDevice);
   	}
   	printk(KERN_INFO "PA2 Module: Class created correctly.\n");

	return 0;
}

static void __exit pa2_exit(void) {

	device_destroy(pa2charClass, MKDEV(Major, 0));
   	class_unregister(pa2charClass);
   	class_destroy(pa2charClass);
   	unregister_chrdev(Major, DEVICE_NAME);

	printk(KERN_INFO "PA2 Module: De-initialized.\n");
}

static int device_open(struct inode *inode, struct file *file) {

	printk(KERN_INFO "PA2 Module: Character device opened %d times.\n", ++numberOpens);

	return 0;
}

static int device_release(struct inode *inode, struct file *file) {

	printk(KERN_INFO "PA2 Module: Character device closed.\n");

	return 0;
}

static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t * offset) {

	int errorCount = 0;
	
	if (msgSize > 0) {
		errorCount = copy_to_user(buffer, msg, msgSize);
		
		if (errorCount == 0) {
				printk(KERN_INFO "PA2 Module: Sent %d characters to the user [%s].\n", msgSize, msg);
				return (msgSize = 0);
		}
		
		printk(KERN_INFO "PA2 Module: Failed to send %d characters to the user.\n", errorCount);
		return -EFAULT;
	}
      	
	printk(KERN_INFO "PA2 Module: User tried to read the empty buffer.\n");
    return -EFAULT;
}

static ssize_t device_write(struct file *filp, const char *buffer, size_t len, loff_t * off) {

	if (msgSize == 0)
		sprintf(msg, "%s", buffer);
	else
		strcat(msg, buffer);
		
	strcat(msg, "\0");
   	msgSize = strlen(msg);

   	printk(KERN_INFO "PA2 Module: Received %zu characters from the user [%s].\n", len, buffer);

   	return len;
}

module_init(pa2_init);
module_exit(pa2_exit);

