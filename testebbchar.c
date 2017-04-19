#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define BUFFER 1024            // store bytes written to it up to a constant buffer size (at least 1KB)
static char sBuffer[BUFFER];   // stores the string

int main()
{

   int ret, thisFile;
   char sendString[BUFFER];
   printf("Starting device test code example...\n");
   //thisFile = open("/dev/ebbchar", O_RDWR);             
	thisFileRead = open("/dev/ebbcharRead", O_RDWR);
	thisFileWrite = open("/dev/ebbcharWrite", O_RDWR);
   
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


   printf("Type in a short string to send to the kernel module:\n");
   scanf("%[^\n]%*c", sendString);               
   
   // if not enough buffer is available to service a write request, the driver must only store up to the available amount
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

   printf("Press ENTER to read back from the device...\n");
   getchar();

   printf("Reading from the device...\n");
   ret = read(thisFileRead, sBuffer, BUFFER);        
   if (ret < 0)
   {
      perror("Failed to read the message from the device.");
      return errno;
   }
   printf("The received message is: [%s]\n", sBuffer);
   memset(sBuffer, 0, sizeof(sBuffer));
   printf("End of the program\n");
   return 0;
}
