#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <arpa/inet.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <libgen.h>     // for dirname()/basename()
#include <time.h> 
#include "common.h" 

#define MAX 256
#define BLK 1024

struct sockaddr_in saddr; 
char *serverIP   = "127.0.0.1";
int   serverPORT = 1234;
int   sock;

int init()
{
    int n; 

    printf("1. create a socket\n");
    sock = socket(AF_INET, SOCK_STREAM, 0); 
    if (sock < 0) { 
        printf("socket creation failed\n"); 
        exit(0); 
    }
    
    printf("2. fill in server IP and port number\n");
    bzero(&saddr, sizeof(saddr)); 
    saddr.sin_family = AF_INET; 
    saddr.sin_addr.s_addr = inet_addr(serverIP); 
    saddr.sin_port = htons(serverPORT); 
  
    printf("3. connect to server\n");
    if (connect(sock, (struct sockaddr *)&saddr, sizeof(saddr)) != 0) { 
        printf("connection with the server failed...\n"); 
        exit(0); 
    }
    printf("4. connected to server OK\n");
    printf("**********Processing loop************\n");
}
  
int main(int argc, char *argv[], char *env[]) 
{ 
    int  n, count, fp, fp2, created, counter, file_sz;
    char line[MAX], ans[MAX], buf[MAX];
    char command[MAX], path[MAX];


    init();
  
    while (1){
      printf("\n****************** menu **********************\n");
      printf("* get  put  ls   cd   pwd  mkdir  rmdir  rm  *\n");
      printf("* lcat      lls  lcd  lpwd lmkdir lrmdir lrm *\n");
      printf("**********************************************\n");
      printf("input command : ");

      bzero(line, MAX);
      bzero(command, MAX);
      bzero(path, MAX);

      fgets(line, MAX, stdin);

      line[strlen(line)-1] = 0;        // kill <CR> at end
      if (line[0]==0)                  // exit if NULL line
         exit(0);

      sscanf(line, "%s %s", command, path);

      if(strcmp(line, "quit") == 0)
      {
          printf("\n");
          exit(EXIT_SUCCESS);
      }
      else
      {
          printf("%s\n", command);
          printf("%s\n", path);
          //write(sock, command, MAX);
          //write(sock, path, MAX);
          //write(sock, line, MAX);
          
          if(!strcmp(command, "lmkdir"))
          {
              created = mkdir(path, 0755);
              if(!created)
              {
                  printf("Directory created\n");
              }
              else
              {
                  printf("Failed to create\n");
              }
              system("dir");
          }
          else if(!strcmp(command, "lcd"))
          {
              created = chdir(path);
              if(!created)
              {
                  printf("Changed directory\n");
              }
              else
              {
                  printf("Failed to change\n");
              }
          }
          else if(!strcmp(command, "lcat"))
          {
              char linehere[MAX];
              char *commandhere = "cat";
              char *pathname = path;
              sprintf(linehere, "%s %s", commandhere, pathname);
              system(linehere);
          }
          else if(!strcmp(command, "lls"))
          {
              system("ls -l");
          }
          else if(!strcmp(command, "lpwd"))
          {
              system("pwd");
          }
          else if(!strcmp(command, "lrmdir"))
          {
              char linehere[MAX];
              char *commandhere = "rmdir";
              char *pathname = path;
              sprintf(linehere, "%s %s", commandhere, pathname);
              system(linehere);
          }
          else if(!strcmp(command, "lrm"))
          {
              char linehere[MAX];
              char *commandhere = "rm";
              char *pathname = path;
              sprintf(linehere, "%s %s", commandhere, pathname);
              system(linehere);
          }
          else if(!strcmp(command, "pwd"))
          {
              char *pathname = "/";
              printf("%s", pathname);
              getcwd(pathname, sizeof(pathname));
              write(sock, path, MAX);
          }
          else if(!strcmp(command, "get"))
          {
              char *pathtofile = "home/srihi/Documents/cpts360/lab5/server.c";
              fp = open(pathtofile, O_RDONLY);
              fp2 = open("server.c", O_CREAT | O_WRONLY| O_APPEND, 0644);
              int rft, wft;
              rft= read(fp,buf, strlen(buf));
              void *ptr = buf;
              while(rft>0)
              {
                  wft = write(fp2, buf, rft);
                  rft = rft - wft;
                  ptr = ptr + wft;
              }
              
              close(fp);
              close(fp2);
          }
          else if(!strcmp(command, "put"))
          {
              struct stat st_file;
              stat(path, &st_file);
              file_sz = st_file.st_size;
              printf("%d", file_sz);
              counter = 0; 
              fp = open(path, O_WRONLY | O_CREAT, 0644);
              while(counter < file_sz)
              {
                  n = read(sock,buf, MAX);
                  counter = counter + n;
                  write(fp, buf, n);
              }
              close(fp);
          }
          else if(!strcmp(command, "ls"))
          {
              char linehere[MAX];
              char *commandhere = "ls -l";
              char *pathname = "/";
              sprintf(linehere, "%s %s", commandhere, pathname);
              system(linehere);
          }
          else{ 
          write(sock, command, MAX);
          write(sock, path, MAX);
          write(sock, line, MAX);
          
          }
      }
    }
}

