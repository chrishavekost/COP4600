#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define BUFFER 1024            // store bytes written to it up to a constant buffer size (at least 1KB)
static char storage[BUFFER];   // stores the string

int main()
{

   int ret, thisFile,reader;
   char sendString[BUFFER];
   int thisFileWrite,thisFileRead;
   printf("Starting device test code example...\n");
   //thisFile = open("/dev/ebbchar", O_RDWR);        
   	thisFileWrite = open("/dev/ebbcharWrite", O_WRONLY);     
	thisFileRead = open("/dev/ebbcharRead", O_RDONLY);
	
   
   if (thisFileRead < 0)
   {
      perror("Failed to open the device...");
      return errno;
   }
   if (thisFileWrite < 0)
   {
      perror("Failed to open the device...");
      return errno;
   }
   
   int selection=0;
   int length;
   
   do{
	   printf("Select an option:\n");
	   printf("1- Write a string to the kernel module:\n");
	   printf("2- Read string from kernel module\n");
	   printf("0- Exit program\n\n");
	   scanf("%d%*c",&selection);
	   if(selection=1){
		   printf("Write a string to kernel module!\n");
		   printf("Type in a short string to send to the kernel module:\n");
		   scanf("%[^\n]%*c", sendString);
		   char newString[BUFFER];
		   if(strlen(sendString) > BUFFER)
		   {
		      strncpy(newString, sendString, 1024);
		      ret = write(thisFileWrite, newString, strlen(newString));
		      printf("Writing message to the device [%s].\n", newString);
		   }
		   else
		   {
		      // allow them to be read back out in FIFO fashion
		      printf("Writing message to the device [%s].\n", sendString);
		      ret = write(thisFileWrite, sendString, strlen(sendString)); 
		   }
		   if (ret < 0)
		   {
		      perror("Failed to write the message to the device.");
		      return errno;
		   }
	   }else if(selection=2){
		   printf("How many bytes would you like to read back?\n");
		   scanf("%d%*c",&reader);
		   if(reader>BUFFER){
			   reader=BUFFER;
		   }
		   printf('Reading from the device...\n');
		   ret=read(thisFileRead,storage,reader);
		   if(ret<0){
			   perror("Failed to read the message from device");
			   return errno;
		   }
		   printf("The received message is: [%s]\n",storage);
	   }
	   else{
		   selection=0;
	   }
   }while(selection>0);
   

   printf("End of the program\n");
   return 0;
}
