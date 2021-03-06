#include <linux/init.h>           
#include <linux/module.h>        
#include <linux/device.h>         
#include <linux/kernel.h>         
#include <linux/fs.h>             
#include <asm/uaccess.h> 

#include <linux/mutex.h>
//#include "ebbcharRead"

#define  DEVICE_NAME "ebbcharWrite"    
#define  CLASS_NAME  "ebbWrite"        

// Credit to Derek Molloy at derekmolloy.ie for a large portion of this code. 

MODULE_LICENSE("GPL");            
MODULE_AUTHOR("Freeman, Havekost, Pantino");    
MODULE_DESCRIPTION("COP4600 Programming Assignment 3");  
MODULE_VERSION("1.0");         

static int    mdNumber;                   // major device number
extern char   message[1024] = {0}; 			// store bytes written to it up to a constant buffer size (at least 1KB)          
extern short  messageLength;              
static int    nOpens = 0;					   // stores the number of times the device opens
static struct class*  charClass  = NULL;  // class pointer
static struct device* charDev = NULL; 		// device pointer

static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

DEFINE_MUTEX(ebbchar_mutex);

static struct file_operations fops =
{
   .open = dev_open,
   .write = dev_write,
   .release = dev_release,
};

// the driver will need to initialize itself
static int __init ebbchar_init(void)
{
   mutex_init(&ebbchar_mutex); // Initalize mutex at runtime with the reader module.
   printk(KERN_INFO "EBBChar: Initializing the EBBChar LKM\n");

   // obtaining a new major device number
   mdNumber = register_chrdev(0, DEVICE_NAME, &fops);
   if (mdNumber < 0)
   {
      printk(KERN_ALERT "EBBChar failed to register a major number\n");
      return mdNumber;
   }

   // registering the driver
   printk(KERN_INFO "EBBChar: registered correctly with major number %d\n", mdNumber);

   // register the device
   charClass = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(charClass))
   {                
      unregister_chrdev(mdNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to register device class\n");
      return PTR_ERR(charClass);          
   }
   printk(KERN_INFO "EBBChar: device class registered correctly\n");

   // initialize the device
   charDev = device_create(charClass, NULL, MKDEV(mdNumber, 0), NULL, DEVICE_NAME);
   if (IS_ERR(charDev))
   {               
      class_destroy(charClass);           
      unregister_chrdev(mdNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to create the device\n");
      return PTR_ERR(charDev);
   }
   printk(KERN_INFO "EBBChar: device class created correctly\n"); 
   return 0;
}


static void __exit ebbchar_exit(void)
{
   // de-register the device
   device_destroy(charClass, MKDEV(mdNumber, 0));     
   class_unregister(charClass);                          
   class_destroy(charClass);
   
   // de-register the major device numberc                         
   unregister_chrdev(mdNumber, DEVICE_NAME);             
   printk(KERN_INFO "EBBChar: Goodbye from the LKM!\n");
}

// open the device
static int dev_open(struct inode *inodep, struct file *filep)
{
   if(!mutex_trylock(&ebbchar_mutex))
   {
      // Tries to acquire the mutex, returns 1 if successful, 0 if it is already in use.
      printk(KERN_ALERT "EBBChar: Device currently being used by another process");
      return -EBUSY;
   }

   nOpens++;

   // report using printk each time its character device is opened
   printk(KERN_INFO "EBBChar: Device has been opened %d time(s)\n", nOpens);
   return 0;
}

// write to the device
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
   sprintf(message, "%s(%zu letters)", buffer, len);   
   messageLength = strlen(message);         
   // report using printk each time its character device is written        
   printk(KERN_INFO "EBBChar: Received %zu characters from the user\n", len);
   return len;
}

// close the device
static int dev_release(struct inode *inodep, struct file *filep)
{
   mutex_unlock(&ebbchar_mutex); // Release mutex so that the writer module can use it.
   
   // report using printk each time its character device is closed
   printk(KERN_INFO "EBBChar: Device successfully closed\n");
   return 0;
}

// initialize the kernel module
module_init(ebbchar_init);

// deinitialize the kernel module
module_exit(ebbchar_exit);
