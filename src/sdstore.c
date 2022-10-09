#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

#define MAX_BUFFER_SIZE 1024
#define STATUS 1
#define PROCESS 2
#define HELP 5
#define PENDING 6
#define PROCESSING 7
#define CONCLUDED 8

int main(int argc, char *argv[])
{
    int fifo_read, fifo_write, pid, bytes_read, valid_cmd, open_file;

    char buffer[MAX_BUFFER_SIZE];
    
    char aux_buffer[MAX_BUFFER_SIZE];

    char char_pid[8];

    char *pid_str, *token = " ", *opt;

    bzero(buffer, MAX_BUFFER_SIZE);

    pid = getpid();

    sprintf(char_pid,"%d", pid);

    pid_str = strndup(char_pid, strlen(char_pid));

    switch (argc)
    {

    case 1:

        sprintf(buffer, "5 %d", pid);

        break;

    default:

        if (strcmp(argv[1], "status") == 0)
        {
            switch (argc)
            {
            case 2:
                sprintf(buffer, "1 %d", pid);

                break;

            default:

                sprintf(buffer, "5 %d", pid);

                break;
            }
        }
        else
        {
            valid_cmd = 0;

            if (strcmp("proc-file", argv[1]) != 0)
            {
                valid_cmd = 1;
            }
            else if ((open_file = open(argv[2], O_RDONLY)) == -1)
            {
                valid_cmd = 1;
            }
            else
            {
                close(open_file);

                sprintf(buffer, "2 %d ", pid);
                
                strcat(buffer, argv[2]);

                strcat(buffer, " ");

                strcat(buffer, argv[3]);

                for (int i = 4; i < argc; i++)
                {
                    if (strcmp("nop", argv[i]) == 0 || strcmp("bcompress", argv[i]) == 0 || strcmp("bdecompress", argv[i]) == 0 || strcmp("gcompress", argv[i]) == 0 || strcmp("gdecompress", argv[i]) == 0 || strcmp("encrypt", argv[i]) == 0 || strcmp("decrypt", argv[i]) == 0)
                    {
                        strcat(buffer, " ");

                        strcat(buffer, argv[i]);

                    } else {

                        valid_cmd = 1;

                        i = argc;
                    } 
                }
            }
            if (valid_cmd == 1)
            {
                bzero(buffer, MAX_BUFFER_SIZE);

                sprintf(buffer, "5 %d", pid);
            }
        }
        break;
    }

    mkfifo(pid_str, 0666);

    fifo_write = open("fifo", O_WRONLY);

    write(fifo_write, &buffer, strlen(buffer));

    bzero(buffer, MAX_BUFFER_SIZE);

    close(fifo_write);

    fifo_read = open(pid_str, O_RDONLY);

    fifo_write = open(pid_str, O_WRONLY);
    
    while((bytes_read = read(fifo_read, &buffer, MAX_BUFFER_SIZE)) > 0)
    {
        strcpy(aux_buffer, buffer);

		opt = strtok(aux_buffer, token);

		switch (atoi(opt))
		{   
            case PENDING:

            case PROCESSING:

                write(1, &buffer, bytes_read);

                break;

            case HELP:

            case CONCLUDED:

            case STATUS:

            default:

                write(1, &buffer, bytes_read);

                close(fifo_read);

                close(fifo_write);

                unlink(pid_str);

                free(pid_str);
        }

        bzero(buffer, MAX_BUFFER_SIZE);

        bzero(aux_buffer, MAX_BUFFER_SIZE);
    }

    return 0;
}
