/**************Statement of Policy****************************/
// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
// A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Haojian Jin
/*************************************************************/

#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include <time.h>
#include "threadbitmap.h"

/***********Global Variable***********************/
typedef struct{
	pthread_t thread_id;
	int thread_num;
	int perfect_num_res[100];
	int total_tested;
	int total_skipped;
	int isIdle ;
	int cur_block_index;
	int total_perfnum_curthread;
}thread_info;

long MAX_NUM = 500, BLOCK_FACTOR = 10; /* Default value */ 
sigjmp_buf jmpenv;

int total_num_tested;
thread_info *thread_info_array; /* thread array */
int total_thread = 0;
int n; /* global variable indicating current test point */
int isIdle = 0;
int i; // Loop counter

pthread_mutex_t mtx[20];
pthread_cond_t cond[20];

time_t process_starttime;

clock_t start, end;
double cpu_time_used;

/*************************************************/

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

int get_total_tested_count()
{
	int tmp_sum=0;
	for(i=0; i<total_thread ; i++)
	{
		tmp_sum+=(*(thread_info_array+i)).total_tested;
	}
	return tmp_sum;
}


void quit()
{
	printf("Quit\n");
	//Report summary
	printf("Pefect Number list:");
	for(i=0; i<total_thread ; i++)
	{
		int j;
		for(j=0; j< (*(thread_info_array+i)).total_perfnum_curthread; j++){
			printf("%d     ", (*(thread_info_array+i)).perfect_num_res[j]);
		}
	}
	printf("\n");

	//Report time;
	printf("Total number tested is: %d .\n", get_total_tested_count());
	printf("Elapsed time is %f seconds. \n", difftime(time(NULL), process_starttime));

	end = clock();
     	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
	printf("CPU time is %f seconds. \n", cpu_time_used);

	exit(0);
}

void *compute_func(void *_thread_info)
{
	thread_info *cur_info = (thread_info *)_thread_info;
       	int cur_index = (*cur_info).cur_block_index;
	int thread_index = (*cur_info).thread_num;
	int skipped_block_counter = 0;
	printf("cur_index : %d, thread_index : %d\n", cur_index , thread_index);
	
	
	while(1){
		pthread_mutex_lock(&mtx[thread_index]);
		while((*(thread_info_array + thread_index)).isIdle)
		{
			pthread_cond_wait(&cond[thread_index], &mtx[thread_index]);
		}
		pthread_mutex_unlock(&mtx[thread_index]);

		if(bitmap_get(cur_index) ==0)
		{
			bitmap_set(cur_index);
		
			int start_num =  (cur_index - 1) * BLOCK_FACTOR + 1;
			
			(*(thread_info_array + thread_index)).cur_block_index = cur_index;
		
			for(i=0; i<BLOCK_FACTOR; i++)
			{	//printf("%d", (start_num + i));
				if(check_perfect(start_num + i))
				{
					//update perfect number;
					(*(thread_info_array + thread_index)).perfect_num_res[(*(thread_info_array+thread_index)).total_perfnum_curthread] = start_num + i;
					
					(*(thread_info_array+thread_index)).total_perfnum_curthread++;
					printf("total perf:%d | %d\n", (start_num + i),(*(thread_info_array+thread_index)).total_perfnum_curthread);
				}
				//update total tested number;
				(*(thread_info_array+thread_index)).total_tested++;
				//printf("total tested number: %d", get_total_tested_count());
			
				if((start_num + i) == MAX_NUM)
				{
					cur_index = 0;
					break;
				}
			}
			cur_index++;
			(*(thread_info_array + thread_index)).cur_block_index = cur_index;
			
		}
		else
		{
			//update total skipped number
			//printf("skipped once");
			(*(thread_info_array+thread_index)).total_skipped += BLOCK_FACTOR;
			cur_index++;
			skipped_block_counter++;
		}
		if(get_total_tested_count() >= MAX_NUM)
		{
			//printf("%d", get_total_tested_count());
			//printf("all the number has been tested.");
			cur_index = 1;
			quit();
		}
		
	}
	return (void *)0;
}

void start_K(int _k)
{
	pthread_t tid;
	if(_k==0)
		_k=1;
	int cur_index = (int)ceil((double)_k/BLOCK_FACTOR);
	//int cur_index = 0;
 
	(*(thread_info_array+total_thread)).thread_num = total_thread;
	(*(thread_info_array+total_thread)).cur_block_index = cur_index;

	//pthread_mutex_t mutex;
	//&mtx[total_thread] = (pthread_mutex_t *)malloc(sizeof(szieof(pthread_mutex_t)) );
	pthread_mutex_init (&mtx[total_thread], NULL);
	//(cond+total_thread) = (pthread_cond_t *)malloc(sizeof(szieof(pthread_cond_t)) );
	pthread_cond_init(&cond[total_thread], NULL);

	if(pthread_create(&tid, NULL, compute_func, (void *)(thread_info_array+total_thread))==0){
		(*(thread_info_array+total_thread)).thread_id = tid;		
		total_thread++;
		printf("New thread has been started. Num: %d\n", total_thread);
	}
	
}

void idle_N(int _n)
{
	_n = _n-1;
	
	pthread_mutex_lock(&mtx[_n]);
	(*(thread_info_array + _n)).isIdle = 1;
	pthread_mutex_unlock(&mtx[_n]);
}

void restart_N(int _n)
{
	_n = _n-1;
	pthread_mutex_lock(&mtx[_n]);
	(*(thread_info_array + _n)).isIdle = 0;
	pthread_cond_signal(&cond[_n]);
	pthread_mutex_unlock(&mtx[_n]);
}

void wait_S(int _s)
{
	printf("Wait for: %d seconds for additional commands\n", _s);
	sleep(_s);
}

void report()
{
	printf("-------------Report Start-----------\n");
	int tmp_sum=0;
	for(i=0; i<total_thread ; i++)
	{
		printf("Thread %d has tested %ld numbers, skipped %ld numbers, and is working on block %d. ", (*(thread_info_array+i)).thread_num, (*(thread_info_array+i)).total_tested, (*(thread_info_array+i)).total_skipped, (*(thread_info_array+i)).cur_block_index);
		if((*(thread_info_array+i)).isIdle)
			printf("Currently, it's idle. \n");
		else
			printf("Currently, it's not idle. \n");
		printf("%d", (*(thread_info_array+i)).total_perfnum_curthread);
		int j;
		for(j=0; j< (*(thread_info_array+i)).total_perfnum_curthread; j++){
			printf("Perfect number %d is found by Thread %d. \n", (*(thread_info_array+i)).perfect_num_res[j], (*(thread_info_array+i)).thread_num);
		}
	}
	printf("-------------Report End-----------\n");
}

void process_cmd(char* command)
{
	if(strncmp(command,"start",5) == 0) 
	{
		int argu = atoi(command+6);
		start_K(argu);
	}
	else if(strncmp(command, "idle", 4) == 0)
	{
		int argu = atoi(command + 5);
		idle_N(argu);
	}
	else if(strncmp(command, "restart", 7) == 0)
	{
		int argu = atoi(command + 8);
		restart_N(argu);
	}
	else if(strncmp(command, "wait", 4) == 0)
	{
		int argu = atoi(command + 5);
		wait_S(argu);
	}
	else if(strncmp(command, "report", 6) == 0)
	{
		report();
	}
	else if(strncmp(command, "quit", 4) == 0)
	{
		quit();
	}
}


int main(argc, argv)
int argc;
char **argv;
{
	process_starttime = time(NULL);
	start = clock();
	bitmap_init(256000);
	//for(i=0; i < 100; i++)
	//	printf("%d", bitmap_get(i));
	char command[100];

	thread_info *tmparray = (thread_info *)malloc(40 * sizeof(thread_info));
	thread_info_array = tmparray;
	if(argc == 3 && ((MAX_NUM = atol (argv[1])) >=0) && ((BLOCK_FACTOR = atol(argv[2])) >=0))
		printf("MAX is %ld, BLOCK factor is %ld\n", MAX_NUM, BLOCK_FACTOR);
	
	sigset_t mask;
	struct sigaction action;

	if(sigsetjmp(jmpenv, 0)){
		printf("Enter the commands: start K; idle N; restart N; wait S; report; quit \n");
		fgets(command, 100, stdin);
		process_cmd(command);
	}
	
	siglongjmp(jmpenv,1);
}
