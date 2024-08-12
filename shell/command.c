#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <time.h>
#include <utmp.h>

#define STDIN  0
#define STDOUT 1
#define STDERR 2

/***********************************************************/
/*                 REDIRCTION VARIABLES                    */
/***********************************************************/
int output_flag =  0 ;
int input_flag  =  0 ;
int error_flag  =  0 ;
int check_flag = 0 ;
int original_stdout ;
int original_stdin ;
int original_stderror ;
/************************************************************/

extern char *token ;
extern int append_flag ;
extern int overwrite_flag;
extern char **args ;
extern int arg_count ;
void redirect(int fd_old , int fd_new);

/************************************************************/
void print_working_directory()
{
	char  buffer[100];
	char *directory ;
	directory = getcwd(buffer,100);
	if(directory != NULL)
	{
		printf("%s\n",directory);
	}
	else
	{
		perror("Error");
	
	}
}
void print_user_input(char **args,int arg_count)
{
	int i , j ;
	for(j = 1 ; j < arg_count ; j++)
	{
		if((args[j][0] == '"') || (args[j][0] == '\''))
		{
			size_t len = strlen(args[j]);
			char *dup = strdup(args[j]);
			for(i = 1 ; i<len ;i++)
			{
				if((args[j][i]=='"') || (args[j][i] == '\''))
				{
					args[j][i] = '\0' ;
					//break ;
				}
			}
			if( i==(len-1) )
			{
				args[j] = (args[j]+1);
			}
			else
			{
				dup[len-1] = '\0' ;
				args[j] = strcat(args[j]+1,dup+(i+2));
			}
		}
	}
      	 if(arg_count > 1 )
	 {
		 for(int j = 1 ; j < arg_count ; j++)
		 {
			 printf("%s ",args[j]);
		 }
	}
	 printf("\n");
}

void copy_files(char **args,int arg_count)
{
	struct stat src_stat, dest_stat;
	char buff[100] ;
	int size  ;
        int fd[2] ;
	char *file1;
	if(args[1][0] == '-')
	{
   		token = args[2];   
	}
        else
	{
		token = args[1];
	}	
	fd[0] = open(token, O_RDONLY);
	if(fd[0] != -1)
	{ 
		file1 = token ;
		if(args[1][0] == '-')
		{
    			token =  args[3];
		}
		else
		{
			token = args[2];
		}
		if(token != NULL && strcmp(token," ")!=0)
		{
			if(append_flag==1)
			{  
				fd[1] = open(token,  O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR );
			}
			else
			{
				fd[1] = open(token, O_WRONLY |  O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
			}
			if(fd[1] != -1)
			{
				/* check if two files are the same by comparing their inodes and device ID
      			      * first get information about the destination file and source file 
			      * and then compare inode and device IDs */
				if(stat(token, &dest_stat)==0 && stat(file1, &src_stat)==0 && src_stat.st_ino == dest_stat.st_ino && src_stat.st_dev == dest_stat.st_dev)
				{
					fflush(stdout);
				    	fprintf(stderr,"mtcp: '%s' and '%s' are the same file\n",file1,token);
				}
				else
				{
					do
			       		{
			 			size = read(fd[0],buff,100);
						write(fd[1],buff,size);
					}while(size ==100);
				}
				close(fd[0]);
				close(fd[1]);
			}
		}
		else
		{
			fflush(stdout);
			fprintf(stderr,"mycp: missing destination file operand\n");
			close(fd[0]); // Close the source file since the destination file is not open
                }
	}
	else
	{
		fflush(stdout);
		fprintf(stderr,"mycp: cannot stat '%s': No such file\n",token);
	}
}

void move_files(char **args)
{
	struct stat src_stat, dest_stat;
        char buff[100] ;
        int size  ;
        int fd[2] ;
        char *file1;
	if(args[1][0] == '-')
	{
	       	token = args[2];
	}
        else
	{
		token = args[1];
	}
	fd[0] = open(token, O_RDONLY);
	if(fd[0] != -1)
	{
		file1 = token ;
		if(args[1][0] =='-')
		{
		    	token = args[3];
		}
		else
		{
			token = args[2];
		}
		if(token != NULL && strcmp(token," ")!=0)
		{
			fd[1] = open(token, O_WRONLY);
			if(fd[1] != -1)
			{
				/* check if two files are the same by comparing their inodes and device I
                                 * first get information about the destination file and source file
                                 * and then compare inode and device IDs */
				if(stat(token, &dest_stat)==0 && stat(file1, &src_stat)==0 && src_stat.st_ino == dest_stat.st_ino && src_stat.st_dev == dest_stat.st_dev)
				{
					fflush(stdout);
					fprintf(stderr,"mymv: '%s' and '%s' are the same file\n",file1,token);
				}
				else					    
			       	{
					if(overwrite_flag==1)
					{
						do
						{
							size = read(fd[0],buff,100);
							write(fd[1],buff,size);
						}while(size==100);
						unlink(file1);
					}
					else
					{
						fflush(stdout);
						fprintf(stderr,"mymv: the file '%s' already exist\nUsage: mv [-f] if you want to overwrite\n",token);
					}
				}
				close(fd[0]);
				close(fd[1]);
				return ;
			}
			else
			{
				fd[1] = open(token, O_WRONLY |  O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
				do
				{
					size = read(fd[0],buff,100);
					write(fd[1],buff,size);
				}while(size==100);
				unlink(file1);
				close(fd[0]);
                                close(fd[1]);
                                return ;
			}
		}
		else
		{
			fflush(stdout);
			fprintf(stderr,"mymv: missing destination file operand\n");
			close(fd[0]); // Close the source file since the destination file is not open
	        }
   	}
	else
	{
		fflush(stdout);
		fprintf(stderr,"mymv: cannot stat '%s': No such file\n",token);
	}
}
void change_directory(char **args)
{
	char *path ;
        char cwd[100];
	// Get the directory argument from the input
        token = args[1];
	if(token != NULL && strcmp(token," ")!=0)
	{
		if(access(token,F_OK)==0)
		{
		      // Get the current working directory
	              if(getcwd(cwd, sizeof(cwd)) == NULL)
	              {
		        perror("getcwd error");
	              }	       
	             else
	             {
	         	// Allocate memory for the new path
                       path = malloc(strlen(cwd) + strlen(token) + 2); // +2 for '/' and '\0'	     
		       strcpy(path, cwd);
		       strcat(path, "/");
		       strcat(path, token);
	               // Change the current directory
		       if (chdir(path) != 0)
		       {
                         perror("chdir error");
		       }
		      free(path);
	              }
		}
		else
		{
			fflush(stdout);
			fprintf(stderr,"bash: cd: '%s': No such file or directory\n",token);
		}
	}
	else
	{
	      fprintf(stderr, "Usage: cd <directory>\n");
        }
}
void print_envir_variable()
{
	extern char** environ;
	int i =0 ;
        while(environ[i] !=NULL)
        {
                printf("%s\n", environ[i++]);
        }
}
void print_command_type(char **args)
{
	char* internal_commands[] = {"myenv","mycp","mymv","cd", "myecho", "exit", "mypwd", "myfree", "myuptime", "help" , "set", "unset", "export", "alias", "unalias", NULL};
	int internal_flag=0;
	int external_flag=0;
	int i= 0 ;
	char *path_env ;
	char *path ;
	char fullpath[1024];
	token = args[1];
	if(token != NULL && strcmp(token," ")!=0)
	{
		for(i = 0 ; internal_commands[i]!=NULL ; i++)
		{
			if(strcmp(token,internal_commands[i])==0)
			{
			    fprintf(stderr,"%s is a shell builtin\n",internal_commands[i]);
			    internal_flag = 1 ;
			    break ;
			}
		}
		if(internal_flag==0)
		{
			path_env = getenv("PATH");
		        path = strdup(path_env);
			path = strtok(path,":");
			while(path != NULL)
			{
				strcpy(fullpath,path);
				strcat(fullpath,"/");
				strcat(fullpath,token);
				if (access(fullpath, X_OK) == 0)
			       	{
                                     printf("%s is %s\n", token, fullpath);
                                     external_flag = 1;
                                      break;
                                }
				path = strtok(NULL,":");
			}
		}
		if(external_flag==0 && internal_flag==0)
		{
			fflush(stdout);
			fprintf(stderr,"bash: type: %s: not found\n",token);
		}

	}
	else
	{
		fprintf(stderr,"type: Enter the command\n");
	}
}
void external_command(char **args)
{
	if(strcmp(args[0],"grep")==0)
	{
		int i ;
		if((args[1][0] == '"') || (args[1][0] == '\''))
		{
			size_t len = strlen(args[1]);
			char *dup = strdup(args[1]);
			for(i = 1 ; i<len ;i++)
			{
				if((args[1][i]=='"') || (args[1][i] == '\''))
                                {
                                        args[1][i] = '\0' ;
                                        break ;
                                }
                        }
			if( i==(len-1) )
                        {
                                args[1] = (args[1]+1);
                        }
                        else
			{
                                dup[len-1] = '\0' ;
                                args[1] = strcat(args[1]+1,dup+(i+2));
			}
		}
	}
	
	int fd ;
	int external_flag = 0; 
	/* check if the command is external or not*/
	char *path_env = getenv("PATH");
        // Duplicate the PATH environment variable string
	char *path = strdup(path_env);
	char fullpath[1024];
	char *path_token = strtok(path,":");
        while(path_token != NULL)
	{
              strcpy(fullpath,path_token);
	      strcat(fullpath,"/");
	      strcat(fullpath,args[0]);
              if (access(fullpath, X_OK) == 0)
              {
                   external_flag = 1;
                   break;
              }
              path_token = strtok(NULL,":");
	}

	if(external_flag)
        {
		//Fork and exec the command
                pid_t pid = fork();
		if( pid>0 )
		{
			int status =0;
                        wait(&status);
		}
		else if( pid==0 )
		{
			//child process
			extern char **environ; // Use the environment of the current process
		        execve(fullpath, args , environ);
                        perror("execv");  // execv returns only on error
		        exit(1) ;
		}
	}
	else
       	{
            fprintf(stderr, "Command not found: %s\n", args[0]);
	}
}
void print_RAM_info()
{
	int fd ;
    	char buff[4096];
    	char line[100] ;
       	char *token ;
    	long value ;
    	long mem_total = 0, mem_used = 0 , mem_shared = 0 , mem_free = 0, mem_available =0 , buffers = 0, cached = 0;
    	long swap_total = 0, swap_used = 0 , swap_free = 0;
    	fd = open("/proc/meminfo",O_RDONLY);
    	if(fd != -1)
    	{
    		ssize_t bytes_read = read(fd,buff,sizeof(buff)-1);
    		if(bytes_read!=-1)
    		{
    			// Null-terminate the buffer to make it a proper string
			buff[bytes_read] = '\0';
		       // Close the file
		       close(fd);
		       token = strtok(buff,"\n");
		       while(token !=NULL)
		       {
   			       sscanf(token, "%s %ld kB",line, &value);
   			       if(strcmp(line,"MemTotal:")==0)
   			       {
    				       mem_total = value ;
   			       }
   			       else if(strcmp(line,"MemFree:")==0)
   			       {
   				       mem_free = value ;
   			       }
   			       else if(strcmp(line,"MemAvailable:")==0)
   			       {
   				       mem_available = value ;
   			       } 
   			       else if(strcmp(line,"Buffers:")==0)
   			       {
   				       buffers = value ;
   			       }
   			       else if(strcmp(line,"Cached:")==0)
   			       {
   				       cached = value ;
   			       }
   			       else if(strcmp(line,"Shmem:")==0)
   			       {
   				       mem_shared = value ;
   			       }
   			       else if(strcmp(line,"SwapTotal:")==0)
   			       {
   				       swap_total = value ;
   			       }
   			       else if(strcmp(line,"SwapFree:")==0)
			       {
				       swap_free = value ;
   			       }
   			       token = strtok(NULL,"\n");
		       }
   		       mem_used = mem_total - mem_free - buffers - cached;
   		       swap_used = swap_total - swap_free;
   		       printf("              total        used         free       shared   buff/cache    available\n");
   		       printf("Mem:  %12ld %12ld %12ld %12ld %12ld %12ld\n", mem_total, mem_used, mem_free, mem_shared, buffers + cached, mem_available);
   		       printf("Swap: %12ld %12ld %12ld\n", swap_total, swap_total - swap_free, swap_free);
    		}
    		else
    		{
    			perror("read");
    		}
	}    
	else 
	{
    		perror("open");
	}
}
void print_uptime()
{
	int fd;
	char buffer[128];
       	double uptime_seconds , idletime_seconds;
    	int uptime_hours, uptime_minutes , idletime_hours , idletime_minutes;
    	int uptime_days = 0 ;

        // Open the /proc/uptime file to read system uptime
        fd = open("/proc/uptime", O_RDONLY);
   	if (fd < 0) 
	{
		perror("open");
		return;
       	}

        // Read the contents of /proc/uptime
	if (read(fd, buffer, sizeof(buffer) - 1) <= 0)
       	{
		perror("read");
		close(fd);
		return;
      	}
    	close(fd);

        // Extract the uptime in seconds
        sscanf(buffer, "%lf %lf", &uptime_seconds,&idletime_seconds);
   
       // Calculate days, hours, and minutes for uptime
       uptime_hours = uptime_seconds / 3600;
       uptime_minutes = (uptime_seconds / 60) - (uptime_hours * 60);
       if (uptime_hours > 24) 
       {
	       uptime_days = uptime_hours / 24;
       	       uptime_hours = uptime_hours % 24;
       }

       // Calculate days, hours, and minutes for ideal process
       idletime_hours = idletime_seconds / 3600;
       idletime_minutes = (idletime_seconds / 60) - (idletime_hours * 60); 

       // Get current time
       time_t current_time = time(NULL);
       struct tm *local_time = localtime(&current_time);

       // Print current time
       printf(" %02d:%02d:%02d up ", local_time->tm_hour, local_time->tm_min, local_time->tm_sec);

       // Print uptime
       if (uptime_days > 0)
       {
     	       printf("%d day%s, %2d:%02d, ", uptime_days, (uptime_days > 1) ? "s" : "", uptime_hours, uptime_minutes);
       }
       else if (uptime_hours > 0)
       {
	       printf("%2d:%02d, ", uptime_hours, uptime_minutes);
       } 
       else
       {
       	       printf("%d min, ", uptime_minutes);
       }

       //print idle time
       printf("idle ");
       if (idletime_hours > 0)
       {
       	       printf("%2d:%02d, ", idletime_hours, idletime_minutes);
       }
       else
       {
	       printf("%d min, ", idletime_minutes);
       }

       // Get and print the number of logged-in users using /var/run/utmp
       fd = open("/var/run/utmp", O_RDONLY);
       if (fd < 0) 
       {
       	       perror("open");
       	       return;
       }
    
       int user_count = 0;
       struct utmp user;
       while (read(fd, &user, sizeof(struct utmp)) == sizeof(struct utmp))
       {
       	       if (user.ut_type == USER_PROCESS)
	       {
	   	       user_count++;
       	       }
       }
       close(fd);

       printf("%d user%s, ", user_count, user_count == 1 ? "" : "s");

       // Print load averages from /proc/loadavg
       fd = open("/proc/loadavg", O_RDONLY);
       if (fd < 0)
       {
       	       perror("open");
       	       return;
       }

       if (read(fd, buffer, sizeof(buffer) - 1) <= 0)
       {
       	       perror("read");
       	       close(fd);
       	       return;
       }
       close(fd);
       double load1, load2, load3;
       sscanf(buffer, "%lf %lf %lf", &load1, &load2, &load3);
       printf("load average: %.2f, %.2f, %.2f\n", load1, load2, load3);
}
char **check_redirection(char **args,int arg_count)
{
	/**************Ensure that flags are cleared**********/
	output_flag = 0 ;
	input_flag = 0 ;
	error_flag = 0 ; 
	check_flag = 0 ;
	/****************************************************/

	int fd ;
	for(int j = 0 ; j < arg_count ; j++)
	{
                 if((strcmp(args[j],">")==0) || (strcmp(args[j],"2>")==0) || (strcmp(args[j],"<")==0))
                                {
                                        /*****************Output redirection**********************/
                                        if(strcmp(args[j],">")==0)
                                        {
                                                fd = open(args[j+1], O_WRONLY |  O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
                                                if(fd!=-1)
                                                {
                                                        output_flag = 1 ;
                                                        original_stdout = dup(1) ;
                                                        redirect(fd,1);
						}
                                                else
                                                {
                                                        printf("bash: syntax error near unexpected token `newline'\n");
							check_flag = 1 ;
                                                }
                                        }
                                        /******************Error redirection**********************/
                                        else if(strcmp(args[j],"2>")==0)
                                        {
                                                fd = open(args[j+1], O_WRONLY |  O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
                                                if(fd!=-1)
                                                {
                                                        error_flag = 1 ;
                                                        original_stderror = dup(2) ;
                                                        redirect(fd,2);
                                                }
                                                else
                                                {
                                                        printf("bash: syntax error near unexpected token `newline'\n");
							check_flag = 1 ;
                                                }
                                        }
                                        /******************Input redirection**********************/
                                        else
                                        {
                                                fd = open(args[j+1], O_RDONLY);
                                                if(fd!=-1)
                                                {
                                                        input_flag = 1 ;
                                                        original_stdin = dup(0);
                                                        redirect(fd,0);
                                                }
                                                else
                                                {
                                                        if(args[j+1]==NULL)
                                                        {
                                                                printf("bash: syntax error near unexpected token `newline'\n");
								check_flag = 1 ;
                                                        }
                                                        else
                                                        {
                                                                printf("bash: %s: No such file or directory\n",args[j+1]);
								check_flag = 1 ;
                                                        }
                                                }
                                        }
                                        args[j]=NULL;
                                        }

        }
	return args ;
}
void redirect(int fd_old , int fd_new)
{
        close(fd_new);
        int fd = dup(fd_old);
        if(fd != fd_new)
        {
                perror("error");
        }
        close(fd_old);
}

void print_help(char **args,int arg_count)
{
	printf("'IMPORTANT'* All external commands are available.\n");
	printf("           * This code also supports redirection and piping.\n");
	printf(" ** INTERNAL COMMANDS  **\n");
	printf("These are all the supported internal commands.\n");
	printf("mypwd:   Print the name of the current working directory.\n\n");
	printf("myecho:  Write arguments to the standard output.\n\n");
	printf("mycp:  copy a file to another file \noptions : \n    -a   append the source content to the end of target file.\n\n");
	printf("mymv:  move a file to another place \noptions : \n   -f  to force overwriting the target file if exits.\n\n");
	printf("mytype:  print the type of command (builtin, external or unsupported).\n\n");
	printf("cd:    change the current working directory.\n\n");
	printf("myenv: print all the environment variable.\n\n"); 
	printf("myfree: print RAM info ( total size, used and free) and Swap area info (total size, used, free).\n\n");
	printf("myuptime: print the uptime for the system and the time spent in the idle process.\n\n");
}
