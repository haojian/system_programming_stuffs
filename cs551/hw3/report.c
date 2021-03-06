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

#define KEY (key_t)54545   /*Key for shared memory segment */

	typedef struct{
		pid_t compute_pid;
		int total_perfectnums;
		long tested_candidates;
		long untested_candidates;
	}process_info;

int main(argc, argv)
int argc;
char **argv;
{
	int sid;	/* segment id of shared memory segment */
	unsigned char *bitmap_array; /* bitmap */
	long *perfectnum_array; /* result array */
	process_info *process_Info;  /* precess array */
	pid_t *manage_Pid;	
	int i;
		
			/* create shared segment if necessary */
	if((sid = shmget (KEY, 20 * sizeof(long) + 20 * sizeof(process_info) + sizeof(pid_t) + (256000/8+1)*sizeof(unsigned char), IPC_CREAT | 0660))==-1)
	{
		perror("shmget");
		exit(1);
	}

			/* map it into our address space*/
	if ((bitmap_array=((unsigned char *) shmat(sid,0,0)))== (unsigned char *)-1) {
		perror("shmat");
		exit(2);
	}

	perfectnum_array = (long *)(bitmap_array+ 256000/8+1);
	process_Info = (process_info *) (perfectnum_array + 20);
	manage_Pid = (pid_t *)(process_Info+20);

	//print the out put.
	printf("Computed perfect numbers:");
	for(i=0; i<20; i++)
	{
		if(perfectnum_array[i] != 0)
			printf("%ld      ", perfectnum_array[i]);
	}
	printf("\n");
	long tested_totalnum=0;
	for(i=0; i<20; i++)
	{
		if((*(process_Info+i)).tested_candidates != 0)
			tested_totalnum+= (*(process_Info+i)).tested_candidates;
	}
	printf("Tested total numbers: %d .\n", tested_totalnum);
	printf("Processes currently computing (Process ids): ");
	for(i=0; i<20; i++)
	{
		if((*(process_Info+i)).compute_pid == 0)
			break;
		printf("%ld           ", (*(process_Info+i)).compute_pid);
	}
	printf("\n");
	if(argc == 2 && !strcmp(argv[1], "-k"))
	{
		printf("Manager %ld was killed by reporter.\n", *manage_Pid);
		kill(*manage_Pid, SIGINT);
	}
}
