/**************Statement of Policy****************************/
// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
// A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Haojian Jin
/*************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>
#include <stropts.h>
#include <poll.h>
#include <signal.h>
#include "rangeschedule_v3.h"

#define MAXCLIENTS 6

/***********Global Variable***********************/
	typedef struct{
		int iscompute;
		char perfectnum_array[100] ;
		int perfectnum_counter;
		int hostname[40];
		long tested_candidates;
		range_info current_range;
		char *range_history;
	}host_info;
	int total_perf=0;
	host_info *host_infolist;
	int johns[MAXCLIENTS][40];	
	char *reportStr;	
	int iskilledbyreporter = 0;	
	int hostnum=0;  //compute host num
	int report_index = -1;

	struct pollfd pollarray[MAXCLIENTS];     /* up to 1+5 simultaneous connections*/
/*************************************************/

char *reportGeneration()
{
	char report[1000];
	int j;
	for(j=0; j<MAXCLIENTS; j++)
	{
		if((*(host_infolist+j)).iscompute == 1){

			snprintf(report, sizeof(report), "---------------------------------------------\n Perfect Numbers: %s are found by Machine %d\nthe name of machine %d is %s\nThis machine has tested %ld numbers\n Currently, it is working on range from %ld to %ld\n---------------------------------------------\n", (*(host_infolist+j)).perfectnum_array,j, j, &johns[j][0],(*(host_infolist+j)).tested_candidates,(*(host_infolist+j)).current_range.start_point,(*(host_infolist+j)).current_range.end_point);
		}
		
	}
	//report ="Report---End!!";
	return report;
}

void handler(int signum)
{
	printf("sig invoked\n");
	int j;
	for(j =1 ; j<MAXCLIENTS;j++){
		if((*(host_infolist+j)).iscompute == 1){
			write(pollarray[j].fd, "k", 1);
		}
		iskilledbyreporter = 0;
	}
	exit(0);
}

int handle_signals(void)
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


main (argc,argv) 
	int argc;
	char *argv[];
{

	handle_signals();

	host_infolist = (host_info *)malloc(MAXCLIENTS * sizeof(host_info));
	range_list_init();
	struct sockaddr_in sin; /* structure for socket address */
	int s;
	int fd;
	int len, i,num,count;

	int sum[MAXCLIENTS];						/* sum [1..5] for the 5 johns */
	
	struct hostent *hostentp;

		/* set up IP addr and port number for bind */
	sin.sin_addr.s_addr= INADDR_ANY;
	sin.sin_port = atoi(argv[1]);

		/* Get an internet socket for stream connections */
	if ((s = socket(AF_INET,SOCK_STREAM,0)) < 0) {
		perror("Socket");
		exit(1);
		}

		/* Do the actual bind */
	if (bind (s, (struct sockaddr *) &sin, sizeof (sin)) <0) {
		perror("bind");
		exit(2);
		}

		/* Allow a connection queue for up to 5 JOHNS */
	listen(s,5);

		/* Initialize the pollarray */
	pollarray[0].fd=s;     /* Accept Socket*/
	pollarray[0].events=POLLIN;
						  /* 5 possible john's */
	for (i=1;i<=5;i++) {pollarray[i].fd=-1;pollarray[i].events=POLLIN;}

	while(1) {
		poll(pollarray,6,-1);   /* no timeout, blocks until some activity*/

				/* Check first for new connection */
		if (pollarray[0].revents & POLLIN) {
			
			len=sizeof(sin);
			if ((fd= accept (s, (struct sockaddr *) &sin, &len)) <0) {
				perror ("accept");
					exit(3);
				}
					/* Find first available slot for new john */
			for (i=1;i<=5;i++) if (pollarray[i].fd == -1) break;
				pollarray[i].fd=fd;
				sum[i]=0;
				hostentp=gethostbyaddr((char *)&sin.sin_addr.s_addr,
					sizeof(sin.sin_addr.s_addr),AF_INET);
				strcpy(&johns[i][0], hostentp->h_name);
			}

				/* If there are no new connections, process waiting john's */
		else for (i=1;i<=5;i++) {
			if ((pollarray[i].fd !=-1) && pollarray[i].revents) {
				char buf[100];
				count=read(pollarray[i].fd, buf, 100);
				if (count > 0) 
				{
					int j=0;
					char *pch = strtok(buf,"|");
					switch(*pch)
					{
						case 'r': 	//register
							pch = strtok (NULL, "|");
							long r_start_point = atol(pch); 
							pch = strtok (NULL, "|");
							long r_range_size = atol(pch);
							range_info tmpRange = newRange(r_start_point, 50000);
							while(tmpRange.start_point == tmpRange.end_point)
								tmpRange = newRange(tmpRange.start_point , 50000);
							snprintf(buf, sizeof(buf), "r|%ld|%ld", tmpRange.start_point, tmpRange.end_point-tmpRange.start_point);
							write(pollarray[i].fd, buf, 100);
							(*(host_infolist+i)).tested_candidates += tmpRange.end_point-tmpRange.start_point;
							(*(host_infolist+i)).iscompute = 1;
							(*(host_infolist+i)).current_range = tmpRange;
							hostnum++;
							break;
						case 's':	//report
							//char *tmpstr;// =  reportGeneration();
							
							reportStr = reportGeneration();
							write(pollarray[i].fd, reportStr, strlen(reportStr)+1);
							report_index = i;
							break;
						case 'k':       //killed by reporter
							printf("Kill cmd received!\n");
							for(j =0 ; j<MAXCLIENTS;j++){
								if((*(host_infolist+j)).iscompute == 1){
									printf("send msg to Machine %d\n", j);
									write(pollarray[j].fd, "k", 1);
								}
								
							}
							iskilledbyreporter = 1;
							report_index = i;
							
							break;
						case 'p':	//report number
							pch = strtok (NULL, "|");
							long perf_num = atol(pch); 
							total_perf++;
							
							
  							strcat ((*(host_infolist+i)).perfectnum_array,"|");
							strcat ((*(host_infolist+i)).perfectnum_array, pch);

							//printf("%s\n", (*(host_infolist+i)).perfectnum_array);
							break;
						case 'u':	// update request.
							pch = strtok (NULL, "|");
							long u_start_point = atol(pch); 
							pch = strtok (NULL, "|");
							long u_range_size = atol(pch);
							//printf("%ld|%ld", u_start_point, u_range_size);
							tmpRange = newRange(u_start_point, u_range_size);
							snprintf(buf, sizeof(buf), "u|%ld|%ld", tmpRange.start_point, tmpRange.end_point-tmpRange.start_point);

							write(pollarray[i].fd, buf, 100);

							(*(host_infolist+i)).tested_candidates += u_range_size;
							(*(host_infolist+i)).current_range = tmpRange;
							break;
						case 'l':	//last number
							pch = strtok (NULL, "|");
							printf("%s\n", pch);
							long last_number = atol(pch); 
							printf("The compute %d has been terminated, the last tested number is: %ld \n", i, last_number);
							hostnum--;
							//printf("%d, %d", iskilledbyreporter, hostnum);
							if(iskilledbyreporter==1 && hostnum ==0)
							{
									reportStr = reportGeneration();
									write(pollarray[report_index].fd, reportStr, strlen(reportStr)+1);									exit(0);
							}
							break;

					}
					
					
				}
				else {
					//printf("John %d (%s) has sum %d\n",i,&johns[i][0], sum[i]);
					//sum[i]=0;
					close(pollarray[i].fd);
					pollarray[i].fd = -1;
					}
				}
			}
		}
}
