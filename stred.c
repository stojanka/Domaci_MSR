#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/wait.h>
#define STRED_SIZE 100
MODULE_LICENSE("Dual BSD/GPL");

dev_t my_dev_id;
static struct class *my_class;
static struct device *my_device;
static struct cdev *my_cdev;

DECLARE_WAIT_QUEUE_HEAD(writeQueue);
DECLARE_WAIT_QUEUE_HEAD(readQueue);
struct semaphore sem;

char stred[100];
int endRead = 0;

int stred_open(struct inode *pinode, struct file *pfile);
int stred_close(struct inode *pinode, struct file *pfile);
ssize_t stred_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset);
ssize_t stred_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset);

struct file_operations my_fops =
{
	.owner = THIS_MODULE,
	.open = stred_open,
	.read = stred_read,
	.write = stred_write,
	.release = stred_close,
};


int stred_open(struct inode *pinode, struct file *pfile)
{
		printk(KERN_INFO "Succesfully opened stred\n");
		return 0;
}

int stred_close(struct inode *pinode, struct file *pfile)
{
		printk(KERN_INFO "Succesfully closed stred\n");
		return 0;
}

ssize_t stred_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset)
{
	char *buff = (char *)kmalloc(length, GFP_KERNEL);
  char *pom;

  int ret;
  int i,j;
	char *original;
	original = buff;

	ret = copy_from_user(buff, buffer, length);
	if(ret)
			return -EFAULT;
	buff[length-1] = '\0';

  if(strncmp(buff, "string=", 7) == 0){

    strsep(&buff, "=");
		if (strlen(buff)>99){
			printk(KERN_ERR "String ce premasiti 100karaktera!\n");
		}else{
			strcpy(stred, buff);
		}
		//if(wait_event_interruptible(writeQueue,(strlen(stred)+strlen(buff)<=99)))
			//return -ERESTARTSYS;

		//strcpy(stred, buff);
		wake_up_interruptible(&readQueue);
  }

  if(strncmp(buff, "clear", 5) == 0){
  strsep(&buff, "r");
    for(i=0; i< strlen(stred); i++){
      stred[i] = '0';
    }
		wake_up_interruptible(&writeQueue);
  }

  if(strncmp(buff, "shrink", 6) == 0){
		strsep(&buff, "k");
    pom = strim(stred);
		strcpy(stred, pom);

		wake_up_interruptible(&writeQueue);
  }

  if(strncmp(buff, "append=", 7) == 0){

    strsep(&buff, "d");
		strreplace(buff, '=', ' ');
	if (strlen(stred)+strlen(buff)>99)
			printk(KERN_ERR "String ce premasiti 100karaktera!\n");

		if(wait_event_interruptible(writeQueue,(strlen(stred)+strlen(buff)<=99)))
			return -ERESTARTSYS; //sistemski poziv je potrebno pozvati ponovo

		strncat(stred, buff, strlen(buff));
		wake_up_interruptible(&readQueue);
  }

  if(strncmp(buff, "truncate=", 9) == 0){
		strsep(&buff, "=");
		kstrtoint(buff, 10, &ret);
			if(ret > strlen(stred)){
				printk(KERN_WARNING "The number is out of range\n");

			}else{
				stred[strlen(stred)-ret] = '\0';
			}
		wake_up_interruptible(&writeQueue);
  }

  if(strncmp(buff, "remove=", 6) == 0){
		strim(buff);
		strsep(&buff, "=");

	if(strstr(stred, buff) == NULL)
	{
		printk("Word not found \n");
	}else
	{
		while(strstr(stred, buff)!=NULL){
			pom = strstr(stred, buff);
 			for (i=0; i < strlen(buff)+1 ; i++){
				if(i==strlen(buff) && *(pom+i)!=' '){
		      *(pom+i) = '\0';
		    }else{
		      *(pom+i) = '#';
		    }
			}

		//brisanje '#'
		j=0;
	  for(i=0; stred[i]!= '\0'; i++){
			if((stred[i]!='#') || ((stred[i]!='#') && (stred[i+1]!=' '))){
			  stred[j++] = stred[i];
		  }
	  }
stred[j] = '\0';

}//od while
} // od else
wake_up_interruptible(&writeQueue);
} // od remove


printk("Buff je: %s \n", buff);
printk("Stred je: %s \n", stred);
  kfree(original);
	return length;

}


ssize_t stred_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset)
{
int ret;

if(endRead){
	endRead=0;
	printk(KERN_INFO "Succesfully read from file \n");
	return 0;
}
if(wait_event_interruptible(readQueue,(strlen(stred) > 0))) //budi se tek kada se u stred upise nesto
	return -ERESTARTSYS; //resetujem sistemski poziv

if(strlen(stred)>0){
	ret= copy_to_user(buffer, stred, strlen(stred));
	if(ret)
		return -EFAULT;
}else{
	printk(KERN_WARNING "Stred is empty \n");
}
wake_up_interruptible(&writeQueue); //cim procitam nesto budim ovaj proces
endRead = 1;

return strlen(stred);
}


static int __init stred_init(void)
{
  int ret = 0;
    int i=0;

    //Initialize array
    for (i=0; i<100; i++)
      stred[i] = 0;

  ret = alloc_chrdev_region(&my_dev_id, 0, 1, "stred");
  if (ret){
      printk(KERN_ERR "failed to register char device\n");
      return ret;
   }
   printk(KERN_INFO "char device region allocated\n");

   my_class = class_create(THIS_MODULE, "stred_class");
   if (my_class == NULL){
     printk(KERN_ERR "failed to create class\n");
     goto fail_0;
   }
   printk(KERN_INFO "class created\n");

   my_device = device_create(my_class, NULL, my_dev_id, NULL, "stred");
   if (my_device == NULL){
      printk(KERN_ERR "failed to create device\n");
      goto fail_1;
   }
    printk(KERN_INFO "device created\n");

        my_cdev = cdev_alloc();
      	my_cdev->ops = &my_fops;
      	my_cdev->owner = THIS_MODULE;
      	ret = cdev_add(my_cdev, my_dev_id, 1);
      	if (ret)
      	{
      printk(KERN_ERR "failed to add cdev\n");
      		  goto fail_2;
      	}
         printk(KERN_INFO "cdev added\n");
         printk(KERN_INFO "Hello world\n");

         return 0;

         fail_2:
            device_destroy(my_class, my_dev_id);
         fail_1:
            class_destroy(my_class);
         fail_0:
            unregister_chrdev_region(my_dev_id, 1);
         return -1;
      }

static void __exit stred_exit(void)
{
  cdev_del(my_cdev);
  device_destroy(my_class, my_dev_id);
  class_destroy(my_class);
  unregister_chrdev_region(my_dev_id,1);
  printk(KERN_INFO "Goodbye, cruel world\n");
}


module_init(stred_init);
module_exit(stred_exit);
