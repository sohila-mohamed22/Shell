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
/*                 REDIRECTION VARIABLES                    */
/***********************************************************/
/* Flags and file descriptors for handling redirection */
int output_flag =  0 ;    // Indicates if output redirection is active
int input_flag  =  0 ;    // Indicates if input redirection is active
int error_flag  =  0 ;    // Indicates if error redirection is active
int check_flag = 0 ;      // Flag to check if an error occurred
int original_stdout ;     // File descriptor for original standard output
int original_stdin ;      // File descriptor for original standard input
int original_stderror ;   // File descriptor for original standard error
/************************************************************/

/* External variables used for redirection and argument handling */
extern char *token ;        // Global variable for tokenized command input
extern int append_flag ;    // Flag indicating if append mode is enabled
extern int overwrite_flag;  // Flag indicating if overwrite mode is enabled
extern char **args ;        // Argument list for commands
extern int arg_count ;      // Count of arguments passed to the command

void redirect(int fd_old , int fd_new);

/************************************************************/
/* Function to print the current working directory        */
/************************************************************/
void print_working_directory()
{
        char  buffer[100];         // Buffer to hold the current directory path
        char *directory ;          // Pointer to the directory path
        directory = getcwd(buffer, 100);  // Get current working directory
        if(directory != NULL)
        {
                printf("%s\n", directory);  // Print the directory path
        }
        else
        {
                perror("Error");  // Print error if unable to get directory
        }
}

/************************************************************/
/* Function to print user input arguments                 */
/************************************************************/
void print_user_input(char **args, int arg_count)
{
        int i, j;
        for(j = 1; j < arg_count; j++)
        {
                // Handle quoted arguments
                if((args[j][0] == '"') || (args[j][0] == '\''))
                {
                        size_t len = strlen(args[j]);    // Length of the argument
                        char *dup = strdup(args[j]);     // Duplicate argument for manipulation
                        for(i = 1; i < len; i++)
                        {
                                // Remove trailing quote characters
                                if((args[j][i] == '"') || (args[j][i] == '\''))
                                {
                                        args[j][i] = '\0';
                                }
                        }
                        // Adjust argument to remove surrounding quotes
                        if(i == (len - 1))
                        {
                                args[j] = (args[j] + 1);  // Remove starting quote
                        }
                        else
                        {
                                dup[len - 1] = '\0';  // Remove ending quote
                                args[j] = strcat(args[j] + 1, dup + (i + 2)); // Concatenate without quotes
                        }
                }
        }

        // Print the arguments, excluding the command itself
        if(arg_count > 1)
        {
                for(int j = 1; j < arg_count; j++)
                {
                        printf("%s ", args[j]);
                }
        }
        printf("\n");  // Print a newline after the arguments
}
/**
 * @brief Copies the contents of a source file to a destination file.
 * 
 * This function performs file copying operations based on the provided command-line arguments.
 * It supports options for appending to or overwriting the destination file. It also checks 
 * whether the source and destination files are the same, and reports an error if so. 
 * 
 * The function handles the following scenarios:
 * - If the `-a` flag is provided, it appends the content to the destination file.
 * - If the `-f` flag is provided, it forces overwriting the destination file.
 * - If no flags are provided, it defaults to overwriting the destination file.
 * 
 * @param args       An array of command-line arguments, where the source and destination files 
 *                   are specified.
 * @param arg_count  The number of arguments in the `args` array.
 */
void copy_files(char **args, int arg_count)
{
        struct stat src_stat, dest_stat;  // Structures to hold file status information
        char buff[100];                  // Buffer for reading file data
        int size;                       // Number of bytes read or written
        int fd[2];                      // File descriptors for source and destination files
        char *file1;                    // Pointer to the source file path

        // Determine the source file based on the presence of an option flag
        if (args[1][0] == '-')
        {
                token = args[2];   // If option flag is present, source file is at args[2]
        }
        else
        {
                token = args[1];   // Otherwise, source file is at args[1]
        }

        // Open the source file for reading
        fd[0] = open(token, O_RDONLY);
        if (fd[0] != -1)
        {
                file1 = token;    // Store the source file path
                if (args[1][0] == '-')
                {
                        token = args[3];  // If option flag, destination file is at args[3]
                }
                else
                {
                        token = args[2];  // Otherwise, destination file is at args[2]
                }

                // Check if destination file path is valid and not just a space
                if (token != NULL && strcmp(token, " ") != 0)
                {
                        // Open the destination file with appropriate flags
                        if (append_flag == 1)
                        {
                                fd[1] = open(token, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
                        }
                        else
                        {
                                fd[1] = open(token, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
                        }

                        if (fd[1] != -1)
                        {
                                /* Check if the source and destination files are the same */
                                if (stat(token, &dest_stat) == 0 && stat(file1, &src_stat) == 0 &&
                                    src_stat.st_ino == dest_stat.st_ino && src_stat.st_dev == dest_stat.st_dev)
                                {
                                        // Print an error message if the files are the same
                                        fflush(stdout);
                                        fprintf(stderr, "mycp: '%s' and '%s' are the same file\n", file1, token);
                                }
                                else
                                {
                                        /* Copy data from the source file to the destination file */
                                        do
                                        {
                                                size = read(fd[0], buff, 100);  // Read data from the source file
                                                write(fd[1], buff, size);      // Write data to the destination file
                                        } while (size == 100);  // Continue until end of file
                                }

                                // Close file descriptors
                                close(fd[0]);
                                close(fd[1]);
                        }
                }
                else
                {
                        // Print an error message if the destination file is missing
                        fflush(stdout);
                        fprintf(stderr, "mycp: missing destination file operand\n");
                        close(fd[0]);  // Close the source file
                }
        }
        else
        {
                // Print an error message if the source file does not exist
                fflush(stdout);
                fprintf(stderr, "mycp: cannot stat '%s': No such file\n", token);
        }
}
/**
 * @brief Moves a file from a source to a destination.
 * 
 * This function moves the contents of a source file to a destination file. It supports an option
 * to forcefully overwrite the destination file if it already exists. The function also checks if
 * the source and destination files are the same and reports an error if so.
 * 
 * The function handles the following scenarios:
 * - If the `-f` flag is provided, it forces overwriting the destination file if it exists.
 * - If no flags are provided, it defaults to moving the file without overwriting if the destination
 *   file already exists.
 * - If the destination file is missing, an error is reported.
 * 
 * @param args  An array of command-line arguments, where the source and destination files are specified.
 */
void move_files(char **args)
{
        struct stat src_stat, dest_stat;  // Structures to hold file status information
        char buff[100];                  // Buffer for reading file data
        int size;                       // Number of bytes read or written
        int fd[2];                      // File descriptors for source and destination files
        char *file1;                    // Pointer to the source file path

        // Determine the source file based on the presence of an option flag
        if (args[1][0] == '-')
        {
                token = args[2];   // If option flag is present, source file is at args[2]
        }
        else
        {
                token = args[1];   // Otherwise, source file is at args[1]
        }

        // Open the source file for reading
        fd[0] = open(token, O_RDONLY);
        if (fd[0] != -1)
        {
                file1 = token;    // Store the source file path
                if (args[1][0] == '-')
                {
                        token = args[3];  // If option flag, destination file is at args[3]
                }
                else
                {
                        token = args[2];  // Otherwise, destination file is at args[2]
                }

                // Check if destination file path is valid and not just a space
                if (token != NULL && strcmp(token, " ") != 0)
                {
                        // Try to open the destination file for writing
                        fd[1] = open(token, O_WRONLY);
                        if (fd[1] != -1)
                        {
                                /* Check if the source and destination files are the same */
                                if (stat(token, &dest_stat) == 0 && stat(file1, &src_stat) == 0 &&
                                    src_stat.st_ino == dest_stat.st_ino && src_stat.st_dev == dest_stat.st_dev)
                                {
                                        // Print an error message if the files are the same
                                        fflush(stdout);
                                        fprintf(stderr, "mymv: '%s' and '%s' are the same file\n", file1, token);
                                }
                                else
                                {
                                        if (overwrite_flag == 1)
                                        {
                                                /* Move file content to destination and delete source file */
                                                do
                                                {
                                                        size = read(fd[0], buff, 100);  // Read data from the source file
                                                        write(fd[1], buff, size);      // Write data to the destination file
                                                } while (size == 100);  // Continue until end of file
                                                unlink(file1);  // Delete the source file
                                        }
                                        else
                                        {
                                                // Print an error message if destination file exists and overwrite is not allowed
                                                fflush(stdout);
                                                fprintf(stderr, "mymv: the file '%s' already exists\nUsage: mv [-f] if you want to overwrite\n", token);
                                        }
                                }
                                close(fd[0]);
                                close(fd[1]);
                                return;
                        }
                        else
                        {
                                // Open the destination file for writing (create if not exists)
                                fd[1] = open(token, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
                                do
                                {
                                        size = read(fd[0], buff, 100);  // Read data from the source file
                                        write(fd[1], buff, size);      // Write data to the destination file
                                } while (size == 100);  // Continue until end of file
                                unlink(file1);  // Delete the source file
                                close(fd[0]);
                                close(fd[1]);
                                return;
                        }
                }
                else
                {
                        // Print an error message if the destination file is missing
                        fflush(stdout);
                        fprintf(stderr, "mymv: missing destination file operand\n");
                        close(fd[0]);  // Close the source file
                }
        }
        else
        {
                // Print an error message if the source file does not exist
                fflush(stdout);
                fprintf(stderr, "mymv: cannot stat '%s': No such file\n", token);
        }
}
/**
 * @brief Changes the current working directory to the specified path.
 * 
 * This function changes the current working directory to the directory specified in the `args` array.
 * It performs the following steps:
 * - Checks if the directory argument is valid and not just a space.
 * - Verifies if the specified directory exists.
 * - Constructs the new path by appending the specified directory to the current working directory.
 * - Attempts to change to the new directory using `chdir()`.
 * - Prints an error message if the specified directory does not exist or if changing the directory fails.
 * 
 * @param args  An array of command-line arguments, where `args[1]` is expected to be the path of the directory to change to.
 */
void change_directory(char **args)
{
        char *path;  // Pointer for the new directory path
        char cwd[100];  // Buffer to store the current working directory

        // Get the directory argument from the input
        token = args[1];
        if (token != NULL && strcmp(token, " ") != 0)
        {
                if (access(token, F_OK) == 0)
                {
                      // Get the current working directory
                      if (getcwd(cwd, sizeof(cwd)) == NULL)
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
                        fprintf(stderr, "bash: cd: '%s': No such file or directory\n", token);
                }
        }
        else
        {
              fprintf(stderr, "Usage: cd <directory>\n");
        }
}

/**
 * @brief Prints all the environment variables.
 * 
 * This function iterates through the `environ` array and prints each environment variable.
 * It is useful for displaying all the current environment variables and their values.
 */
void print_envir_variable()
{
        extern char **environ;  // External pointer to environment variables
        int i = 0;
        while (environ[i] != NULL)
        {
                printf("%s\n", environ[i++]);
        }
}
/**
 * @brief Determines whether a given command is an internal shell command or an external executable.
 * 
 * This function checks if the command provided in the `args` array is one of the predefined internal shell commands.
 * If it is not an internal command, it then checks if the command is an external executable by searching the directories
 * listed in the `PATH` environment variable. The function prints whether the command is a shell builtin or an executable,
 * or reports that the command was not found.
 * 
 * @param args  An array of command-line arguments, where `args[1]` is expected to be the command to check.
 */
void print_command_type(char **args)
{
        // List of internal shell commands
        char* internal_commands[] = {"myenv", "mycp", "mymv", "cd", "myecho", "exit", "mypwd", "myfree", "myuptime", "help", "set", "unset", "export", "alias", "unalias", NULL};
        int internal_flag = 0;   // Flag to indicate if the command is an internal command
        int external_flag = 0;   // Flag to indicate if the command is an external executable
        int i = 0;
        char *path_env;           // Environment variable for PATH
        char *path;              // Path from PATH environment variable
        char fullpath[1024];     // Buffer to store full path of executable

        // Get the command to check
        token = args[1];
        if (token != NULL && strcmp(token, " ") != 0)
        {
                // Check if the command is an internal command
                for (i = 0; internal_commands[i] != NULL; i++)
                {
                        if (strcmp(token, internal_commands[i]) == 0)
                        {
                            fprintf(stderr, "%s is a shell builtin\n", internal_commands[i]);
                            internal_flag = 1;
                            break;
                        }
                }

                // If not an internal command, check for external executable
                if (internal_flag == 0)
                {
                        path_env = getenv("PATH");     // Get the PATH environment variable
                        path = strdup(path_env);       // Duplicate PATH string to tokenize
                        path = strtok(path, ":");      // Tokenize PATH by ':'
                        while (path != NULL)
                        {
                                // Construct the full path of the command
                                strcpy(fullpath, path);
                                strcat(fullpath, "/");
                                strcat(fullpath, token);
                                // Check if the file is executable
                                if (access(fullpath, X_OK) == 0)
                                {
                                     printf("%s is %s\n", token, fullpath);
                                     external_flag = 1;
                                      break;
                                }
                                path = strtok(NULL, ":");    // Get next directory in PATH
                        }
            
                }

                // If neither internal nor external command was found
                if (external_flag == 0 && internal_flag == 0)
                {
                        fflush(stdout);
                        fprintf(stderr, "bash: type: %s: not found\n", token);
                }
        }
        else
        {
                fprintf(stderr, "type: Enter the command\n");
        }
}
/**
 * @brief Executes an external command by forking a new process and using `execve`.
 * 
 * This function checks if the command specified in `args` is an external executable
 * by searching the directories listed in the `PATH` environment variable. If the command
 * is found, it forks a new process to execute the command. If the command is not found,
 * it prints an error message to `stderr`.
 * 
 * @param args      An array of command-line arguments where `args[0]` is the command
 *                  to be executed and the rest are its arguments.
 * @param arg_count The number of arguments in the `args` array.
 */
void external_command(char **args, int arg_count)
{
        // Process any quotes around the arguments (specifically for 'grep' command)
        if(arg_count > 1 && strcmp(args[0], "grep") == 0)
        {
                if(args[1][0] == '"' || args[1][0] == '\'')
                {
                        size_t len = strlen(args[1]);
                        char *dup = strdup(args[1]);
                        int i;
                        for(i = 1; i < len; i++)
                        {
                                if(args[1][i] == '"' || args[1][i] == '\'')
                                {
                                        args[1][i] = '\0';  // End the quoted string
                                        break;
                                }
                        }
                        if(i == len - 1)
                        {
                                args[1] = (args[1] + 1);  // Remove leading quote
                        }
                        else
                        {
                                dup[len - 1] = '\0';
                                args[1] = strcat(args[1] + 1, dup + (i + 2));  // Remove surrounding quotes
                        }
                }
        }

        int fd;
        int external_flag = 0;  // Flag to indicate if the command is external
        char *path_env = getenv("PATH");  // Get the PATH environment variable
        char *path = strdup(path_env);    // Duplicate PATH for tokenization
        char fullpath[1024];             // Buffer for constructing the full path of the command
        char *path_token = strtok(path, ":"); // Tokenize PATH by ':'

        // Search for the command in the directories listed in PATH
        while(path_token != NULL)
        {
              strcpy(fullpath, path_token);
              strcat(fullpath, "/");
              strcat(fullpath, args[0]);
              if (access(fullpath, X_OK) == 0)
              {
                   external_flag = 1;  // Command found and is executable
                   break;
              }
              path_token = strtok(NULL, ":");  // Get the next directory in PATH
        }

        if(external_flag)
        {
                // Fork a new process to execute the command
                pid_t pid = fork();
                if(pid > 0)
                {
                        // Parent process: wait for the child process to complete
                        int status = 0;
                        wait(&status);
                }
                else if(pid == 0)
                {
                        // Child process: execute the command
                        extern char **environ; // Use the environment of the current process
                        execve(fullpath, args, environ);
                        perror("execve");  // execve returns only on error
                        exit(1);  // Exit if execve fails
                }
        }
        else
        {
            // Command not found in PATH
            fprintf(stderr, "Command not found: %s\n", args[0]);
        }

        free(path); // Free the duplicated PATH string
}
/**
 * @brief Prints detailed information about the system's memory and swap usage.
 * 
 * This function reads data from the `/proc/meminfo` file, which provides information about the system's memory and swap usage.
 * It parses this data to extract values for total memory, used memory, free memory, shared memory, buffer/cache memory, available memory,
 * and swap memory. The function then prints these values in a human-readable format.
 */
void print_RAM_info()
{
        int fd;
        char buff[4096];
        char line[100];
        char *token;
        long value;
        long mem_total = 0, mem_used = 0, mem_shared = 0, mem_free = 0, mem_available = 0;
        long buffers = 0, cached = 0;
        long swap_total = 0, swap_used = 0, swap_free = 0;

        // Open the /proc/meminfo file for reading
        fd = open("/proc/meminfo", O_RDONLY);
        if (fd != -1)
        {
                // Read the contents of /proc/meminfo into the buffer
                ssize_t bytes_read = read(fd, buff, sizeof(buff) - 1);
                if (bytes_read != -1)
                {
                        // Null-terminate the buffer to treat it as a string
                        buff[bytes_read] = '\0';
                       
                        // Close the file descriptor as it's no longer needed
                        close(fd);
                       
                        // Tokenize the buffer by lines
                        token = strtok(buff, "\n");
                       
                        // Parse each line to extract memory information
                        while (token != NULL)
                        {
                                sscanf(token, "%s %ld kB", line, &value);
                                if (strcmp(line, "MemTotal:") == 0)
                                {
                                        mem_total = value;
                                }
                                else if (strcmp(line, "MemFree:") == 0)
                                {
                                        mem_free = value;
                                }
                                else if (strcmp(line, "MemAvailable:") == 0)
                                {
                                        mem_available = value;
                                }
                                else if (strcmp(line, "Buffers:") == 0)
                                {
                                        buffers = value;
                                }
                                else if (strcmp(line, "Cached:") == 0)
                                {
                                        cached = value;
                                }
                                else if (strcmp(line, "Shmem:") == 0)
                                {
                                        mem_shared = value;
                                }
                                else if (strcmp(line, "SwapTotal:") == 0)
                                {
                                        swap_total = value;
                                }
                                else if (strcmp(line, "SwapFree:") == 0)
                                {
                                        swap_free = value;
                                }
                                // Move to the next line
                                token = strtok(NULL, "\n");
                        }
                       
                        // Calculate used memory and swap
                        mem_used = mem_total - mem_free - buffers - cached;
                        swap_used = swap_total - swap_free;
                       
                        // Print the memory and swap usage in a formatted table
                        printf("              total        used         free       shared   buff/cache    available\n");
                        printf("Mem:  %12ld %12ld %12ld %12ld %12ld %12ld\n", mem_total, mem_used, mem_free, mem_shared, buffers + cached, mem_available);
                        printf("Swap: %12ld %12ld %12ld\n", swap_total, swap_total - swap_free, swap_free);
                }
                else
                {
                        // Print an error message if reading the file fails
                        perror("read");
                }
        }
        else
        {
                // Print an error message if opening the file fails
                perror("open");
        }
}
/**
 * @brief Prints the system uptime, idle time, number of logged-in users, and load averages.
 *
 * This function performs the following tasks:
 * 1. Opens and reads the `/proc/uptime` file to get the system uptime and idle time.
 * 2. Calculates the days, hours, and minutes of uptime and idle time.
 * 3. Gets the current local time and prints it.
 * 4. Displays the formatted uptime and idle time.
 * 5. Opens and reads the `/var/run/utmp` file to count the number of logged-in users.
 * 6. Opens and reads the `/proc/loadavg` file to get the system load averages.
 * 7. Prints the number of logged-in users and the load averages.
 * 
 * This function utilizes system files and standard library functions to gather and format 
 * system information for display. Error handling is included for file operations to 
 * ensure that appropriate messages are displayed in case of failures.
 */
void print_uptime()
{
    int fd;
    char buffer[128];
    double uptime_seconds, idletime_seconds;
    int uptime_hours, uptime_minutes, idletime_hours, idletime_minutes;
    int uptime_days = 0;

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
    sscanf(buffer, "%lf %lf", &uptime_seconds, &idletime_seconds);

    // Calculate days, hours, and minutes for uptime
    uptime_hours = uptime_seconds / 3600;
    uptime_minutes = (uptime_seconds / 60) - (uptime_hours * 60);
    if (uptime_hours > 24)
    {
        uptime_days = uptime_hours / 24;
        uptime_hours = uptime_hours % 24;
    }

    // Calculate days, hours, and minutes for idle time
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

    // Print idle time
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
/**
 * @brief Parses and handles I/O redirection in a command.
 *
 * This function examines the command arguments for I/O redirection symbols (`>`, `2>`, `<`)
 * and modifies the file descriptors as necessary. It also handles errors related to invalid
 * redirection attempts.
 *
 * The function performs the following tasks:
 * 1. Resets redirection flags and variables to default states.
 * 2. Iterates through command arguments to identify redirection symbols.
 * 3. Opens files for redirection based on the symbols found:
 *    - `>` for output redirection
 *    - `2>` for error redirection
 *    - `<` for input redirection
 * 4. Updates file descriptors to achieve the desired redirection.
 * 5. Prints appropriate error messages if redirection fails or files are missing.
 *
 * @param args Array of command arguments including potential redirection symbols.
 * @param arg_count Number of command arguments.
 * @return Modified array of command arguments with redirection symbols replaced by NULL.
 */
char **check_redirection(char **args, int arg_count)
{
    /**************Ensure that flags are cleared**********/
    output_flag = 0;
    input_flag = 0;
    error_flag = 0;
    check_flag = 0;
    /****************************************************/

    int fd;
    for(int j = 0; j < arg_count; j++)
    {
        if((strcmp(args[j], ">") == 0) || (strcmp(args[j], "2>") == 0) || (strcmp(args[j], "<") == 0))
        {
            /*****************Output redirection**********************/
            if(strcmp(args[j], ">") == 0)
            {
                fd = open(args[j + 1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
                if(fd != -1)
                {
                    output_flag = 1;
                    original_stdout = dup(STDOUT_FILENO);
                    redirect(fd, STDOUT_FILENO);
                }
                else
                {
                    printf("bash: syntax error near unexpected token `newline'\n");
                    check_flag = 1;
                }
            }
            /******************Error redirection**********************/
            else if(strcmp(args[j], "2>") == 0)
            {
                fd = open(args[j + 1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
                if(fd != -1)
                {
                    error_flag = 1;
                    original_stderror = dup(STDERR_FILENO);
                    redirect(fd, STDERR_FILENO);
                }
                else
                {
                    printf("bash: syntax error near unexpected token `newline'\n");
                    check_flag = 1;
                }
            }
            /******************Input redirection**********************/
            else
            {
                fd = open(args[j + 1], O_RDONLY);
                if(fd != -1)
                {
                    input_flag = 1;
                    original_stdin = dup(STDIN_FILENO);
                    redirect(fd, STDIN_FILENO);
                }
                else
                {
                    if(args[j + 1] == NULL)
                    {
                        printf("bash: syntax error near unexpected token `newline'\n");
                        check_flag = 1;
                    }
                    else
                    {
                        printf("bash: %s: No such file or directory\n", args[j + 1]);
                        check_flag = 1;
                    }
                }
            }
            args[j] = NULL;
        }
    }
    return args;
}

/**
 * @brief Redirects a file descriptor.
 *
 * This function duplicates an old file descriptor to a new file descriptor,
 * effectively redirecting I/O operations from one descriptor to another.
 *
 * @param fd_old The file descriptor to duplicate.
 * @param fd_new The target file descriptor for redirection.
 */
void redirect(int fd_old, int fd_new)
{
    close(fd_new);
    int fd = dup(fd_old);
    if(fd != fd_new)
    {
        perror("error");
    }
    close(fd_old);
}
/**
 * @brief Prints the help information for the available commands.
 *
 * This function displays a help message outlining the supported internal commands, their usage,
 * and additional features of the shell. It provides information on both internal commands and
 * external commands, along with details about redirection and piping.
 *
 * @param args Array of arguments (not used in this function).
 * @param arg_count The number of arguments (not used in this function).
 */
void print_help(char **args, int arg_count)
{
        printf("********** IMPORTANT INFORMATION **********\n");
        printf("* All external commands are available.\n");
        printf("* This shell supports redirection and piping.\n\n");

        printf("********** INTERNAL COMMANDS **********\n");
        printf("Below is a list of supported internal commands and their descriptions:\n\n");

        printf("1. mypwd\n");
        printf("   - Description: Prints the name of the current working directory.\n\n");

        printf("2. myecho\n");
        printf("   - Description: Writes the provided arguments to the standard output.\n\n");

        printf("3. mycp\n");
        printf("   - Description: Copies a file to another file.\n");
        printf("   - Options:\n");
        printf("     -a  Append the source content to the end of the target file.\n\n");

        printf("4. mymv\n");
        printf("   - Description: Moves a file to a different location.\n");
        printf("   - Options:\n");
        printf("     -f  Forcefully overwrite the target file if it already exists.\n\n");

        printf("5. mytype\n");
        printf("   - Description: Displays the type of the specified command (builtin, external, or unsupported).\n\n");

        printf("6. cd\n");
        printf("   - Description: Changes the current working directory to the specified path.\n\n");

        printf("7. myenv\n");
        printf("   - Description: Prints all environment variables.\n\n");

        printf("8. myfree\n");
        printf("   - Description: Prints information about system memory (RAM) and swap area.\n");
        printf("   - Details:\n");
        printf("     - RAM: Total size, used size, free size, shared memory, buffers, and cached memory.\n");
        printf("     - Swap: Total size, used size, and free size.\n\n");

        printf("9. myuptime\n");
        printf("   - Description: Displays the system uptime and the time spent in the idle process.\n\n");

        printf("For more information on usage, please refer to the command's specific help documentation or man pages.\n");
}


