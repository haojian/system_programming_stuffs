/**************Statement of Policy****************************/
// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
// A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Haojian Jin
/*************************************************************/

#include <stdio.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int uniqify(char *_inputPath, int _sort_Process_Number)
{
	FILE *finput, *foutput;
	int pipe_parser2sort[_sort_Process_Number][2];		//pipes from parser to sorting processes
	int pipe_sort2merger[_sort_Process_Number][2];		//pipes from sorting processes to merger
	int pipe_merger2output[2];				//merger's output.
	char buffer_Input[BUFSIZ], buffer_sort2merger[_sort_Process_Number][BUFSIZ], buffer_Output[BUFSIZ];  //temporary buffer.
	pid_t pid, pid2;         
	int cur_Word_Index = 0;					//total word counter.

	int wordsCountInProcess[_sort_Process_Number];

	FILE *fparser2sort[_sort_Process_Number];
	FILE *fsort2merger[_sort_Process_Number];
	int i;
	
	if((finput = fopen(_inputPath, "r")) == NULL)
	{
		printf("Can't read the text file successfully.\n");
		return -1;
	}
	

	//From paser to sorting processes.
	for(i=0; i < _sort_Process_Number; i++){
		if(pipe(pipe_parser2sort[i]) < 0 || pipe(pipe_sort2merger[i]) <0)
			printf("pipe error");
		fparser2sort[i] = fdopen(pipe_parser2sort[i][1], "w");
	}
	while(fgets(buffer_Input, BUFSIZ, finput) != NULL)
	{
		int n = strlen (buffer_Input);
		for(i = 0 ; i < n; i++)
		{
			if((*(buffer_Input+i)>='A'&&*(buffer_Input+i)<='Z')||(*(buffer_Input+i)>='a')&&(*(buffer_Input+i)<='z'))
			{
				if((*(buffer_Input+i)>='A'&&*(buffer_Input+i)<='Z'))
				{
					*(buffer_Input+i) += 32;
				}
			}
			else
			{
				*(buffer_Input+i)=' ';
			}
		}
		char *pch;
		pch = strtok(buffer_Input, " ");
		while(pch != NULL)
		{
			cur_Word_Index++;
			int assigned_Process_ID = (cur_Word_Index-1)%_sort_Process_Number;
			wordsCountInProcess[assigned_Process_ID]++;
			int tmpLength = strlen(pch);
			char tmpPch[tmpLength + 1];
			sprintf(tmpPch, "%s%s", pch, "\n"); 
			fputs(tmpPch, fparser2sort[assigned_Process_ID]);
			pch = strtok(NULL, " ");
		}
	}
	for(i = 0; i< _sort_Process_Number; i++ )
	{
		pid = fork();
		if(pid < 0)
			printf("Fork failed\n");
		else if( pid > 0 )
		{
			close(pipe_parser2sort[i][0]);
			close(pipe_sort2merger[i][1]);

			fclose(fparser2sort[i]);
			fsort2merger[i] =fdopen(pipe_sort2merger[i][0],"r"); 
			
			close(pipe_parser2sort[i][1]);
			waitpid(pid, NULL, 0);
		}
		else
		{
			dup2(pipe_parser2sort[i][0], STDIN_FILENO);
			close(pipe_parser2sort[i][0]);
			close(pipe_parser2sort[i][1]);
			dup2(pipe_sort2merger[i][1], STDOUT_FILENO);
			close(pipe_sort2merger[i][0]);
			close(pipe_sort2merger[i][1]);
			execlp("sort", "sort",(char *)NULL);
		}
	}


	//From sorting processes to merger.
	pid2 = fork();
	if(pipe(pipe_merger2output) < 0)
		printf("Pipe Error\n");

	if(pid2 < 0)
		printf("Fork failed\n");
	else if( pid2 > 0 )
	{
		waitpid(pid2, NULL, 0);
	}
	else
	{
		// suppress duplicate words and write data to pipe
		for(i=0; i < _sort_Process_Number; i ++)
		{
			if(fgets(buffer_sort2merger[i], BUFSIZ, fsort2merger[i]) ==NULL)
			{
				printf("Number of sorting processes should be bigger than words number.\n");
				return;
			}
		}

		foutput = fdopen(pipe_merger2output[1], "w");
		
		// close read end;
		close(pipe_merger2output[0]);

		int is_all_word_processed[_sort_Process_Number];
		memset(is_all_word_processed, 0, sizeof(int)*_sort_Process_Number);
		char tmp_first_word[BUFSIZ];
		int tmp_first_word_index = -1;
		char last_first_word[BUFSIZ];
		last_first_word[0]=' ';
		while(cur_Word_Index > 0)
		{
			tmp_first_word[0] = '{';
			tmp_first_word_index = -1;
			for(i=0; i<_sort_Process_Number; i++){
				if(strcmp(tmp_first_word, buffer_sort2merger[i]) > 0)
				{
					strcpy (tmp_first_word,buffer_sort2merger[i]);
					//tmp_first_word = buffer_sort2merger[i];
					tmp_first_word_index = i;
				}
			}
			if(tmp_first_word[0] != '{'  && strcmp(last_first_word, tmp_first_word) != 0)
			{
				write(STDOUT_FILENO, tmp_first_word, strlen(tmp_first_word));
				strcpy (last_first_word,tmp_first_word);
			}	
			
			cur_Word_Index--;		
			//fputs(tmp_first_word, foutput);
			if(fgets(buffer_sort2merger[tmp_first_word_index], BUFSIZ, fsort2merger[tmp_first_word_index]) == NULL){
				buffer_sort2merger[tmp_first_word_index][0] = '{';
			}
		}	
		//close write end & file stream.
		fclose(fsort2merger[i]);
		close(pipe_sort2merger[i][0]);
		fclose(foutput);
		close(pipe_merger2output[1]);
		return 1;
	}
	return 1;
}

int main(argc, argv)
int argc;
char **argv;
{
	if(argc != 3)
	{
		printf("usage: ./uniqify <inputfile> n \n  n is the number of sorting processes\n");
	}
	else
	{
		uniqify(argv[1], atoi(argv[2]));
		//uniqify("datafile", "", 5);	
		printf("Application End\n");
	}
	return 0;
}
