#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
/**
  1. dynamic allocate tokens number
  2. backgroud process check for pipe
  3. multi pipe
 */

void sig_fork(int signo)
{
    waitpid(0,NULL,WNOHANG);
    return;
}

int main()
{

	//int fd[2];
	pid_t pid;
	char string[256]={0},string2[256],string3[256],token_tmp[256];
	int pids[256];
	int pindex=0;
	int index;
	char* token;
	char* token2;
	char* token_ptrsav;
	char* token2_ptrsav;
	char* tokens[256];
	int backgroud_process_flag, pipe_fileIn_flag, pipe_fileOut_flag;
	int pipe_num;
	char* pipe_flag;
	char* find_pipe;
	int file;   /* for output redirection*/
	char fname_in[256];   /*    input this file  */
	char fname_out[256];   /*   to this file  */
	signal (SIGCHLD, sig_fork); 
	printf("$ ");
	while(1){
		index = 0;
		backgroud_process_flag = 0;
		pipe_fileIn_flag = 0;
		pipe_fileOut_flag = 0;
		pipe_num = 0;
		fgets(string,256,stdin);
		strcpy(string2, string);
		strcpy(string3, string);
		//pipe_flag = strchr(string2, '|');
		find_pipe = string;
		while(*find_pipe!='\0')
		{	
			if(*find_pipe == '|')pipe_num++;
			find_pipe++;
		}	


		if(pipe_num!=0)
		{
			token = strtok_r(string2, "|\n\r",&token_ptrsav);  /**out**/
			int pipefds[2*pipe_num];
			int i;
			int command_count=0;
			int pipe_th=0;
			/** open pipe*/
			for(i=0;i<pipe_num;i++)
			{
				if (pipe(pipefds+i*2) == -1){
					perror("pipe");
					exit(EXIT_FAILURE);
				}
			}
			while(token!=NULL)
			{
				command_count++;
				index=0;  //***init 0 
				//printf("%s\n",token);
				strcpy(token_tmp, token);
				token2 = strtok_r(token_tmp, " \n\r\0",&token2_ptrsav);
				while(token2!=NULL)
				{
					if(command_count != pipe_num+1)  /* not the last command*/
					{
						tokens[index++]=token2;
						token2 = strtok_r(NULL, " \n\r\0",&token2_ptrsav);
					}else{
						tokens[index++]=token2;

						/** check "&"*/
						char* and_ptr;
						and_ptr = strchr(tokens[index-1], '&');	
				//		if(!backgroud_process_flag)
							if(and_ptr == NULL)
							{
								backgroud_process_flag = 0;
							}else
							{
								backgroud_process_flag = 1;
								/* delete the '&' */
								if(! strcmp(tokens[index-1], "&"))
								{
									tokens[index-1] = NULL;
									index--;
								}
								else *and_ptr = '\0';

							}
						/* end of check &*/
						/* check '<' */
						char* pipeFileIn;
						pipeFileIn = strchr(tokens[index-1], '<');	
						if(!pipe_fileIn_flag)
							if(pipeFileIn == NULL)
							{
								pipe_fileIn_flag = 0;
							}else
							{
								pipe_fileIn_flag = 1;
								//printf("token: %s, *%d-%d\n",token,*(pipeFileIn-1) ,*(pipeFileIn+1) );
								/* delete the '<' and target */
								if(strcmp(tokens[index-1],"<") && *(pipeFileIn+1) == 0)
								{
									*pipeFileIn ='\0';
									strcpy(fname_in, strtok(NULL, " \n\r\0"));
								}else if(*(pipeFileIn-1) != 0 && *(pipeFileIn+1) != 0){
									*pipeFileIn = 0;
									pipeFileIn++;
									int i=0;
									while(*pipeFileIn != 0){
										fname_in[i++] = *pipeFileIn;
										*pipeFileIn = 0;
										pipeFileIn++;
									}
									fname_in[i] = 0;
								}else if(!strcmp(tokens[index-1], "<")){
									tokens[index-1] = NULL;
									index--;
									strcpy(fname_in, strtok_r(NULL, " \n\r\0",&token2_ptrsav) );
								}else{
									strcpy(fname_in, (tokens[index-1])+1);	
									tokens[index-1] = NULL;
									index--;
									strtok_r(NULL, " \n\r\0",&token2_ptrsav);
								}

							}
						/* check end '<'*/

						/* check '>' */
						char* pipeFileOut;	
						pipeFileOut = strchr(tokens[index-1], '>');	
						if(!pipe_fileOut_flag)
							if(pipeFileOut == NULL)
							{
								pipe_fileOut_flag = 0;
							}else
							{
								pipe_fileOut_flag = 1;
								//printf("token: %s, *%d-%d\n",token,*(pipeFileIn-1) ,*(pipeFileIn+1) );
								/* delete the '>' and target */
								if(strcmp(tokens[index-1],">") && *(pipeFileOut+1) == 0)
								{
									*pipeFileOut ='\0';
									strcpy(fname_out, strtok_r(NULL, " \n\r\0",&token2_ptrsav) );
								}else if(*(pipeFileOut-1) != 0 && *(pipeFileOut+1) != 0){
									*pipeFileOut = 0;
									pipeFileOut++;
									int i=0;
									while(*pipeFileOut != 0){
										fname_out[i++] = *pipeFileOut;
										*pipeFileOut = 0;
										pipeFileOut++;
									}
									fname_out[i] = 0;
								}else if(!strcmp(tokens[index-1], ">")){
									tokens[index-1] = NULL;
									index--;  //**
									strcpy(fname_out, strtok_r(NULL, " \n\r\0",&token2_ptrsav) );
								}else{
									strcpy(fname_out, (tokens[index-1])+1);	
									tokens[index-1] = NULL;
									index--;
									strtok_r(NULL, " \n\r\0",&token2_ptrsav) ;
								}

							}
						/* check end '>' */
						token2 = strtok_r(NULL, " \n\r\0",&token2_ptrsav);
					}
				}
				tokens[index] = NULL;
				pid = fork();
				pids[pindex++] = pid;
				if (pid < 0) { /* error occurred */
					fprintf(stderr, "Fork Failed");
					exit(-1);
				}
				/* child process */
				else if (pid == 0) {
					//printf("%d th \n",pipe_th);	
					if(pipe_th != 0)  /**not first command*/
					{
						if(dup2(pipefds[(pipe_th-1)*2],0)<0)
						{
							perror("not first: dup2");
							exit(errno);
						}
					}else{	/* first command */
						if(pipe_fileIn_flag){
							file = open(fname_in, O_RDWR | O_CREAT , S_IRUSR | S_IWUSR);
							dup2(file,0);
						}
							
					}
					if(pipe_th != (pipe_num)) /** not last command*/
					{
						if(dup2(pipefds[pipe_th*2+1],1)<0)
						{
							perror("not last: dup2");
							exit(errno);
						}
					}else{	/* last command */
						if(pipe_fileOut_flag){
							file = open(fname_out, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
							dup2(file,1);
						}
					}	
					/** close all pipe*/
					for(i=0;i<pipe_num*2;i++)close(pipefds[i]);
					if(execvp(tokens[0], tokens) == -1)
					{
						perror("error: ");
						exit(errno);
					}
				}
				else { /* parent process */
					/* parent will wait for the child to complete */
				}
				pipe_th++;
				token = strtok_r(NULL, "|\n\r",&token_ptrsav);  /**out**/
			} /** out while*/
			for(i=0;i<pipe_num*2;i++)close(pipefds[i]);
			if(backgroud_process_flag)
				for(i=0;i<pindex;i++)waitpid(-1,NULL,WNOHANG);
			else
				for(i=0;i<pindex;i++)waitpid(pids[i],NULL,0);
			pindex = 0;
			printf("$ ");
		}else /* NO PIPE*/ 
		{
			
			token = strtok(string, " \n\r\0");
			if(token == NULL)
			{
				printf("$ ");
				continue;
			}

			while(token!=NULL)
			{
				//printf("%s\n",token);
				tokens[index++]=token;
				/** check "&"*/
				char* and_ptr;
				and_ptr = strchr(tokens[index-1], '&');	
				//if(!backgroud_process_flag)
					if(and_ptr == NULL)
					{
						backgroud_process_flag = 0;
					}else
					{
						backgroud_process_flag = 1;
						/* delete the '&' */
						if(! strcmp(tokens[index-1], "&"))
						{
							tokens[index-1] = NULL;
							index--;
						}
						else *and_ptr = '\0';

					}
				/* end of check &*/
				/* check '<' */
				char* pipeFileIn;
				pipeFileIn = strchr(tokens[index-1], '<');	
				if(!pipe_fileIn_flag)
					if(pipeFileIn == NULL)
					{
						pipe_fileIn_flag = 0;
					}else
					{
						pipe_fileIn_flag = 1;
						//printf("token: %s, *%d-%d\n",token,*(pipeFileIn-1) ,*(pipeFileIn+1) );
						/* delete the '<' and target */
						if(strcmp(tokens[index-1],"<") && *(pipeFileIn+1) == 0)
						{
							*pipeFileIn ='\0';
							strcpy(fname_in, strtok(NULL, " \n\r\0"));
						}else if(*(pipeFileIn-1) != 0 && *(pipeFileIn+1) != 0){
							*pipeFileIn = 0;
							pipeFileIn++;
							int i=0;
							while(*pipeFileIn != 0){
								fname_in[i++] = *pipeFileIn;
								*pipeFileIn = 0;
								pipeFileIn++;
							}
							fname_in[i] = 0;
						}else if(!strcmp(tokens[index-1], "<")){
							tokens[index-1] = NULL;
							index--;
							strcpy(fname_in, strtok(NULL, " \n\r\0"));
						}else{
							strcpy(fname_in, (tokens[index-1])+1);	
							tokens[index-1] = NULL;
							index--;
							strtok(NULL, " \n\r\0");
						}

					}
				/* check end '<'*/

				/* check '>' */
				char* pipeFileOut;	
				pipeFileOut = strchr(tokens[index-1], '>');	
				if(!pipe_fileOut_flag)
					if(pipeFileOut == NULL)
					{
						pipe_fileOut_flag = 0;
					}else
					{
						pipe_fileOut_flag = 1;
						//printf("token: %s, *%d-%d\n",token,*(pipeFileIn-1) ,*(pipeFileIn+1) );
						/* delete the '>' and target */
						if(strcmp(tokens[index-1],">") && *(pipeFileOut+1) == 0)
						{
							*pipeFileOut ='\0';
							strcpy(fname_out, strtok(NULL, " \n\r\0"));
						}else if(*(pipeFileOut-1) != 0 && *(pipeFileOut+1) != 0){
							*pipeFileOut = 0;
							pipeFileOut++;
							int i=0;
							while(*pipeFileOut != 0){
								fname_out[i++] = *pipeFileOut;
								*pipeFileOut = 0;
								pipeFileOut++;
							}
							fname_out[i] = 0;
						}else if(!strcmp(tokens[index-1], ">")){
							tokens[index-1] = NULL;
							index--;  //**
							strcpy(fname_out, strtok(NULL, " \n\r\0"));
						}else{
							strcpy(fname_out, (tokens[index-1])+1);	
							tokens[index-1] = NULL;
							index--;
							strtok(NULL, " \n\r\0");
						}

					}
				/* check end '>' */
				token = strtok(NULL, " \n\r\0");
			}
			/** set last element NULL*/
			tokens[index] = NULL;
			/* fork another process */
			pid = fork();
			// printf("%d\n",pid);
			if (pid < 0) { /* error occurred */
				fprintf(stderr, "Fork Failed");
				exit(-1);
			}
			/* child process */
			else if (pid == 0) {
				/* check '<' or '>' */
				if(pipe_fileIn_flag){
					file = open(fname_in, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
					dup2(file,0);
				}			
				if(pipe_fileOut_flag){
					file = open(fname_out, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
					dup2(file,1);
					close(file);
				}			

				/* exec command */
				if(execvp(tokens[0], tokens) == -1)
				{
					perror("error: ");
					exit(errno);
				}
			}
			else { /* parent process */
				/* parent will wait for the child to complete */
				// waitpid(pid,NULL,WNOHANG);
				if(backgroud_process_flag)
				{
					waitpid(pid,NULL,WNOHANG);
					printf("$ ");
				}else
				{
					//wait(NULL);
					waitpid(pid,NULL,0);
					printf("$ ");
				}
			}

		}


	}
}
