#ifndef PRINT_WORKING_DIRECTORY_H
#define PRINT_WORKING_DIRECTORY_H
void check_command(char **args,int arg_count,char *token);
void print_working_directory();
void print_user_input(char **args,int arg_count);
void copy_files(char **args);
void move_files(char **args);
void change_directory(char **args);
void print_envir_variable();
void print_command_type(char **args);
void external_command(char **args,int arg_count);
void print_help();
void print_RAM_info();
void print_uptime();
void redirect(int fd_old , int fd_new);
char **check_redirection(char **args,int arg_count);
#endif // PRINT_WORKING_DIRECTORY_H


