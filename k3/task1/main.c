#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <err.h>
#include <stdint.h>

int main(int argc, char * argv[])
{
	if(argc != 4)
	{
		errx(1, "Invalid number of arguments.");
	}

	int fd1 = open(argv[1], O_RDONLY);
	if(fd1 == -1)
	{
		errx(1, "Couldn't open file 1.");
	}


	int fd2 = open(argv[2], O_RDONLY);
	if(fd2 == -1)
	{
		close(fd1);
		errx(1, "Couldn't open file 2.");
	}

	int fd3 = open(argv[3], O_WRONLY | O_TRUNC | O_CREAT, 0664);
	if(fd3 == -1)
	{
		close(fd1);
		close(fd2);
		errx(1, "Couldn't open file 3.");
	}

	uint16_t spacing = 0;
	uint8_t origbyte;
	uint8_t newbyte;

	while(1)
	{
		//If one file is bigger than the other, stop checking for differences
		if(read(fd1, &origbyte, 1) != 1)
		{
			break;
		}

		if(read(fd2, &newbyte, 1) != 1)
		{
			break;
		}

		if(origbyte != newbyte)
		{
			if(write(fd3, &spacing, 2) != 2)
			{
				close(fd1);
				close(fd2);
				close(fd3);
				errx(1, "Couldn't write to file.");
			}
			if(write(fd3, &origbyte, 1) != 1)
			{
				close(fd1);
				close(fd2);
				close(fd3);
				errx(1, "Couldn't write to file.");
			}
			if(write(fd3, &newbyte, 1) != 1)
			{
				close(fd1);
				close(fd2);
				close(fd3);
				errx(1, "Couldn't write to file.");
			}
		}

		spacing++;
	}

	close(fd1);
	close(fd2);
	close(fd3);

	return 0;
}
