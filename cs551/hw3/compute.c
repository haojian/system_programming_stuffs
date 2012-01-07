/**************Statement of Policy****************************/
// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
// A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Haojian Jin
/*************************************************************/

#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "bitmap.h"

#define KEY (key_t)54545   /*Key for shared memory segment */
#define SERVER_FIFO_NAME "fifo_server"
#define PERM_FILE		(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)


/***********Global Variable***********************/
	
	typedef struct{
		pid_t compute_pid;
		int total_perfectnums;
		long tested_candidates;
		long untested_candidates;
	}process_info;
	typedef struct{
		pid_t computer_pid;
		long message;
	}compute_message;

	int fd_server, fd_client = -1, fd_client_w = -1;
	ssize_t nread;
	char fifo_name[100];
	int i; // Loop conter

	int sid; /* segment id of shared memory segment */
	unsigned char *bitmap_array; /* bitmap */
	long *perfectnum_array; /* result array */
	process_info *process_Info;  /* precess array */
	pid_t *manage_pid, *cur_pid;
	compute_message msg2manager;

	int start_number=0;
	process_info *curProcess_Info;
	int cur_index_process_entries = -1 ;

/*************************************************/

static void clean_up(void)
{
	printf("Compute: %ld has been cleaned up\n", getpid());
	
	if(cur_index_process_entries == -1)
		exit(0);
	
	//Delete the process entry
	for(i=cur_index_process_entries; i<20-1; i++)
	{
		(*(process_Info+i)).compute_pid = (*(process_Info+i+1)).compute_pid;
		(*(process_Info+i)).total_perfectnums = (*(process_Info+i+1)).total_perfectnums;
		(*(process_Info+i)).tested_candidates = (*(process_Info+i+1)).tested_candidates;
		(*(process_Info+i)).untested_candidates = (*(process_Info+i+1)).untested_candidates;
	}

	(*(process_Info+19)).compute_pid = NULL;
	(*(process_Info+19)).total_perfectnums = 0;
	(*(process_Info+19)).tested_candidates = 0;
	(*(process_Info+19)).untested_candidates = 0;


	exit(0);
}

static void handler(int signum)
{
	printf("End sig invoked\n");
	clean_up();
}
 
static int handle_signals(void)
{
	sigset_t set;
	struct sigaction act;
	
	sigfillset(&set);
	sigprocmask(SIG_SETMASK, &set, NULL);
	memset(&act, 0, sizeof(act));
	sigfillset(&act.sa_mask);
	act.sa_handler = handler;
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGQUIT, &act, NULL);
	sigaction(SIGHUP, &act, NULL);
	sigemptyset(&set);
	sigprocmask(SIG_SETMASK, &set, NULL);
	return 1;
}


int check_perfect(int num)
{
	int sum, cur_factor;
	int result = 0;
	sum = 0;
	for(cur_factor = 1; cur_factor <=(num -1); cur_factor++)
	{
		if((num%cur_factor) ==0)
		{
			sum += cur_factor;
		}
	}
	if(sum == num)
	{
		return sum;
	}
	else
		return 0;
}

int make_fifo_name(pid_t pid, char *name, size_t name_max)
{
	snprintf(name, name_max, "fifo%ld", (long)pid);
	return 1;
}

int recv_confirmation(int sig_num)
{
	if(sig_num > 0)
	{
		printf("Client %ld :  submitted perfect number %d successfully!\n", (long)getpid(), sig_num);
	}
	else if(sig_num < 0)
	{
		printf("Client %ld :  registered successfully, the index of corresponding process entry is %d!\n", (long)getpid(), (-sig_num));
		cur_index_process_entries = -sig_num;
		curProcess_Info = process_Info - sig_num -1;
	}
	else
		printf("Error in communication!\n");
}

int send_message_to_manager(compute_message msg)
{
	write(fd_server, &msg, sizeof(msg));
	if(fd_client == -1)
		fd_client = open(fifo_name, O_RDONLY);
	if(fd_client_w == -1)
		fd_client_w = open(fifo_name, O_WRONLY);
	nread = read (fd_client, &msg, sizeof(msg));
	if(nread == 0)
		errno = ENETDOWN;
	recv_confirmation((int)msg.message);
}

int compute_perfect_number()
{
	printf("start computing");
	long bit_index = 0;
	int prefect_result=0;
	printf("%ld\n", (*curProcess_Info).untested_candidates );
	while((*curProcess_Info).untested_candidates !=0 )
	{
		if(bitmap_get(bit_index, bitmap_array) ==0)
		{
			bitmap_set(bit_index, bitmap_array);
			(*curProcess_Info).untested_candidates--;
			(*curProcess_Info).tested_candidates++;
			if((prefect_result=check_perfect(start_number + bit_index))>0)
			{
				msg2manager.computer_pid = getpid();
				msg2manager.message = prefect_result;
				send_message_to_manager(msg2manager);
			}
		}
		bit_index++;
		if(bit_index >= 256000)
			return;
	}
}

int main(argc, argv)
int argc;
char **argv;
{
	handle_signals();
	//Analyze the arguments
	if(argc == 2 && ((start_number = atoi(argv[1]))>=0))
	{
		printf("Start number of this process is %d", start_number);
	}

	//Set up the memory space pointer
	if((sid = shmget (KEY, 20 * sizeof(long) + 20 * sizeof(process_info) + sizeof(pid_t) + (256000/8+1)*sizeof(unsigned char), IPC_CREAT | 0660))==-1)
	{
		perror("shmget");
		exit(1);
	}

	if((bitmap_array = ((unsigned char*)shmat(sid, NULL, 0))) == (unsigned char *) -1)
	{
		perror("bitmap shmat");
		exit(2);
	}

	bitmap_init(256000, bitmap_array);

	perfectnum_array = (long *)(bitmap_array+ 256000/8+1);
	process_Info = (process_info *) (perfectnum_array + 20);
	
	manage_pid = (pid_t *)(process_Info+20);

	//Set up communications.
	printf("Client %ld started!\n", (long)getpid());
	msg2manager.computer_pid = getpid();
	make_fifo_name(msg2manager.computer_pid, fifo_name, sizeof(fifo_name));

	if(mkfifo(fifo_name, PERM_FILE) == -1 && errno != EEXIST)
	{
		printf("Error in FIFOs");
	}
	fd_server = open(SERVER_FIFO_NAME, O_WRONLY);
	msg2manager.computer_pid = getpid();
	msg2manager.message = 0;
	send_message_to_manager(msg2manager);

	//Compute the perfect number
	compute_perfect_number();
	
	close(fd_server);
	close(fd_client);
	close(fd_client_w);
	unlink(fifo_name);
	printf("Client %ld done! \n", (long)getpid());
	exit(0);
}
