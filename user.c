/* Userspace program
 * Nikolay Nikolov
 *
 *
 */

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>

#define CHAR_DEVICE "/dev/A2"

int val;
int user;
int newval;
void toread();
int ret, fd;

int main ( void )
{
  /* File descriptor and returned value from write */

  printf (" This is a test \n");
  /* Open the module */

  fd = open(CHAR_DEVICE, O_RDWR); /* Open the device*/

  if (fd < 0)
  {
    perror("Device is broken \n");
    exit(1);
  }

  printf("Reading from the device \n");
  int retval = read(fd,&ret, 1);
  if (retval < 0)
  {
    perror ("Failed to read the message from the device. ");
    exit(1);
  }
  printf ("The syscall_val is : %d \n",ret);



  /* Write the new value for syscall */

  printf ("Type a number for syscall_val \n");
  scanf("%d", &newval);
  printf("The new value for syscall_vall is going to be %d \n",newval);

  /* Write */
  ret = write (fd, &newval, 1);
  if (ret < 0)
  {
    perror("Failed to write ! ");
    exit(1);
  }

  toread();

  return 0;

}

/* The function to read the new value */
void toread()
{
  do{
    printf (" Press 1 to read back from the device or press 0 to exit\n");
    scanf("%d",&user);

  }while (user != 0 && user != 1);

  if (user == 0)
  {
    exit(0);

  }
  if (user == 1)
  {
    printf("Reading from the device \n");
    int retval = read(fd,&ret, 1);
    if (retval < 0)
    {
      perror ("Failed to read the message from the device. ");
      exit(1);
    }
    printf ("The syscall_val is : %d \n",ret);

    printf("Exit ... \n");

  }
}


