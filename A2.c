/* Assignment 2
 * The purpose of this assignment is to build a character  driver.
 * For more information about Char Drivers please refer to the  READ_ME file for
 * the program
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>/*The file system header is the header required for writing
		       device drivers */
#include <linux/kdev_t.h>
#include <linux/cdev.h>/* Represent Char Devices Internally */
#include <linux/slab.h>
#include <linux/uaccess.h>


#define DEVNAME "A2"
#define DEVCNT 1



/* A kernel module that creates and 
 * registers a single character device (struct cdev)*/

/* You will want to embed the cdev structure within a device specific
 * structure*/

static struct mydev_dev
{

  struct cdev cdev; /* Char device structure */

  int syscall_val;/* Used by the user */

}mydev;



static int exam = 1;

struct class *A2_class; /* Tie with the device model */
static dev_t A2_node;

/*this shows up under /sys/modules/A2/parameters */

/* These symbolic constants are defined for the file mode 
 * bits that control access permission for the file: */
/* Read permission bit for the owner of the file */
/* Write permission bit for the owner of the file */

module_param(exam,int, S_IRUSR | S_IWUSR) ;

/* The module_param() macro takes 3 arguments: the name of the variable, 
 * its type and permissions for the corresponding file in sysfs. 
 * Integer types can be signed as usual or unsigned. */


/* this doesn't appear in /sys/module */
static int exam_nosysfs = 40;
module_param(exam_nosysfs, int, 0);

/* Open is the method called every time someone opens
 * your device's file. Device opening will always be successful in cases where
 * this method is not defined. You usually use this method to perform device
 * and data structure initialization, and return a negative error code if
 * something goes wrong or 0. The prototype of the open method is defined as :
 * int (*open)(struct inode *inode, struct file *flip);*/

static int A2_open(struct inode *inode, struct file *file)
{

  struct mydev_dev *mydev = NULL;
  /* Get the per - device structure that contains this cdev */
  mydev = container_of(inode->i_cdev, struct mydev_dev, cdev);

  /* Initialize */

  mydev -> syscall_val = 40;

  /* Easy access to mydev_dev from rest of the entry points */
  file->private_data = mydev;

  printk(KERN_INFO "Success! Opened ! \n");
  return 0;


  /* For each open performed on your char device , the callback function will be
   * given a struct inode as a parameter, which is the kernel lower-level
   * representation of the file. The struct inode structure has a field named
   * i_cdev, which points to the cdev we have allocated in the init function */

}

static ssize_t A2_read(struct file *file, char __user *buf, size_t len, loff_t
    *offset)
{
  /* - *buf is the buffer we receive from the user space
   * - len is the size of the requested transfer (size of the user buffer)
   * - *offset indicates the start position from which the data should be read in
   * the file*/

  /* Get a local kernel buffer set aside */
  int ret;

  if (*offset >= sizeof(int)) return 0; /* */


  /*  Copy the data into the user space buffer and return an error on failure : */


  if (copy_to_user (buf,&mydev.syscall_val, sizeof(int)))
  {
    ret = -EFAULT;
    goto out;
  }

  /* Make sure our user wasn't bad...*/
  if (!buf)
  {
    ret = EINVAL;
    goto out;
  }

  ret = sizeof(int);
  *offset += len;

  /* Success */

  printk(KERN_INFO "User got from us %d\n", mydev.syscall_val);

out:
  return ret;

}

static ssize_t A2_write(struct file *file, const char __user *buf,size_t len,
    loff_t *offset)
{
  /* The write method is used to send data to the device,
   * whenever a user app calls the write function on the device's file, the
   * kernel implementation is called. Its prototype is as follows:
   * ssize_t(*write)(struct file *flip, const char _user *buf, size_t count,
   * loff_t *pos);
   *
   * - The return value is the number of bytes(size) written
   * - *buf represents the data buffer coming from the user space
   * - count is the size of the requested transfer
   * - *pos indicates the start position from which the data should be written in
   * the file.
   *
   *Suggested steps:
   * 1 . Check for bad or invalid requests coming from the user space.
   * 2 . Adjust count for the remaining bytes in order to not go beyond the file
   *   size.
   * 3 . Find the location from which you will start to write.
   * 4 . Copy data from the user space and write it into the appropriate kernel
   *   space.
   * 5 . Write to the physical device and return an error on failure. */

  /* Have local kernel memory ready */
  int *kern_buf;
  int ret;

  /* Make sure our user isn't bad... */
  if (!buf)
  {
    ret = -EINVAL;
    goto out;
  }

  /* Get some memory to copy into... */
  kern_buf = kmalloc (len, GFP_KERNEL);

  /* ...and make sure it is good to go */

  if (!kern_buf)
  {
    ret = -ENOMEM;
    goto out;

  }

  /* Copy from the user-provided buffer */
  if (copy_from_user(&mydev.syscall_val, buf, len))
  {
    ret = -EFAULT;
    goto mem_out;

  }

  ret = len;

  /* print what userspace gave us */
  exam = mydev.syscall_val;

  printk (KERN_INFO "Userspace wrote \"%n\" to us \n", &mydev.syscall_val);



mem_out:
  kfree (kern_buf);
out:
  return ret;

}

int  A2_release (struct inode *inode, struct file *file)
{
  return 0;
}

static struct file_operations mydev_fops = {

  /* C99 way of assigning to elements of a structure,
   * 	you should use this syntax in case someone wants to
   * 	port your driver */

  .owner = THIS_MODULE,
  .open  = A2_open,
  .read  = A2_read,
  .write = A2_write,
  .release = A2_release,
};



/* File operations for our device */


/* Driver initialization */
static int __init A2_init(void)
{

  /* Request Dynamic allocation of a device major number */

  printk(KERN_INFO "A2 module loading... exam =%d\n",exam);

  if(alloc_chrdev_region(&A2_node, 0, DEVCNT, DEVNAME) < 0)
  {
    printk(KERN_ERR "alloc_chrdev_region() failed! \n");

    return -1;
  }

  /* Populate sysfs entries */

  A2_class = class_create(THIS_MODULE, DEVNAME);

  /* Send uevents to udev , so it'll create /dev nodes */
  device_create(A2_class,NULL,A2_node, NULL, DEVNAME);

  /* Initialize the char device and add it to the kernel */

  /* Connect the file operations with the cdev */
  cdev_init (&mydev.cdev, &mydev_fops);

  mydev.cdev.owner = THIS_MODULE;/* has an owner field that should be set to
				    "THIS_MODULE" */

  /* Connect the major /minor number to the cdev */

  if (cdev_add (&mydev.cdev,A2_node,DEVCNT))
  {
    printk (KERN_ERR "cdev_add() failed ! \n");

    /* Clean up chrdev allocation */

    unregister_chrdev_region(A2_node, DEVCNT);

    return -1;
  }

  printk (KERN_INFO "Allocated %d devices at major: %d \n", DEVCNT,
      MAJOR(A2_node));
  return 0;
}


//---------------------------------------

static void __exit A2_exit(void)
{
  /*destroy the cdev */
  cdev_del(&mydev.cdev);

  /* clean up the devices */
  unregister_chrdev_region(MAJOR(A2_node), 1);

  /* Destroy A2_class */
  class_destroy(A2_class);

  printk(KERN_INFO "A2 module loading... exam =%d\n",exam);
  printk(KERN_INFO " A2 device unloaded ! \n");

}

MODULE_AUTHOR(" NIK NIKOLOV ");
MODULE_LICENSE("GPL");
MODULE_VERSION("2100");

module_init(A2_init);
module_exit(A2_exit);



