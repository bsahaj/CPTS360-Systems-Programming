/************** lab3base.c file **************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#define MAX 128
#define LIMIT 64
char *dirpath;
char gdir[MAX];    // gdir[ ] stores dir strings
char *dir[64];
int  ndir;

char *gpath;   // gpath[ ] stores token strings
char *name[64] = {0};
int  ntoken;

const char *home = "";
FILE *fd;
char *head[64] = {0};
char *tail[64] = {0};

int pipeArr[2];
int tokenizePath(char *pathname)
{
  ntoken = 0;
  gpath = strtok(pathname, " ");
  while (gpath != NULL)
  {
    name[ntoken] = (char*)malloc(sizeof(char)*strlen(gpath));
    name[ntoken] = strdup(gpath);
    printf("name[%d] = %s\n", ntoken, name[ntoken]);
    ntoken++;
    gpath= strtok(NULL, " ");
  }
  name[ntoken] = NULL;

}

int tokenizeDir(char *pathname)
{
  ndir = 0;
  dirpath = strtok(pathname, ":");
  while (dirpath != NULL)
  {
    dir[ndir] = (char*)malloc(sizeof(char)*strlen(dirpath));
    dir[ndir] = strdup(dirpath);
    printf("%s\t",dir[ndir]);
    ndir++;
    dirpath= strtok(NULL, ":");
  }
  dir[ndir] = NULL;
}

int ioHandler()
{
	
		 char * file;
		 file = name[2];
		 if(strcmp(name[1] , "<") == 0)
		 {
	             close(0);
		     fd = fopen(file, "r");
		     if (fd == NULL)
		     {
		             printf("cannot access file");
		     }
		     name[1] = NULL;
		     name[2] = NULL;
		     return 1;
		 }
		 if(strcmp(name[1], ">") == 0)
		 {
	             printf("greater thnan\n");
		     close(0);
		     fd = fopen(file, "w");
		     if (fd == NULL)
		     {
		             printf("cannot access file");
		     }
		     name[1] = NULL;
		     name[2] = NULL;
		     return 1;
		 }
		 if(strcmp(name[1], ">>") == 0)
		 {
	             close(0);
		      fd = fopen(file, "a");
		     if (fd == NULL)
		     {
		             printf("cannot access file");
		     }
		     name[1] = NULL;
		     name[2] = NULL;
		     return 1;
		 }
		 else
		 {
			 return -1;
		 }
}

int pipeHandler()
{
	int i=0, h_i, t_i;
	while(name[i])
	{
		if((strcmp(name[i], "|")) == 0)
		{
			tail[t_i] = (char*) malloc(sizeof(char)*strlen(name));
			strcpy(tail[++t_i], name[i]);
		}
		else
		{
		        head[h_i] = (char*) malloc(sizeof(char)*strlen(name));
			strcpy(head[h_i++], name[i]);
		}
	}
	head[h_i] = NULL;
	tail[t_i] = NULL;
	return;


}

//char* pipingFunc()
//{
//
//	return name; 
//}


int main(int argc, char *argv[], char *env[])
{
  int  i, r;
  int  pid, status;
  char *s, cmd[64], line[MAX];

  printf("************* Welcome to kcsh **************\n");
  i = 0;
  char *homeDir = getenv("HOME");
  printf("3.Home directory is %s\n", homeDir);
  while (env[i]){ 
    // Looking for PATH=
    if (strncmp(env[i], "PATH=", 5)==0){
      printf("1.show PATH:\n%s\n", env[i]);

      printf("2.decompose PATH into dir strings\n");
      strcpy(gdir, &env[i][5]);
      /*************** 1 ******************************
      Write YOUR code here to decompose PATH into dir strings in gdir[ ]
      pointed by dir[0], dir[1],..., dir[ndir-1]
      ndir = number of dir strings
      print dir strings
      ************************************************/
      tokenizeDir(gdir);
      break;
    }
    i++;
  }
  
  printf("\n4.*********** kcsh processing loop **********\n");
  
  while(1){
     for(int j=0;j<LIMIT; j++)
     {
             name[i] = NULL;
	     head[i] = NULL;
	     tail[i] = NULL;
     }
     printf("kcsh % : ");

     fgets(line, 128, stdin);
     line[strlen(line)-1] = 0;      // fgets() has \n at end

     if (line[0]==0)
       continue;
     printf("line = %s\n", line);   // print line to see what you got

     /***************** 2 **********************
      Write YOUR code here to decompose line into token strings in gpath[ ]
      pointed by name[0], name[1],..., name[ntoken-1]
      ntoken = number of token strings
      print the token strings
      ************************************************/
     tokenizePath(line);

     // 3. Handle name[0] == "cd" or "exit" case
     if(!strcmp(name[0],"exit"))
     {
	 printf("kcsh PROC %d exits\n", getppid());
	 exit(1);
     }
     else if(!strcmp(name[0], "cd"))
     {
	     if(!name[1])
	     {
		 chdir(homeDir);
		 printf("%d cd to %s\n", getppid(), homeDir);
	     }
	     else
	     {
		 chdir(name[1]);
		 printf("%d cd to %s\n", getppid(), homeDir);
	     }
     }

     // 4. name[0] is not cd or exit:
     else
     {
         pid = fork();   // fork a child sh

         if (pid){
		 pid = wait(&status);
		 printf("child sh %d died : exit status = %04x\n", pid, status);
                 continue;
          }
          else{
	         
		 printf("Parent kcsh PROC %d forks a child process %d \n", getppid(), getpid());
		 printf("parent sh %d waits\n", getppid());
                 printf("child sh %d begins\n", getpid());
		 printf("PROC %d  :  line = %s\n", getpid(), name[0]);
                 printf("PROC do_command: %s", name[0]);
		 printf("PROC %d tries to %s in each PATH dir:\n", getpid(),name[0]);
		 
	         int c = ioHandler();
		 for (i=0; i<ndir; i++){
			 strcpy(cmd, dir[i]); strcat(cmd, "/"); strcat(cmd, name[0]);
	                 printf("name[0]=%s i=%d cmd=%s\n", name[0], i, cmd);
	                 int pdes = pipe(pipeArr);
                         int peid = fork();
                          if(peid < 0)
	                  {
	                      printf("piping failed");
	                  }	 
	                  if(peid)
	                  {
	                 	 close(pipeArr[0]);
	                 	 dup(pipeArr[1]);
	                 	 close(pipeArr[1]);
	                  }
	                  else
	                  {
	                 	 close(pipeArr[1]);
	                 	 dup(pipeArr[0]);
	                 	 close(pipeArr[0]);
	                 	 pipeHandler();
	                 	 for(int a=0; a< LIMIT; a++)
	                 	 {
	                 		 strcpy(name[a], tail[a]);
	                 	 }	 
	                         
	                  }
			 r = execve(cmd, name, env);
	   
	         }
                 fclose(fd); 


	   printf("cmd %s not found, child sh exit\n", name[0]);
	   exit(123);   // die with value 123

           }  

     }
  }
}



/******************* 6 ***********************
 Handle pipe: check pipe symbol | in input line;
 if so, divide line into head, tail

 create PIPE, fork a child to share the pipe
 parent write to  pipe and exec head;
 child  read from pipe and exec tail
********************************************/


                         /*********************** 5 *********************
                         Write your code to do I/O redirection:
                         Example: check any (name[i] == ">"). 
                         If so, set name[i] = 0; 
                         redirecct stdout to name[i+1] 
                         ********************************************/ 
