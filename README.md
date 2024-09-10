# 🐚 Custom Shell README

## 📚 Table of Contents
- [🔍 Introduction](#-introduction)
- [📜 Overview](#-overview)
- [✨ Features](#-features)
  - [🚀 Command Execution](#1--command-execution)
  - [🔗 Piping Between Processes](#2--piping-between-processes)
  - [📥 Redirection](#3--redirection)
- [⚡How to Use Built-in Commands](#How-to-Use-Built-in-Commands)
- [⚙️ Installation and Compilation](#%EF%B8%8F-installation-and-compilation)

## 🔍 Introduction

A **shell** is a command-line interpreter that allows users to interact with the operating system by typing commands. Shells provide an interface between the user and the kernel, enabling tasks such as file management, program execution, and system control. Popular shells include `bash`, `zsh`, and `sh`, each with unique features and scripting capabilities. Shells are a fundamental part of Unix-like operating systems, allowing users to execute commands either interactively or through scripts.

## 📜 Overview
This custom shell program is designed to read user commands, interpret them, and execute them much like a typical shell environment. It supports basic commands, piping, and redirection, offering functionality similar to common Unix shells but with custom commands (`mypwd`, `myecho`, etc.). The shell also allows file management operations such as copying and moving files with built-in options like `-a` (append) and `-f` (force).

## ✨ Features

### 1. 🚀 Command Execution
The shell reads user input and tokenizes it into arguments to be executed either as built-in commands or external commands. Supported commands include:

- 🗂️ **mypwd** - Print the current working directory.
- 🔊 **myecho** - Echo the user input.
- ❌ **exit** - Exit the shell.
- 📋 **mycp** - Copy files with optional `-a` for appending.
- 📝 **mymv** - Move files with optional `-f` for force overwrite.
- 📂 **cd** - Change the current working directory.
- 🌍 **myenv** - Print environment variables.
- 🔍 **type** - Display the type of a command.
- 💾 **myfree** - Show RAM information.
- ⏲️ **myuptime** - Display system uptime.
- ❓ **help** - Show a help message.

### 2. 🔗 Piping Between Processes
The shell supports piping between two commands using the `|` symbol, where the output of the first command is passed as the input to the second command.

### 3. 📥 Redirection
Input, output, and error redirection are supported. For example, you can redirect command output to a file or read input from a file.

## ⚡ How to Use Built-in Commands
Here is a brief overview of how to use each built-in command:
### `mypwd`
Prints the current working directory.
**Usage:**
```bash
mypwd
```
**Output:**
```bash
/home/user/my_directory
```

### `myecho`
 Echoes the text you provide as input.
**Usage:**
```bash
myecho Hello, world!
```
**Output:**
```bash
Hello, world!
```
### `mycp`
Copy files. Use -a for appending the content to the destination file.
**Usage:**
```bash
mycp source.txt destination.txt
//To append:
mycp -a source.txt destination.txt
```
### `mymv`
Move files. Use -f to force overwrite.
**Usage:**
```bash
mymv source.txt destination.txt
//To force overwrite:
mymv -f source.txt destination.txt
```
### `type`
Show the type of a command (e.g., whether it's a built-in command or an external program).
**Usage:**
```bash
type mypwd
```
**Output:**
```bash
mypwd is a shell built-in command
```
## ⚙️ Installation and Compilation 

### Step 1: Clone the Repository
```bash
git clone https://github.com/your-username/your-repo.git
cd your-repo
```
### Step 2: Compile the code
```bash
gcc Shell.c command.c -o shell
```
### Step 3: Run the shell
```bash
./shell
```
