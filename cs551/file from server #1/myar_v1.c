/**************Statement of Policy****************************/
// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
// A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Haojian Jin
/*************************************************************/


#include <stdio.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <fcntl.h>
#include "time.h"
#include <utime.h> 


#define BLKSIZE = 1024;

struct ar_hdr
{
	char ar_name[16];		/* Member file name, sometimes / terminated. */
	char ar_date[12];		/* File date, decimal seconds since Epoch.  */
	char ar_uid[6], ar_gid[6];	/* User and group IDs, in ASCII decimal.  */
	char ar_mode[8];		/* File mode, in ASCII octal.  */
	char ar_size[10];		/* File size, in ASCII decimal.  */
	char ar_fmag[2];		/* Always contains ARFMAG.  */
};

char *modes[] = {
	"---", "--x", "-w-", "-wx",
	"r--", "r-x", "rw-", "rwx"
};

void printHint()
{
	printf("/**********************************************************/\n");
	printf("Currently, this program only supoort following option keys\n");
	printf("  -q            - quickly append named file to archive\n");
	printf("  -x            - extract named files\n");
	printf("  -t            - print a concise table of contents of the archive\n");
	printf("  -v            - print a verbose table of contents of the archive\n");
	printf("  -d            - delete file(s) from the archive\n");
	printf("  -A 		- quickly append all 'regular' files in the current directory\n");
	printf("/**********************************************************/\n");
	printf("\n");
}

void print_SingleVerbos_Entry(struct ar_hdr ar_hdr_data)
{
	int i;
	for(i = 2; i >= 0 ; i--)
	{
		int j = (int)ar_hdr_data.ar_mode[5-i] - '0';
		printf("%s", modes[j]);
	}
	printf(" %s/%s", ar_hdr_data.ar_uid, ar_hdr_data.ar_gid);
	printf("      %s ", ar_hdr_data.ar_size);
	time_t rawtime =atoi(ar_hdr_data.ar_date) ;
	struct tm * timeinfo;
	timeinfo = localtime ( &rawtime );
	printf ( " %.24s", asctime (timeinfo) );
	printf(" %s", ar_hdr_data.ar_name);
	printf("\n");
}

void append_Files_to_Archive(int argc, char ** argv)
{
	int  n;
	int  from, to;
	char buf[1024];

	struct stat stat_Tmp;
	char header_Tmp[100];
	char ar_name_Tmp[16];
	
	if(argc == 2)
	{
		printf("Please enter the archive name at least\n");
	}
	
	//Open the target files(creat if it doesn't exist)
	if((to = open( argv[2], O_WRONLY|O_CREAT|O_APPEND, 0666)) <= 0)
	{
		printf("Error in target file parameter\n");
	}
	
	stat(argv[2], &stat_Tmp);
	if(((int)stat_Tmp.st_size) == 0)
	{
		write(to, "!<arch>\n", 8);	
	}

	int i=3;
	for(; i < argc; i++)
	{
		
		if(stat(argv[i], &stat_Tmp) == 0)
		{
			sprintf(ar_name_Tmp, "%s%s", argv[i], "/"); 
			sprintf(header_Tmp, "%-16s%-12d%-6d%-6d%-8s%-10d%-2s", ar_name_Tmp, (int)stat_Tmp.st_mtime, stat_Tmp.st_uid, stat_Tmp.st_gid, "100644", (int)stat_Tmp.st_size, "`\n");	
			write(to, header_Tmp, 60);
		}
		if((from = open(argv[i], O_RDONLY)) < 0)
		{
			printf("Error in appended files parameter\n");
		}
			
		while((n = read(from, buf, sizeof(buf))) >0 )
		{	
			write(to, buf, n);
		}
	}
	close(from);
	close(to);
}

void extract_Named_File(int argc, char **argv)
{
	int n, where;
	int from, to;
	char buf[1024];
	off_t file_size;
	char c[100];
	char header_Tmp[100];
	char ar_name_Tmp[16];
	int isMatch=0;
	
	if(argc == 2)
	{
		printf("Please enter the archive name at least\n");
	}
	if((from = open(argv[2], O_RDONLY|O_EXCL, 0666)) <= 0)
	{
		printf("No such file exists.\n");
		return;
	}
	where = lseek(from, 0, SEEK_END);
	lseek(from, 0, SEEK_SET);
	read(from, &c, 7);
	if(strcmp(c, "!<arch>") != 0)
	{
		printf("Bad file format");
		return;
	}
	if(read(from, &c, 1)<=0)
		return;
	if(argc == 3)
	{
	while(1)
	{
		struct ar_hdr ar_hdr_Tmp;
		if((n=read(from, &c, 59)) > 0)
		{
			char *pch;
			pch = strtok(c, " /");
			int i=0;
			while(pch != NULL)
			{
				switch(i)
				{
					case 0: sprintf(ar_hdr_Tmp.ar_name, "%s", pch);break;
					case 1: sprintf(ar_hdr_Tmp.ar_date, "%s", pch);break;
					case 2: sprintf(ar_hdr_Tmp.ar_uid, "%s", pch);break;
					case 3: sprintf(ar_hdr_Tmp.ar_gid, "%s", pch);break;
					case 4: sprintf(ar_hdr_Tmp.ar_mode, "%s", pch);break;
					case 5: sprintf(ar_hdr_Tmp.ar_size, "%s", pch);break;
					case 6: sprintf(ar_hdr_Tmp.ar_fmag, "%s", pch);break;
				}
				i++;
				pch = strtok(NULL, " ");
				
			}
			
		}
		if(read(from, &c, 1)<=0)
			return;
		if(ar_hdr_Tmp.ar_name)
		{
			//Open the target files(creat if it doesn't exist)
			if((to = open(ar_hdr_Tmp.ar_name, O_WRONLY|O_CREAT|O_APPEND|O_TRUNC, 0666)) <= 0)
			{
				printf("Error in creat files\n");
				return;
			}
			
			if((atoi(ar_hdr_Tmp.ar_size)) < sizeof(buf))
			{
				if((n = read(from, buf, (atoi(ar_hdr_Tmp.ar_size))))>0)
					write(to, buf, n); 		
			}
			else{
				while(( n = read (from, buf, sizeof(buf))) > 0)
				{
					write(to, buf, n);
				}
			}
			struct utimbuf buf;    
    			buf.actime=atoi(ar_hdr_Tmp.ar_date);  
    			buf.modtime=atoi(ar_hdr_Tmp.ar_date);  
  			utime(ar_hdr_Tmp.ar_name,&buf);
			close(to);
		}
	}
	}
	else
	{
	while(1)
	{
		struct ar_hdr ar_hdr_Tmp;
		if((n=read(from, &c, 59)) > 0)
		{
			char *pch;
			pch = strtok(c, " /");
			int i=0;
			while(pch != NULL)
			{
				switch(i)
				{
					case 0: sprintf(ar_hdr_Tmp.ar_name, "%s", pch);break;
					case 1: sprintf(ar_hdr_Tmp.ar_date, "%s", pch);break;
					case 2: sprintf(ar_hdr_Tmp.ar_uid, "%s", pch); break;
					case 3: sprintf(ar_hdr_Tmp.ar_gid, "%s", pch); break;
					case 4: sprintf(ar_hdr_Tmp.ar_mode, "%s", pch);break;
					case 5: sprintf(ar_hdr_Tmp.ar_size, "%s", pch);break;
					case 6: sprintf(ar_hdr_Tmp.ar_fmag, "%s", pch);break;
				}
				i++;
				pch = strtok(NULL, " ");
			}			
		}
		if(read(from, &c, 1)<=0)
			return;
		printf("%s\n", ar_hdr_Tmp.ar_name);
		int i=3;
		for(; i<argc; i++)
		{
			if(strcmp(ar_hdr_Tmp.ar_name, argv[i])==0)
			{
				isMatch = 1;
				//Open the target files(creat if it doesn't exist)
			if((to = open(ar_hdr_Tmp.ar_name, O_WRONLY|O_CREAT|O_APPEND|O_TRUNC, 0666)) <= 0)
			{
				printf("Error in creat files\n");
				return;
			}
		
				sprintf(ar_name_Tmp, "%s%s", ar_hdr_Tmp.ar_name, "/"); 
				sprintf(header_Tmp, "%-16s%-12s%-6s%-6s%-8s%-10s%-2s", ar_name_Tmp, ar_hdr_Tmp.ar_date, ar_hdr_Tmp.ar_uid, ar_hdr_Tmp.ar_gid, ar_hdr_Tmp.ar_mode, ar_hdr_Tmp.ar_size, ar_hdr_Tmp.ar_fmag);	
				//write(to, header_Tmp, 59);
				//write(to, "\n", 1);
				if((n = read(from, buf, (atoi(ar_hdr_Tmp.ar_size))))>0)
					write(to, buf, n);
				struct utimbuf buf;    
    				buf.actime=atoi(ar_hdr_Tmp.ar_date);  
    				buf.modtime=atoi(ar_hdr_Tmp.ar_date);  
  				utime(ar_hdr_Tmp.ar_name,&buf); 

				break;
				
			}
		}
		if(!isMatch)
			lseek(from, (atoi(ar_hdr_Tmp.ar_size)), SEEK_CUR);
		if((lseek(from, 0, SEEK_CUR) + 60)>where)
			return;
		

		}
	}	
}

void print_Concise_Table(int argc, char **argv)
{
	int n, where;
	int from, to;
	char buf[1024];
	char c[100];
	
	if(argc == 2)
	{
		printf("Please enter the archive name at least\n");
	}
	if((from = open(argv[2], O_RDONLY|O_EXCL, 0666)) <= 0)
	{
		printf("No such file exists.\n");
		return;
	}
	where = lseek(from, 0, SEEK_END);
	lseek(from, 0, SEEK_SET);
	read(from, &c, 7);
	if(strcmp(c, "!<arch>") != 0)
	{
		printf("Bad file format");
		return;
	}
	if(read(from, &c, 1)<=0)
		return;
	while(1)
	{
		struct ar_hdr ar_hdr_Tmp;
		if((n=read(from, &c, 59)) > 0)
		{
			char *pch;
			pch = strtok(c, " /");
			int i=0;
			while(pch != NULL)
			{
				switch(i)
				{
					case 0: sprintf(ar_hdr_Tmp.ar_name, "%s", pch);break;
					case 5: sprintf(ar_hdr_Tmp.ar_size, "%s", pch);break;
					case 6: sprintf(ar_hdr_Tmp.ar_fmag, "%s", pch);break;
				}
				i++;
				pch = strtok(NULL, " ");
			}
			if(strcmp(ar_hdr_Tmp.ar_fmag, "`\n"))
				printf("%s\n", ar_hdr_Tmp.ar_name);
			else				
				return;
			
		}
		if(read(from, &c, 1)<=0)
			return;
		if((lseek(from, (atoi(ar_hdr_Tmp.ar_size)), SEEK_CUR) + 60)>where)
			return;
		
	}
}

void print_Verbose_Table(int argc, char **argv)
{
		int n, where;
	int from, to;
	char buf[1024];
	char c[100];
	
	if(argc == 2)
	{
		printf("Please enter the archive name at least\n");
	}
	if((from = open(argv[2], O_RDONLY|O_EXCL, 0666)) <= 0)
	{
		printf("No such file exists.\n");
		return;
	}
	where = lseek(from, 0, SEEK_END);
	lseek(from, 0, SEEK_SET);
	read(from, &c, 7);
	if(strcmp(c, "!<arch>") != 0)
	{
		printf("Bad file format");
		return;
	}
	if(read(from, &c, 1)<=0)
		return;
	while(1)
	{
		struct ar_hdr ar_hdr_Tmp;
		if((n=read(from, &c, 59)) > 0)
		{
			char *pch;
			pch = strtok(c, " /");
			int i=0;
			while(pch != NULL)
			{
				switch(i)
				{
					case 0: sprintf(ar_hdr_Tmp.ar_name, "%s", pch);break;
					case 1: sprintf(ar_hdr_Tmp.ar_date, "%s", pch);break;
					case 2: sprintf(ar_hdr_Tmp.ar_uid, "%s", pch); break;
					case 3: sprintf(ar_hdr_Tmp.ar_gid, "%s", pch); break;
					case 4: sprintf(ar_hdr_Tmp.ar_mode, "%s", pch);break;
					case 5: sprintf(ar_hdr_Tmp.ar_size, "%s", pch);break;
					case 6: sprintf(ar_hdr_Tmp.ar_fmag, "%s", pch);break;
				}
				i++;
				pch = strtok(NULL, " ");
			}
			if(strcmp(ar_hdr_Tmp.ar_fmag, "`\n")){
				print_SingleVerbos_Entry(ar_hdr_Tmp);
			}
			else				
				return;
			
		}
		if(read(from, &c, 1)<=0)
			return;
		if((lseek(from, (atoi(ar_hdr_Tmp.ar_size)), SEEK_CUR) + 60)>where)
			return;
		
	}
}

void delete_Named_Files(int argc, char **argv)
{
	int n;
	int from, to;
	char buf[1024];
	char c[100];
	int where;
	char header_Tmp[100];
	char ar_name_Tmp[16];
	int isDelete = 0;

	if(argc == 2)
	{
		printf("Please enter the archive name at least\n");
	}
	if((from = open(argv[2], O_RDONLY|O_EXCL, 0666)) <= 0)
	{
		printf("No such archive file exists.\n");
		return;
	}
	where = lseek(from, 0, SEEK_END);
	lseek(from, 0, SEEK_SET);
	read(from, &c, 7);
	if(strcmp(c, "!<arch>") != 0)
	{
		printf("Bad file format");
		return;
	}
	if(read(from, &c, 1)<=0)
		return;

	unlink(argv[2]);
	//creat a fresh file
	if((to = open( argv[2], O_WRONLY|O_CREAT|O_TRUNC |O_APPEND, 0666)) <=0)
	{
		printf("Error in creating files");
	}


	if(argc == 3)
		return;
	write(to, "!<arch>\n", 8);

	while(1)
	{
		struct ar_hdr ar_hdr_Tmp;
		if((n=read(from, &c, 59)) > 0)
		{
			isDelete = 0;
			char *pch;
			pch = strtok(c, " /");
			int i=0;
			while(pch != NULL)
			{
				switch(i)
				{
					case 0: sprintf(ar_hdr_Tmp.ar_name, "%s", pch);break;
					case 1: sprintf(ar_hdr_Tmp.ar_date, "%s", pch);break;
					case 2: sprintf(ar_hdr_Tmp.ar_uid, "%s", pch); break;
					case 3: sprintf(ar_hdr_Tmp.ar_gid, "%s", pch); break;
					case 4: sprintf(ar_hdr_Tmp.ar_mode, "%s", pch);break;
					case 5: sprintf(ar_hdr_Tmp.ar_size, "%s", pch);break;
					case 6: sprintf(ar_hdr_Tmp.ar_fmag, "%s", pch);break;
				}
				i++;
				pch = strtok(NULL, " ");
			}
		}

		int i=3;
		for(; i<argc; i++)
		{
			if(strcmp(ar_hdr_Tmp.ar_name, argv[i])==0)
			{
				isDelete = 1;
				lseek(from, (atoi(ar_hdr_Tmp.ar_size))+1, SEEK_CUR);
				break;
			}
		}
		if(!isDelete){
				sprintf(ar_name_Tmp, "%s%s", ar_hdr_Tmp.ar_name, "/"); 
				sprintf(header_Tmp, "%-16s%-12s%-6s%-6s%-8s%-10s%-2s", ar_name_Tmp, ar_hdr_Tmp.ar_date, ar_hdr_Tmp.ar_uid, ar_hdr_Tmp.ar_gid, ar_hdr_Tmp.ar_mode, ar_hdr_Tmp.ar_size, ar_hdr_Tmp.ar_fmag);	
				write(to, header_Tmp, 59);
				if((n = read(from, buf, (atoi(ar_hdr_Tmp.ar_size)) + 1))>0)
					write(to, buf, n);				
		}
		if((lseek(from, 0, SEEK_CUR) + 59) > where)
		{
				return;
		}
		
	}

	close(from);
	close(to);
}

void append_All_Files_In_Current_Dir(int argc, char ** argv)
{
	int  n;
	int  from, to;
	char buf[1024];

	struct stat stat_Tmp;
	char header_Tmp[100];
	char ar_name_Tmp[16];
	DIR *dp;
	struct direct *dir;
	
	if(argc == 2)
	{
		printf("Please enter the archive name at least\n");
	}
	
	//Open the target files(creat if it doesn't exist)
	if((to = open( argv[2], O_WRONLY|O_CREAT|O_APPEND, 0666)) <= 0)
	{
		printf("Error in target file parameter\n");
	}
	
	stat(argv[2], &stat_Tmp);
	if(((int)stat_Tmp.st_size) == 0)
	{
		write(to, "!<arch>\n", 8);	
	}

	if((dp = opendir("."))== NULL)
	{
		printf("Error! No files in current directory");
	}
	while((dir = readdir(dp)) != NULL)
	{
		if(dir->d_ino == 0)
			continue;
		
		if(stat(dir->d_name, &stat_Tmp) == 0)
		{
			if(S_ISREG(stat_Tmp.st_mode) && strcmp(dir->d_name, argv[2]))
			{
				sprintf(ar_name_Tmp, "%s%s", dir->d_name, "/");
				sprintf(header_Tmp, "%-16s%-12d%-6d%-6d%-8s%-10d%-2s", ar_name_Tmp, (int)stat_Tmp.st_mtime, stat_Tmp.st_uid, stat_Tmp.st_gid, "100644", (int)stat_Tmp.st_size, "`\n");	
				write(to, header_Tmp, 60);
				//printf("%20s\n", dir->d_name);			
	
				if((from = open(dir->d_name, O_RDONLY)) < 0)
				{
					printf("Error in appended files parameter\n");
				}
				int remain_size = (int)stat_Tmp.st_size;
				
				while(remain_size > sizeof(buf))
				{
					if(( n = read (from, buf, sizeof(buf))) > 0)
						write(to, buf, n);
					remain_size -= sizeof(buf);
				}

				if( remain_size < sizeof(buf))
				{
					if((n = read(from, buf, remain_size))>0)
						write(to, buf, n);
				}
			}
		}
	}
	close(from);
	close(to);
	close(dp);

}

int main(argc, argv)
int argc;
char **argv;
{
	if(argc > 1 && argv[1][0] == '-')
	{
		if (strcmp(argv[1], "-q") == 0)
		{
			append_Files_to_Archive(argc, argv);
		}
		else if (strcmp(argv[1], "-x") == 0)
		{
				extract_Named_File(argc, argv);	
		}
		else if (strcmp(argv[1], "-t") == 0)
		{
			print_Concise_Table(argc, argv);
		}
		else if (strcmp(argv[1], "-v") == 0)
		{
			print_Verbose_Table(argc, argv);
		}
		else if (strcmp(argv[1], "-d") == 0)
		{
			delete_Named_Files(argc, argv);
		}
		else if (strcmp(argv[1], "-A") == 0)
		{
			append_All_Files_In_Current_Dir(argc, argv);
		}
		else
		{
			printHint();
		}
	}
	else
	{
		printHint();
	}
	return 1;
}
