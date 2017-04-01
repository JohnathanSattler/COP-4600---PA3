#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>
 
#define BUFFER_LENGTH 1024
static char receive[BUFFER_LENGTH];
 
int main() {

	int ret, fd, exit;
	char stringToSend[BUFFER_LENGTH];

	printf("Starting device test code example...\n");

	fd = open("/dev/pa2char", O_RDWR);

	if (fd < 0) {
		perror("Failed to open the device...");
		return errno;
	}

	printf("Type in a string to send to the kernel module (press ENTER to skip):\n");
	fgets(stringToSend, BUFFER_LENGTH, stdin);
	
	if (stringToSend[strlen(stringToSend) - 1] == '\n')
		stringToSend[strlen(stringToSend) - 1] = '\0';

	if (strcmp(stringToSend, "") != 0) {
		printf("Writing message to the device [%s].\n", stringToSend);
		ret = write(fd, stringToSend, strlen(stringToSend));
	}

	if (ret < 0) {
		perror("Failed to write the message to the device.");
		return errno;
	}
 
	while (ret >= 0 || stringToSend[0] == 'q') {
		printf("Press ENTER to read from the device, or e to exit...\n");
		exit = getchar();
	   
		if (exit == 'e')
			break;
		
		printf("Reading from the device...\n");
		ret = read(fd, receive, BUFFER_LENGTH);

		if (ret < 0) {
			perror("Failed to read the message from the device.");
			return errno;
		}

		printf("The received message is: [%s]\n", receive);
	}
	
	printf("End of the program\n");

	return 0;
}
