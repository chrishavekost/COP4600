#include <linux/init.h>           
#include <linux/module.h>        
#include <linux/device.h>         
#include <linux/kernel.h>         
#include <linux/fs.h>             
#include <asm/uaccess.h>   
#include <linux/mutex.h>
#define BUFFER 1024
#define  DEVICE_NAME "ebbcharRead"    
#define  CLASS_NAME  "ebb"        

// Credit to Derek Molloy at derekmolloy.ie for a large portion of this code. 

MODULE_LICENSE("GPL");            
MODULE_AUTHOR("Freeman, Havekost, Patino");    
MODULE_DESCRIPTION("COP4600 Programming Assignment 3");  
MODULE_VERSION("1.0");         

static int    mdNumber;                   // major device number
static char   message[1024] = {0}; 			// store bytes written to it up to a constant buffer size (at least 1KB)          
static short  messageLength;              
static int    nOpens = 0;					   // stores the number of times the device opens
static struct class*  charClass  = NULL;  // class pointer
static struct device* charDev = NULL; 		// device pointer
extern struct mutex global_mutex;

static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);

static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .release = dev_release,
};

// the driver will need to initialize itself
static int __init ebbchar_init(void)
{
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
   strcpy(message, "");
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
   nOpens++;

   // report using printk each time its character device is opened
   printk(KERN_INFO "EBBChar: Device has been opened %d time(s)\n", nOpens);
   return 0;
}

// read from the device
static ssize_t dev_read(struct file *filep, char *buffer, size_t length, loff_t *offset)
{
   int error_count = 0;
   static char messageTemp [BUFFER] = {0};
   size_t i;
   
   if(!mutex_trylock(&global_mutex)){
	   printk("EBBChar : Device in use by another process");
	   return -EBUSY;
   }
   if(length >= messageLength){
	   if(messageLength == 0){
		   error_count = copy_to_user(buffer,"",1);
	   }else{
		   error_count=copy_to_user(buffer,message,messageLength);
	   }
	   if(error_count == 0){
		   printk(KERN_INFO "EBBChar: Sent %d characters to the user", messageLength);
		   strcpy(message,"");
		   messageLength=0;
		   mutex_unlock(&global_mutex);
		   return 0;
	   }else{
		   printk(KERN_INFO "EBBChar: Failed to send characters to the user");
		   mutex_unlock(&global_mutex);
		   return -EFAULT;
	   }
   }else{
	   error_count = copy_to_user(buffer,message,length);
	   if(error_count==0){
		   printk(KERN_INFO "EBBChar: Sent %d characters to the user", messageLength);
		   messageLength -= length;
		   for(i=length;i<BUFFER; i++){
			   messageTemp[i-length]=message[i];
		   }
		   strcpy(message,messageTemp);
		   strcpy(messageTemp,"");
		   mutex_unlock(&global_mutex);
		   return 0;
	   }else{
		   printk(KERN_INFO "EBBChar: Failed to send characters to the user");
		   mutex_unlock(&global_mutex);
		   return -EFAULT;
	   }
   }
   mutex_unlock(&global_mutex);
}

// close the device
static int dev_release(struct inode *inodep, struct file *filep)
{
   // report using printk each time its character device is closed
   printk(KERN_INFO "EBBChar: Device successfully closed\n");
   return 0;
}

// initialize the kernel module
module_init(ebbchar_init);

// deinitialize the kernel module
module_exit(ebbchar_exit);
