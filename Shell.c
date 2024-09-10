#include <unistd.h>     // For POSIX constants and functions (e.g., read, write, dup, close)
#include <string.h>     // For string manipulation functions (e.g., strtok, strcpy, strlen)
#include <stdio.h>      // For standard input and output functions (e.g., printf)
#include <stdlib.h>     // For general purpose functions (e.g., malloc, free, realloc)
#include <fcntl.h>      // For file control options (e.g., O_WRONLY, O_CREAT)
#include "command.h"    // Header file for command-related declarations
#include <sys/wait.h>   // For wait functions (e.g., wait)

// Define standard file descriptor constants
#define STDIN  0
#define STDOUT 1
#define STDERR 2 

// Define constants for maximum arguments, input size, and initial argument size
#define MAX_ARGS 50
#define MAX_INPUT 1024
#define INITIAL_ARG_SIZE 10

/***********************************************************/
/*                 REDIRECTION VARIABLES                    */
/***********************************************************/
// Declare external variables for redirection flags and original file descriptors
extern int output_flag ;
extern int input_flag  ;
extern int error_flag  ;
extern int check_flag ;
extern int original_stdout ;
extern int original_stdin ;
extern int original_stderror ;
/************************************************************/

/************************************************************/
// Define local variables for token and flags related to redirection
char *token ;
int append_flag = 0 ;
int overwrite_flag = 0;
/***********************************************************/

/* Main function to handle shell command input and execution */
int main()
{
        // Buffers for command input and processing
        char cmd[100];
        char pipe_cmd[100];
        int opt ;
        char command[100];
        ssize_t readsize = 0 ;
        int i =0 ;
        const char *shellmsg = "Enter the desired command:";
        
        // Infinite loop to continuously accept and process commands
        while(1)
        {
                int pipe_flag = 0 ; // Flag to indicate if piping is used
                token = NULL ; // Reset token
                i =0 ;
               // Initialize buffers with zeros
                memset(command, 0, sizeof(command));
                memset(cmd, 0, sizeof(cmd));
                write(STDOUT, shellmsg, strlen(shellmsg)); // Prompt user for command
                readsize = read(STDIN, command, 100); // Read user input

                command[readsize-1] = '\0'; // Ensure the buffer is null-terminated
                // Copy the command to other buffers for processing
                strcpy(cmd, command);
                strcpy(pipe_cmd, command);
                
                // Tokenize the input into arguments
                 char **args = malloc(INITIAL_ARG_SIZE * sizeof(char *));
                 int arg_count = 0 ;
                 int arg_size = INITIAL_ARG_SIZE;
                 token = strtok(cmd, " ");
                 while (token != NULL)
                 {
                            // Reallocate memory for arguments if needed
                            if (arg_count >= arg_size - 1)
                            {
                               arg_size *= 2;  // Double the size
                               args = realloc(args, arg_size * sizeof(char *));  // Reallocate memory
                            }
                            args[arg_count++] = token; // Store token as argument
                            token = strtok(NULL, " ");
                 }
                 args[arg_count] = NULL;  // Null-terminate the argument list

                /*************************************************************/
                /*               Piping Between Two Processes                */
                /*************************************************************/
                // Check for pipe operator and handle piping
                for(int i  = 0 ; i < arg_count ; i++)
                {
                        if(strcmp(args[i],"|") == 0)
                        {
                                pipe_flag = 1 ; // Set pipe flag
                                int pipefd ; // File descriptor for the pipe
                                char **args_of_cmd1 = malloc(INITIAL_ARG_SIZE * sizeof(char *));
                                char **args_of_cmd2 = malloc(INITIAL_ARG_SIZE * sizeof(char *));
                                int arg_count_of_cmd1 = 0 , arg_count_of_cmd2 = 0;

                                // Parse the command into two parts separated by '|'
                                char *cmd1 = strtok(pipe_cmd, "|");
                                char *cmd2 = strtok(NULL, "|");
                                /******************** Process #1 ***********************/
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

                                /********************** Process #2 *******************************/
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
                                // Open a temporary file to act as a pipe
                                pipefd = open("pipe", O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
                                int original_stdout = dup(STDOUT);  // Save the original stdout
                                redirect(pipefd, STDOUT); // Redirect stdout to the pipe
                                check_command(args_of_cmd1, arg_count_of_cmd1, token_of_cmd1); // Execute command 1
                                redirect(original_stdout, STDOUT); // Restore original stdout
                                close(pipefd); // Close the pipe file

                                /********************************************************************/
                                /*                    Execute the second command                    */
                                /*******************************************************************/
                                 pipefd = open("pipe", O_RDONLY); // Open the pipe for reading
                                 int original_stdin = dup(STDIN);  // Save the original stdin
                                 redirect(pipefd, STDIN); // Redirect stdin from the pipe
                                 check_command(args_of_cmd2, arg_count_of_cmd2, token_of_cmd2); // Execute command 2
                                 redirect(original_stdin, STDIN);   // Restore original stdin
                                 close(pipefd); // Close the pipe file
                                 unlink("pipe"); // Remove the temporary pipe file
                        }
                }
                // If no pipe flag was set, execute the command normally
                if(pipe_flag == 0)
                {
                        check_command(args, arg_count, command);
                }
        }
        return 0 ; // Return success
}
void check_command(char **args, int arg_count, char *token)
{
        /****************************************************************/
        /*                        CHECK REDIRECTION                     */
        /* Check and handle any redirection operators (input/output/error) */
        /* Adjust the argument list accordingly                        */
        /****************************************************************/
        // Process redirection and update argument list
        args = check_redirection(args, arg_count);

        // Recalculate the number of arguments after redirection handling
        int k = 0;
        arg_count = 0;
        while (args[k] != NULL)
        {
                arg_count++;
                k++;
        }

        // Tokenize the command if there are multiple arguments
        if (arg_count > 1)
        {
                token = strtok(token, " ");
        }
        else
        {
                /******* Remove extraneous spaces from the token *******/
                int i = 0, j = 0;
                while (token[i])
                {
                        if (token[i] != ' ')
                        {
                                token[j++] = token[i];
                        }
                        i++;
                }
                token[j] = '\0';  // Null-terminate the cleaned string
        }

        // Execute the appropriate command based on the token
        if (check_flag == 0)
        {
                // Handle built-in commands
                if (strcmp(token, "mypwd") == 0)
                {
                        print_working_directory();
                }
                else if (strcmp(token, "myecho") == 0)
                {
                        print_user_input(args, arg_count);
                }
                else if (strcmp(token, "exit") == 0)
                {
                        printf("Good Bye\n");
                        exit(0);
                }
                else if (strcmp(token, "mycp") == 0)
                {
                        append_flag = 0; // Reset append flag before processing options
                        if (arg_count == 1)
                        {
                                fflush(stdout);
                                fprintf(stderr, "Enter the source and destination files\n");
                        }
                        else
                        {
                                // Handle options for copy command (e.g., -a for append)
                                if (args[1][0] == '-')
                                {
                                        if (strcmp(args[1], "-a") == 0)
                                        {
                                                append_flag = 1;
                                                token = strtok(NULL, " ");
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
                else if (strcmp(token, "mymv") == 0)
                {
                        overwrite_flag = 0; // Reset overwrite flag before processing options
                        if (arg_count == 1)
                        {
                                fflush(stdout);
                                fprintf(stderr, "Enter the source and destination files\n");
                        }
                        else
                        {
                                // Handle options for move command (e.g., -f for force overwrite)
                                if (args[1][0] == '-')
                                {
                                        if (strcmp(args[1], "-f") == 0)
                                        {
                                                overwrite_flag = 1;
                                                token = strtok(NULL, " ");
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
                else if (strcmp(token, "cd") == 0)
                {
                        change_directory(args);
                }
                else if (strcmp(token, "myenv") == 0)
                {
                        print_envir_variable();
                }
                else if (strcmp(token, "type") == 0)
                {
                        print_command_type(args);
                }
                else if (strcmp(token, "myfree") == 0)
                {
                        print_RAM_info();
                }
                else if (strcmp(token, "myuptime") == 0)
                {
                        print_uptime();
                }
                else if (strcmp(token, "help") == 0)
                {
                        print_help();
                }
		else if(strcmp(token,"")==0)
		{
		}
                else
                {
                        // Execute external commands
                        external_command(args, arg_count);
                }
        }

        // Free dynamically allocated memory for arguments
        free(args);

        /***************************************************/
        /*             RESTORE ORIGINAL FILE DESCRIPTORS  */
        /* Restore file descriptors if they were redirected */
        /***************************************************/
        if (output_flag == 1)
        {
                redirect(original_stdout, STDOUT);
        }
        if (input_flag == 1)
        {
                redirect(original_stdin, STDIN);
        }
        if (error_flag == 1)
        {
                redirect(original_stderror, STDERR);
        }
}

