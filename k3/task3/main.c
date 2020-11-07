#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <err.h>
#include <stdint.h>

//винаги брои един ред повече, който съдържа символа за нов ред, защото по конвенция файловете завършват с нов ред.
//съжалявам, че кодът не е изнесен във функции, но не ми остана време да го направя, заради объркването със setbuf.
int main(int argc, char * argv[])
{
	setbuf(stdout, NULL);
	int showLineNumbers = 0;

	//програмата е викната без параметри: чете от stdin без номера на редовете
	if(argc == 1)
	{
		char buffer[4096];
		int readBytes;
		while((readBytes = read(0, &buffer, 4096)))
		{
			if(write(1, &buffer, readBytes) == -1)
			{
				errx(1, "Couldn't write output.");
			}
		}

		return 0;
	}

	if(argc == 2 && strcmp("-n", argv[1]) == 0)
	{
		int lineNumber = 1;
		int firstLine = 1;

		char c;

		while(read(0, &c, 1) == 1)
		{
			if(firstLine)
			{
				printf("%d ", lineNumber);
				firstLine = 0;
			}

			if(c == '\n')
			{
				printf("\n");

				if(showLineNumbers)
				{
					lineNumber++;
					printf("%d ", lineNumber);
				}

				continue;
			}

			write(1, &c, 1);
		}

	}

	if(strcmp("-n", argv[1]) == 0)
	{
		showLineNumbers = 1;
	}

	int lineNumber = 1;
	int firstLine = 1;

	for(int i = showLineNumbers + 1; i < argc; i++)
	{
		

		if(strcmp(argv[i], "-") == 0)
		{
			char c;

			while(read(0, &c, 1) == 1)
			{
				if(firstLine && showLineNumbers)
				{
					printf("%d ", lineNumber);
					firstLine = 0;
				}

				if(c == '\n')
				{
					printf("\n");

					if(showLineNumbers)
					{
						lineNumber++;
						printf("%d ", lineNumber);
					}

					continue;
				}

				write(1, &c, 1);
			}
		}
		else
		{
			int fd = open(argv[i], O_RDONLY);
			if(fd == -1)
			{
				errx(1, "Couldn't open file");
			}

			char c;

			while(read(fd, &c, 1) == 1)
			{
				if(firstLine && showLineNumbers)
				{
					printf("%d ", lineNumber);
					firstLine = 0;
				}

				if(c == '\n')
				{
					printf("\n");

					if(showLineNumbers)
					{
						lineNumber++;
						printf("%d ", lineNumber);
					}

					continue;
				}

				write(1, &c, 1);
			}

			close(fd);
		}
	}


	return 0;
}
