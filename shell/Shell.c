#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "command.h"
#include <sys/wait.h>

#define STDIN  0
#define STDOUT 1
#define STDERR 2 
#define MAX_ARGS 50
#define MAX_INPUT 1024
#define INITIAL_ARG_SIZE 10

/***********************************************************/
/*                 REDIRCTION VARIABLES                    */
/***********************************************************/
extern int output_flag ;
extern int input_flag  ;
extern int error_flag  ;
extern int check_flag ;
extern int original_stdout ;
extern int original_stdin ;
extern int original_stderror ;
/************************************************************/


/************************************************************/
char *token ;
int append_flag = 0 ;
int overwrite_flag = 0;
/***********************************************************/

int main()
{
	char cmd[100];
	char pipe_cmd[100];
	int opt ;
	char command[100];
	ssize_t readsize = 0 ;
	int i =0 ;
	const char *shellmsg = "Enter the desired command:";
	while(1)
	{
		int pipe_flag = 0 ;
		token = NULL ;
		i =0 ; 
	       // Initialize command with zeros
                memset(command, 0, sizeof(command));
		memset(cmd, 0, sizeof(cmd));
		write(STDOUT,shellmsg,strlen(shellmsg));
		readsize = read(STDIN,command,100);
                
                command[readsize-1] = '\0'; // Ensure the buffer is null-termination
		//put the command in another token 
		strcpy(cmd, command);
                strcpy(pipe_cmd,command);  
		// Tokenize the input into arguments
                 char **args = malloc(INITIAL_ARG_SIZE * sizeof(char *));
		 int arg_count = 0 ;
                 int arg_size = INITIAL_ARG_SIZE;               
                 token = strtok(cmd, " ");
                 while (token!=NULL) 
		 {
                            if (arg_count >= arg_size - 1)
			    {
                               arg_size *= 2;  // Double the size
                               args = realloc(args, arg_size * sizeof(char *));  // Reallocate memory
                            }
                            args[arg_count++] = token;
                            token = strtok(NULL, " ");
                 }
                 args[arg_count] = '\0';  // Null-terminate the argument lisit
	
		/*************************************************************/
                /*               Piping Between Two Processes                */
                /*************************************************************/
		for(int i  = 0 ; i < arg_count ; i++)
                {
                        if(strcmp(args[i],"|")==0)
                        {
				pipe_flag = 1 ;
				int pipefd ;
				char **args_of_cmd1 = malloc(INITIAL_ARG_SIZE * sizeof(char *));
				char **args_of_cmd2 = malloc(INITIAL_ARG_SIZE * sizeof(char *));
		      		int arg_count_of_cmd1 = 0 , arg_count_of_cmd2 = 0;

                                // Parse the command into two parts separated by '|'
                                char *cmd1 = strtok(pipe_cmd, "|");
                                char *cmd2 = strtok(NULL, "|");		
	                        /********************process #1***********************/
				cmd1 = strtok(cmd1, " ");
				char *token_of_cmd1 = cmd1 ;
                                while (cmd1)
                                {
					// Tokenize command 1 into arguments
                                        if (arg_count_of_cmd1 >= arg_size - 1)
                                        {
                                                arg_size *= 2;  // Double the size
                                                args_of_cmd1 = realloc(args_of_cmd1, arg_size * sizeof(char *));  // Reallocate memory
                                        }
                                        args_of_cmd1[arg_count_of_cmd1++] = cmd1;
                                        cmd1 = strtok(NULL, " ");

                                }
                                args_of_cmd1[arg_count_of_cmd1] = NULL;  // Null-terminate the argument list
				
                                /**********************process #2*******************************/
                                cmd2 = strtok(cmd2, " ");
				char *token_of_cmd2 = cmd2 ; 
                                while (cmd2)
                                {
					// Tokenize command 2 into arguments
                                        if (arg_count_of_cmd2 >= arg_size - 1)
                                        {
                                                arg_size *= 2;  // Double the size
                                                args_of_cmd2 = realloc(args_of_cmd2, arg_size * sizeof(char *));  // Reallocate memory
                                        }
                                        args_of_cmd2[arg_count_of_cmd2++] = cmd2;
                                        cmd2 = strtok(NULL, " ");

                                }
   				args_of_cmd2[arg_count_of_cmd2] = NULL;  // Null-terminate the argument list

				/********************************************************************/
			     	/*                    Execute the first command                    */
				/*******************************************************************/
			
				pipefd = open("pipe", O_WRONLY |  O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR); //Open the shared file, which is being used as a pipe.
				int original_stdout = dup(STDOUT);  // Save the original stdout
                                redirect(pipefd, STDOUT);
				check_command(args_of_cmd1,arg_count_of_cmd1,token_of_cmd1);
				redirect(original_stdout,STDOUT); //restore the original output file
			        close(pipefd); //close pipe file
			       	
                                /********************************************************************/
                                /*                    Execute the second command                    */
                                /*******************************************************************/
                                 pipefd = open("pipe", O_RDONLY);
                                 int original_stdin = dup(STDIN);  // Save the original stdout
				 redirect(pipefd,STDIN);
                                 check_command(args_of_cmd2,arg_count_of_cmd2,token_of_cmd2);
				 redirect(original_stdin,STDIN);   //restore the original input file
				 close(pipefd); //close pipe file
				 unlink("pipe"); //remove pipe file
                        }	
		}
		if(pipe_flag == 0)
		{
			check_command(args,arg_count,command);
		}
                                                   
	}	
	return 0 ;
}

void check_command(char **args,int arg_count,char *token)
{
	/****************************************************************/
	/*                    CHECK REDIRECTION                         */
	/* First, we check if redirection is detected                   */
	/* Then, we recalculate the number of arguments                 */
	/****************************************************************/
	args = check_redirection(args,arg_count);
	int k = 0 ;
	arg_count = 0 ;
	while(args[k]!= NULL)
	{
		arg_count++ ;
		k++ ;
	}
	if(arg_count > 1)
	{
		token = strtok(token," ");
	}
	else
	{
		/*******remove space******/
		 int i = 0, j = 0;
	      	 while (token[i]) {
		 	 if (token[i] != ' ') {
		     		 token[j++] = token[i];
		 	 }
		 	 i++;
	     	 }
	     	 token[j] = '\0';  // Null-terminate the resulting string
	}
	if(check_flag == 0 )
	{
		if(strcmp(token,"mypwd")==0)
		{
			print_working_directory();
		}
		else if(strcmp(token,"myecho")==0)
		{
			print_user_input(args,arg_count);
		}
		else if(strcmp(token,"exit")==0)
		{
			printf("Good Bye\n");
			exit(0) ;
		}
		else if(strcmp(token,"mycp")==0)
		{
			append_flag = 0; // Reset flag before processing options
			if(arg_count==1)
			{
				fflush(stdout);
				fprintf(stderr,"Enter the source and destination files\n");
			}
			else
			{
				if (args[1][0]=='-')
				{
					if(strcmp(args[1],"-a")==0)
					{
						append_flag = 1 ;
						token = strtok(NULL," ");
						copy_files(args);
					}
					else
					{
						fprintf(stderr, "Usage: %s [-a] <source> <dest>\n", args[0]);
					}
				}
				else
				{
					copy_files(args);
				}
			}
	        }
		else if(strcmp(token,"")==0)
		{
		}
		else if(strcmp(token,"mymv")==0)
		{
			overwrite_flag = 0; // Reset flag before processing options
			if(arg_count==1)
			{
				fflush(stdout);
				fprintf(stderr,"Enter the source and destination files\n");
			}
			else
			{
				if (args[1][0]=='-')
				{
					if(strcmp(args[1],"-f")==0)
					{
						overwrite_flag = 1 ;
						token = strtok(NULL," ");
						move_files(args);
					}
					else					
					{
						fflush(stdout);
						fprintf(stderr, "Usage: %s [-f] <source> <dest>\n", args[0]);
					}
				}
				else
				{
					move_files(args);
				}
			}
         	   }
		else if (strcmp(token,"cd")==0)
		{
			change_directory(args);
		}
		else if(strcmp(token,"myenv")==0)
		{
			print_envir_variable();
		}
		else if(strcmp(token,"type")==0)
		{
			print_command_type(args);
		}
		else if(strcmp(token,"myfree")==0)
		{
			print_RAM_info();
		}
		else if(strcmp(token,"myuptime")==0)
		{
			print_uptime();
		}
		else if(strcmp(token,"help")==0)
		{
			print_help();
		}
		else
		{	
			external_command(args,arg_count);	
		}
	}
	free(args);
	/***************************************************/
	/*         RESTORE THE ORIGINAL FILES              */
	/***************************************************/
	if(output_flag == 1)
	{
		redirect(original_stdout,STDOUT);
	}
	if(input_flag == 1)
	{
		redirect(original_stdin,STDIN);
	}
	if(error_flag == 1)
	{
		redirect(original_stderror,STDERR);
	}
}
