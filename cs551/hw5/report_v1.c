/**************Statement of Policy****************************/
// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
// A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Haojian Jin
/*************************************************************/


#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/time.h>
#include <signal.h>
/***********Global Variable***********************/
	typedef struct{
		long *perfectnum_array;
		int perfectnum_counter;
		int hostname[40];
		long tested_candidates;
		char *current_range;
	}host_info;
	
	host_info *host_infolist;

int s;			//socket file descriptor


/*************************************************/

int main (int argc,char *argv[])
{
	struct sockaddr_in sin; /* socket address for destination */
	int len;		//length of address
	long address;
	long start_point;
	int i;
	char buf[100];
	long range_size = -1;

	/* Fill in Manager's Address */
	address = *(long *) gethostbyname(argv[1])->h_addr;
	sin.sin_addr.s_addr= address;
	sin.sin_family= AF_INET;
	sin.sin_port = atoi(argv[2]);

	while(1) { /*loop waiting for Manager if Necessary */		
			/* create the socket */
	if ((s = socket(AF_INET,SOCK_STREAM,0)) < 0) {
		perror("Socket");
		exit(1);
		}

		/* try to connect to Manager */
	if (connect (s, (struct sockaddr *) &sin, sizeof (sin)) <0) {
		printf("Where is that Manager!\n");
		close(s);
		sleep(10);
		continue;
		}
	break; /* connection successful */
	}
	printf("connection successful \n");
	
	/* Register on the manager, and get the range */
	
	if(argc == 4 && !strcmp(argv[3], "-k"))
	{
		printf("Manager was killed by reporter.\n");
		write(s, "k", 1);
	}

	else
		write(s, "s", 1);
	int nread ;
	while(nread= read(s, buf, 100))
		printf("%s\n", buf);
	exit(0);
}
