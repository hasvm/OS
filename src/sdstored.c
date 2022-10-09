#include<sys/wait.h>
#include<sys/stat.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#include<string.h>
#include<stdio.h>

#define MAX_BUFF_SIZE 1024
#define MAX_TASKS 32
#define MAX_CONFIGS 7
#define MAX_RESPONSE_BUFF 12
#define STATUS 1
#define PROCESS 2
#define UPDATE 3
#define CONCLUDED 4
#define HELP 5
#define CMDS_NUM 2

typedef struct configs
{
	int nop;
	int bcompress;
	int bdecompress;
	int gcompress;
	int gdecompress;
	int encrypt;
	int decrypt;
} Configs;

typedef struct queue
{
	int task_num;
	int args_num;
	int priority;
	char *clt_pid;
	char *args[MAX_TASKS];
	struct queue *next;

} Queue;

Queue *createNode(int task_num, char *clt_pid, char *args[MAX_TASKS], int args_num)
{
	Queue *new_node = malloc(sizeof(Queue));

	new_node->task_num = task_num;

	new_node->args_num = args_num;

	new_node->clt_pid = strdup(clt_pid);

	for (int i = 0; i < args_num; i++)
	{
		
		new_node->args[i] = strdup(args[i]);
	}
	new_node->next = NULL;

	return new_node;
}

Queue *addNode(Queue *node, Queue *new_node)
{
	if (node == NULL)
	{
		return new_node;
	}
	Queue *aux_node = node;

	aux_node = aux_node->next;

	while (aux_node != NULL)
	{
		aux_node = aux_node->next;
	}

	aux_node = new_node;

	return node;
}

Queue *removeNode(Queue *node, int task_num)
{

	if (node->task_num == task_num)
	{
		free(node);

		return NULL;
	}

	Queue *prev = node;

	Queue *next = node->next;

	while (next->task_num != task_num || next->next == NULL)
	{
		prev = next;

		next = next->next;
	}

	if (next->task_num == task_num)
	{
		prev->next = next->next;

		free(next);

		return node;
	}

	return NULL;
}

Queue *getNode(Queue *node, int task_num)
{
	Queue *ret;

	if (node->task_num == task_num)
	{
		ret = node;

		node = node->next;

		return ret;
	}

	Queue *prev = node;

	Queue *next = node->next;

	while (next->task_num != task_num || next->next == NULL)
	{
		prev = next;

		next = next->next;
	}

	if (next->task_num == task_num)
	{
		ret = next;

		prev->next = next->next;

		return ret;
	}

	return NULL;
}

int able_to_process(Queue *task, Configs *confs, Configs *in_use)
{
	int num_confs = task->args_num;

	int able_to_process = 1;

	char *trans;

	for (int i = 2; i < num_confs; i++)
	{

		trans = strdup(task->args[i]);

		if (strcmp("nop", trans) == 0)
		{
			if (confs->nop <= in_use->nop)
			{
				able_to_process = 0;

				i = num_confs;
			}
		}

		if (strcmp("bcompress", trans) == 0)
		{
			if (confs->bcompress <= in_use->bcompress)
			{
				able_to_process = 0;

				i = num_confs;
			}
		}

		if (strcmp("bdecompress", trans) == 0)
		{
			if (confs->bdecompress <= in_use->bdecompress)
			{
				able_to_process = 0;

				i = num_confs;
			}
		}

		if (strcmp("gcompress", trans) == 0)
		{
			if (confs->gcompress <= in_use->gcompress)
			{
				able_to_process = 0;

				i = num_confs;
			}
		}

		if (strcmp("gdecompress", trans) == 0)
		{
			if (confs->gdecompress <= in_use->gdecompress)
			{
				able_to_process = 0;

				i = num_confs;
			}
		}

		if (strcmp("encrypt", trans) == 0)
		{
			if (confs->encrypt <= in_use->encrypt)
			{
				able_to_process = 0;

				i = num_confs;
			}
		}

		if (strcmp("decrypt", trans) == 0)
		{
			if (confs->decrypt <= in_use->decrypt)
			{
				able_to_process = 0;

				i = num_confs;
			}
		}

	}

	return able_to_process;
}

Queue *checkIfTaskAvailable(Queue *tasks, Configs *configs, Configs *in_use)
{

	if (tasks == NULL) {

		return NULL;
	}

	Queue *task = tasks;

	if (able_to_process(task, configs, in_use))
	{
		return task;
	}

	while (task->next != NULL)
	{
		if (able_to_process(task->next, configs, in_use))
		{
			return task->next;
		}

		task = task->next;
	}

	return NULL;
}

Configs* update_in_use(Queue *task, Configs *in_use, int val)
{
	int num_confs = task->args_num;

	for (int i = 0; i < num_confs; i++)
	{
		if (strcmp("nop", task->args[i]) == 0)
		{
			in_use->nop += val;
		}

		if (strcmp("bcompress", task->args[i]) == 0)
		{
			in_use->bcompress += val;
		}

		if (strcmp("bdecompress", task->args[i]) == 0)
		{
			in_use->bdecompress += val;
		}

		if (strcmp("gcompress", task->args[i]) == 0)
		{
			in_use->gcompress += val;
		}

		if (strcmp("gdecompress", task->args[i]) == 0)
		{
			in_use->gdecompress += val;
		}

		if (strcmp("encrypt", task->args[i]) == 0)
		{
			in_use->encrypt += val;
		}

		if (strcmp("decrypt", task->args[i]) == 0)
		{
			in_use->decrypt += val;
		}
	}

	return in_use;
}

void set_configuration(Configs *configs, Configs *in_use, char *conf_file, char *path)
{
	char buff[MAX_BUFF_SIZE];

	int fd, bytes_read;

	char *conf, *num;

	int opts;

	fd = open(conf_file, O_RDONLY);

	switch (fd)
	{
		case -1:

			perror("Open configs file");

			break;

		default:

			break;
	}

	while ((bytes_read = read(fd, &buff, MAX_BUFF_SIZE)) > 0)
	{
		char *token = "\n";

		char lines[MAX_CONFIGS][MAX_BUFF_SIZE];

		char *line = strtok(buff, token);

		for(int i = 1; i <=MAX_CONFIGS; i++) {

			strcpy(lines[i-1], line);

			line = strtok(NULL, token);
		}

		token = " ";

		for(int i = 0; i < MAX_CONFIGS; i++){

			conf = strtok(lines[i], token);

			num = strtok(NULL, token);

			opts = atoi(num);

			if ((strcmp(conf, "nop")) == 0)
			{
				configs->nop = opts;

				in_use->bcompress = 0;
			}

			if ((strcmp(conf, "bcompress")) == 0)
			{
				configs->bcompress = opts;

				in_use->bcompress = 0;
			}

			if ((strcmp(conf, "bdecompress")) == 0)
			{
				configs->bdecompress = opts;

				in_use->bdecompress = 0;
			}

			if ((strcmp(conf, "gcompress")) == 0)
			{
				configs->gcompress = opts;

				in_use->gcompress = 0;
			}

			if ((strcmp(conf, "gdecompress")) == 0)
			{
				configs->gdecompress = opts;

				in_use->gdecompress = 0;
			}

			if ((strcmp(conf, "encrypt")) == 0)
			{
				configs->encrypt = opts;

				in_use->encrypt = 0;
			}

			if ((strcmp(conf, "decrypt")) == 0)
			{
				configs->decrypt = opts;

				in_use->decrypt = 0;
			}
		}
	}

	close(fd);
}

Queue *add_tasks_queue(Queue *tasks, Queue *task)
{
	if (tasks == NULL)
	{
		return task;
	}

	else
	{
		Queue *aux = tasks;

		while (aux->next != NULL)
		{
			aux = aux->next;
		}

		aux->next = task;

		return tasks;
	}
}

void sendStatus(Configs *confs, Configs *use, Queue *tasks_status, char *clt_pid)
{
	char status[MAX_BUFF_SIZE];

	char aux_status[MAX_BUFF_SIZE];

	char args[MAX_BUFF_SIZE];

	Queue *aux = tasks_status;

	int fd;

	bzero(args, MAX_BUFF_SIZE);

	bzero(status, MAX_BUFF_SIZE);

	bzero(aux_status, MAX_BUFF_SIZE);

	switch (fork())
	{

	case -1:

		perror("Status process");

		break;

	case 0:

		fd = open(clt_pid, O_WRONLY);

		while(aux != NULL) {

			for(int i = 0; i < aux->args_num; i++) {
				
				if(i == 0) {

					snprintf(args, strlen(aux->args[i])+1, "%s", aux->args[i]);

				} else {
					
					snprintf(args, (strlen(aux->args[i]) + strlen(aux_status) + 2), "%s %s", aux_status, aux->args[i]);
				}

				sprintf(aux_status, "%s", args);
			}

			bzero(aux_status, MAX_BUFF_SIZE);
			
			sprintf(status, "1 Task #%d: proc-file %d", aux->task_num, aux->priority);

			snprintf(aux_status, (strlen(args) + strlen(status) + 5), "%s %s \n", status, args);

			write(fd, &aux_status, strlen(aux_status));

			bzero(status, MAX_BUFF_SIZE);

			bzero(args, MAX_BUFF_SIZE);

			aux = aux->next;
		}

		sprintf(status,"1 transf nop: %d/%d (runnig/max)\n", use->nop, confs->nop);

		write(fd,&status,strlen(status));

		bzero(status, MAX_BUFF_SIZE);

		sprintf(status,"1 transf bcompress: %d/%d (runnig/max)\n", use->bcompress, confs->bcompress);

		write(fd,&status,strlen(status));

		bzero(status, MAX_BUFF_SIZE);

		sprintf(status,"1 transf bdecompress: %d/%d (runnig/max)\n", use->bdecompress, confs->bdecompress);

		write(fd,&status,strlen(status));

		bzero(status, MAX_BUFF_SIZE);

		sprintf(status,"1 transf gcompress:  %d/%d (runnig/max)\n", use->gcompress, confs->gcompress);

		write(fd,&status,strlen(status));

		bzero(status, MAX_BUFF_SIZE);

		sprintf(status,"1 transf gdecompress:  %d/%d (runnig/max)\n", use->gdecompress, confs->gdecompress);

		write(fd,&status,strlen(status));

		bzero(status, MAX_BUFF_SIZE);

		sprintf(status,"1 transf encrypt: %d/%d (runnig/max)\n", use->encrypt, confs->encrypt);

		write(fd,&status,strlen(status));

		bzero(status, MAX_BUFF_SIZE);

		sprintf(status,"1 transf decrypt:  %d/%d (runnig/max)\n", use->decrypt, confs->decrypt);

		write(fd,&status,strlen(status));

		bzero(status, MAX_BUFF_SIZE);

		close(fd);

		_exit(0);

		break;

	default:

		break;
	}
}

void sendHelp(char *clt_pid)
{
	int fd;

	char cmds[CMDS_NUM][MAX_BUFF_SIZE];

	sprintf(cmds[0], "5 ./sdstore status\n");

	sprintf(cmds[1], "5 ./sdstore proc_file priority input-filename output-filename transformation-id-1 transformation-id-2...\n");

	switch (fork())
		{

		case -1:

			perror("Fork");

			break;

		case 0:

			fd = open(clt_pid, O_WRONLY);
		
			for(int i = 0; i < CMDS_NUM; i++){

				write(fd, &cmds[i], strlen(cmds[i]));

			}

			close(fd);

			_exit(0);

			break;

		default:

			break;
	}
}

void process_transformation(Queue *task, char *tranformations_path)
{
	int num_cmds = task->args_num - 2;

	int pipes[num_cmds][2];

	int status[num_cmds];

	int fifo, fd;


		for (int i = 0; i < num_cmds; i++)
		{
			if (i == 0)
			{
				char *file = strdup(task->args[0]);

				char *trans =  strdup(tranformations_path);

				char *arg = strdup(task->args[i + 2]);


				if (pipe(pipes[i]) != 0)
				{
					perror("Pipe");
				}

				switch (fork())
				{
				case -1:

					perror("Fork");

				case 0:

					close(pipes[i][0]);

					dup2(pipes[i][1],1);

					close(pipes[i][1]);

					strcat(trans, "/");

					strcat(trans, arg);

					int fd = open(file, O_RDONLY, 0600);

					dup2(fd,0);

					execlp(trans, arg, NULL);
					
					_exit(0);

				default:

					free(file);

					close(pipes[i][1]);
				}
			}
			else if (i == num_cmds - 1)
			{
				char *arg = strdup(task->args[i+2]);

				char *file = strdup(task->args[1]);

				char *trans = strdup(tranformations_path);

				fd = open(file, O_CREAT | O_WRONLY, 0666);

				switch (fork())
				{
				case -1:

					perror("Fork");

				case 0:

					strcat(trans, "/");

					strcat(trans, arg);

					dup2(pipes[i - 1][0], 0);

					close(pipes[i - 1][0]);

					dup2(fd,1);

					execlp(trans, arg, NULL);

					_exit(0);

				default:

					free(arg);

					free(file);

					free(trans);

					close(pipes[i - 1][0]);
				}
			}
			else
			{
				char *arg = strdup(task->args[i + 2]);

				char *trans = strdup(tranformations_path);

				if (pipe(pipes[i]) != 0)
				{
					perror("Pipe");
				}
				switch (fork())
				{
				case -1:

					perror("Fork");

				case 0:

					strcat(trans, "/");

					strcat(trans, arg);

					close(pipes[i][0]);

					dup2(pipes[i - 1][0], 0);

					close(pipes[i - 1][0]);

					dup2(pipes[i][1],1);

					close(pipes[i][1]);

					execlp(trans, arg, NULL);

					_exit(0);

				default:

					free(arg);

					free(trans);

					close(pipes[i][1]);

					close(pipes[i-1][0]);
				}
			}
		}
		
		for(int j=0; j < num_cmds; j++) {
			
			wait(&status[j]);
		}

		char *finish;

		finish = strdup("4 ");

		strcat(finish, task->clt_pid);

		char num_task[4];

		sprintf(num_task, " %d", task->task_num);

		strncat(finish, num_task, strlen(num_task));

		fifo = open("fifo", O_WRONLY);

		write(fifo, finish, strlen(finish));

		close(fifo);
}

int create_controller(int pipe_controller[], Configs *configuration, Configs *in_use, char *tranformations_path)
{
	char *args[MAX_TASKS];

	char *opt, *concluded_task, *next_arg, *clt_pid;

	int bytes_read, pipe_clt, args_num, task_num = 0, flag = 1;

	char buffer[MAX_BUFF_SIZE];

	char aux_buffer[MAX_BUFF_SIZE];

	char *token = " ";

	Queue *task = NULL, *tasks_processing = NULL, *tasks_pending = NULL;

	bzero(buffer, MAX_BUFF_SIZE);

	while (flag)
	{
		if ((bytes_read = read(pipe_controller[0], &buffer, MAX_BUFF_SIZE)) > 0)
		{

			strcpy(aux_buffer, buffer);

			opt = strtok(aux_buffer, token);

			switch (atoi(opt)){

				case UPDATE:

					concluded_task = strtok(NULL, token);
					
					task = getNode(tasks_processing, atoi(concluded_task));

					in_use = update_in_use(task, in_use, -1);

					task = checkIfTaskAvailable(tasks_pending, configuration, in_use);

					if (task != NULL)
					{
						in_use = update_in_use(task, in_use, 1);

						tasks_processing = addNode(tasks_processing, task);

						process_transformation(task, tranformations_path);

						pipe_clt = open(task->clt_pid, O_WRONLY);

						write(pipe_clt, "6 Processing\n", strlen("6 Processing\n"));

						close(pipe_clt);
					}

					tasks_processing = removeNode(tasks_processing, atoi(concluded_task));

					bzero(buffer, MAX_BUFF_SIZE);

					bzero(aux_buffer, MAX_BUFF_SIZE);

					break;

				case PROCESS:

					args_num = 0;

					clt_pid = strtok(NULL, token);

					next_arg = strtok(NULL, token);

					while(next_arg != NULL) {
						
						args[args_num] = strdup(next_arg);

						next_arg = strtok(NULL, token);

						args_num++;

					}

					task_num++;
		
					task = createNode(task_num, clt_pid, args, args_num);

					if (able_to_process(task, configuration, in_use)) {

						update_in_use(task, in_use, 1);
						
						tasks_processing = addNode(tasks_processing, task);

						switch(fork()){

							case -1:

								perror("fork");

							case 0:
							
								process_transformation(task, tranformations_path);

								_exit(0);

							default:

								break;
						}

						pipe_clt = open(task->clt_pid, O_WRONLY);

						write(pipe_clt, "7 Processing\n", strlen("7 Processing\n"));

						close(pipe_clt);

					} else {

						Queue *task_pending = malloc(sizeof(Queue));

						task_pending = task;

						tasks_pending = addNode(tasks_pending, task_pending);

					}

					bzero(buffer, MAX_BUFF_SIZE);

					bzero(aux_buffer, MAX_BUFF_SIZE);

					break;

				case STATUS:

					clt_pid = strtok(NULL, token);

					sendStatus(configuration, in_use, tasks_processing, clt_pid);

					bzero(buffer, MAX_BUFF_SIZE);

					bzero(aux_buffer, MAX_BUFF_SIZE);

					break;

				case HELP:

					clt_pid = strtok(NULL, token);

					sendHelp(clt_pid);

					bzero(buffer, MAX_BUFF_SIZE);

					bzero(aux_buffer, MAX_BUFF_SIZE);

					break;

				default:

					bzero(buffer, MAX_BUFF_SIZE);

					bzero(aux_buffer, MAX_BUFF_SIZE);
					
					break;
			}
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int fifo_read, fifo_write, pipe_clt, pipe_controller[2];

	size_t bytes_read;

	char *config, *path, *clt_pid, *opt, *processed_task;

	char buffer[MAX_BUFF_SIZE], aux_buffer[MAX_BUFF_SIZE];

	char *token = " ";

	Configs *configuration, *in_use;

	if ((mkfifo("fifo", 0666)) == -1)
	{
		perror("fifo already created\n");
	}

	else
	{
		perror("Fifo created\n");
	}

	config = argv[1];

	path = argv[2];

	configuration = malloc(sizeof(Configs));

	in_use = malloc(sizeof(Configs));


	set_configuration(configuration, in_use, config, path);
	
	
	if ((pipe(pipe_controller)) == -1)
	{
		perror("Pipe Controller");
	}
	
	switch (fork())
	{
		case -1:

			perror("Erro a criar processo filho controller\n");

			break;

		case 0:

			close(pipe_controller[1]);

			create_controller(pipe_controller, configuration, in_use, path);

			break;

		default:

			close(pipe_controller[0]);

			break;
	}

	if ((fifo_read = open("fifo", O_RDONLY)) == -1)
	{
		perror("FIFO");
	}

	if ((fifo_write = open("fifo", O_WRONLY)) == -1)
	{
		perror("FIFO");
	}

	bzero(&buffer, MAX_BUFF_SIZE);

	bzero(&aux_buffer, MAX_BUFF_SIZE);
	
	while ((bytes_read = read(fifo_read, &buffer, MAX_BUFF_SIZE)) > 0)
	{
		strcpy(aux_buffer, buffer);

		opt = strtok(buffer, token);

		switch (atoi(opt))
		{
			case STATUS:

				write(pipe_controller[1], &aux_buffer, bytes_read);

				break;

			case PROCESS:

				clt_pid = strtok(NULL, token);

				pipe_clt = open(clt_pid, O_WRONLY);

				write(pipe_clt, "6 Pending\n", strlen("6 Pending\n"));

				close(pipe_clt);

				write(pipe_controller[1], &aux_buffer, bytes_read);

				break;

			case CONCLUDED:

				clt_pid = strtok(NULL, token);

				pipe_clt = open(clt_pid, O_WRONLY);

				write(pipe_clt, "8 Concluded\n", strlen("8 Concluded\n"));

				close(pipe_clt);

				bzero(aux_buffer, MAX_BUFF_SIZE);

				processed_task = strtok(NULL, token);

				sprintf(aux_buffer, "3 %s", processed_task);

				write(pipe_controller[1], &aux_buffer, bytes_read);

				bzero(aux_buffer, MAX_BUFF_SIZE);

				bzero(buffer, MAX_BUFF_SIZE);

				break;

			case HELP:

				clt_pid = strtok(NULL, token);

				write(pipe_controller[1], &aux_buffer, bytes_read);

			default:

				break;
		}

		bzero(&buffer, MAX_BUFF_SIZE);

		bzero(&aux_buffer, MAX_BUFF_SIZE);
	}

	close(pipe_controller[1]);

	close(fifo_read);

	close(pipe_controller[1]);

	close(fifo_write);

	unlink("fifo");

	return 0;
}