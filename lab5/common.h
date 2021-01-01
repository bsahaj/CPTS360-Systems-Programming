#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char *commandStr[15] = {"get", "put", "ls", "cd", "pwd", "mkdir", "rmdir" "rm","lcat", "lls", "lcd", "lpwd", "lmkdir", "lrmdir", "lrm"};
int check;

int commandExec(char *command, char *pathname);