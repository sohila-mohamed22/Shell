# Custom Shell 
## Overview
This project implements a custom shell that provides both internal and external command execution. The shell is designed to mimic the functionality of standard Unix-like shells, with additional features and custom internal commands. It supports essential shell features such as input/output redirection and process piping.
## Current Features
### 1. Internal Commands
The shell currently supports the following internal commands:
* mycp: Copy files and directories.
* mymv: Move or rename files and directories.
* type: Display the type of a command (whether it's an internal command or an external command).
* help: Provide help information for internal commands.
* exit: Exit the shell.
* cd: Change the current working directory.
* myenvir: Display environment variables.
### 3. External Commands
* Execution of External Commands: The shell supports executing any external command that is available in the system's PATH environment variable.
## New Features to be Added
### 1. Additional Internal Commands
* free Command: This command will print detailed information about the system's memory usage, including:
RAM Information:
Total size
Used memory
Free memory
Swap Area Information:
Total size
Used swap
Free swap
* uptime Command: This command will display the system uptime and the time spent in the idle process, providing insights into how long the system has been running and its idle state duration.

### 2. Shell Features
* Input, Output, and Error Redirection:
The shell will support redirection of standard input, output, and error streams. This allows users to direct input from files, output to files, or capture error messages in a file.
* Piping Between Two Processes:
The shell will support piping, allowing the output of one command to be used as the input to another command. This is useful for chaining commands together to perform complex operations.
## Help
![help](https://github.com/user-attachments/assets/d1f43c58-fed7-483b-88f3-13de3537ba3d)
