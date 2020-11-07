#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <err.h>
#include <stdint.h>

int main()
{
	int pipefd[2];

	if(pipe(pipefd) == -1)
	{
		errx(1, "Couldn't open pipe.");
	}

	int child_pid = fork();
	if(child_pid == -1)
	{
		close(pipefd[0]);
		close(pipefd[1]);
		errx(1, "Couldn't fork.");
	}

	//Цялата команда: cut -d ':' -f 7 /etc/passwd | sort | uniq -c | sort -n
	//fork и създаване  на нов pipe за всяка нова команда

	//За всяка команда процесът дете затваря входа на тръбата си и dup2-ва изхода, за да може изхода от изпълнената команда да отиде в тръбата
	//За всяка команда в родителския процес се затваря изхода на тръбата и се dup2-ва входа, за да може следващата pipe-ната команда да получи изхода на предната като вход.
	//Накрая, при последната команда, пишем крайния изход на стандартния изход.

	//Приемам, че незатворените тръби ще бъдат затворени автоматично след като execlp смени binary-то на програмата и се зачистят.

	//cut -d ':' 'f 7 /etc/passwd
	if(child_pid > 0) //cut parent
	{
		close(pipefd[1]);
		if(dup2(pipefd[0], 0) == -1)
		{
			errx(1, "Couldn't dup2.");
		}

		int pipefd2[2];
		if(pipe(pipefd2) == -1)
		{
			errx(1, "Couldn't open pipe 2.");
		}

		//sort
		if(fork() > 0) //sort parent
		{
			close(pipefd2[1]);
			if(dup2(pipefd2[0], 0) == -1)
			{
				errx(1, "Couldn't dup2.");
			}

			int pipefd3[2];
			if(pipe(pipefd3) == -1)
			{
				errx(1, "Couldn't open pipe 3.");
			}

			//uniq -c
			if(fork() > 0) //uniq parent
			{
				close(pipefd3[1]);
				if(dup2(pipefd3[0], 0) == -1)
				{
					errx(1, "Couldn't dup2.");
				}

				int pipefd4[2];
				if(pipe(pipefd4) == -1)
				{
					errx(1, "Couldn't open pipe 4.");
				}

				//sort -n
				if(fork() > 0) //sort -n parent
				{
					close(pipefd4[1]);
					if(dup2(pipefd4[0], 0) == -1)
					{
						errx(1, "Couldn't dup2.");
					}

					char buffer[4096];
					int readBytes;
					while((readBytes = read(pipefd4[0], &buffer, 4096)))
					{
						if(write(1, &buffer, readBytes) != readBytes)
						{
							errx(1, "Couldn't write output.");
						}
					}
				}
				else //sort -n child
				{
					close(pipefd4[0]);
					if(dup2(pipefd4[1], 1) == -1)
					{
						errx(1, "Couldn't dup2.");
					}

					if(execlp("sort", "sort", "-n", (char*)NULL) == -1)
					{
						errx(1, "Couldn't exec sort -n.");
					}
				}
			}
			else //uniq child
			{
				close(pipefd3[0]);
				if(dup2(pipefd3[1], 1) == -1)
				{
					errx(1, "Couldn't dup2.");
				}

				if(execlp("uniq", "uniq", "-c", (char*)NULL) == -1)
				{
					errx(1, "Couldn't exec uniq -c.");
				}
			}
		}
		else //sort child
		{
			close(pipefd2[0]);
			if(dup2(pipefd2[1], 1) == -1)
			{
				errx(1, "Couldn't dup2.");
			}

			if(execlp("sort", "sort", (char*)NULL) == -1)
			{
				errx(1, "Couldn't exec sort.");
			}
		}
	}
	else //cut child
	{
		close(pipefd[0]);
		if(dup2(pipefd[1], 1) == -1)
		{
			errx(1, "Couldn't dup2.");
		}

		if(execlp("cut", "cut", "-d", ":", "-f", "7", "/etc/passwd", (char*)NULL) == -1)
		{
			errx(1, "Couldn't exec cut.");
		}
	}

	return 0;
}
