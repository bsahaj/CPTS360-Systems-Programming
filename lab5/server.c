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

#define MAX   256
#define BLK  1024

int server_sock, client_sock;
char *serverIP = "127.0.0.1";      // hardcoded server IP address
int serverPORT = 1234;             // hardcoded server port number

struct sockaddr_in saddr, caddr;   // socket addr structs

int init()
{
    printf("1. create a socket\n");
    server_sock = socket(AF_INET, SOCK_STREAM, 0); 
    if (server_sock < 0) { 
        printf("socket creation failed\n"); 
        exit(0); 
    }
    
    printf("2. fill in server IP and port number\n");
    bzero(&saddr, sizeof(saddr)); 
    saddr.sin_family = AF_INET; 
     saddr.sin_addr.s_addr = inet_addr(serverIP);
    saddr.sin_port = htons(serverPORT);
    
    printf("3. bind socket to server\n");
    if ((bind(server_sock, (struct sockaddr *)&saddr, sizeof(saddr))) != 0) { 
        printf("socket bind failed\n"); 
        exit(0); 
    }
    printf("4. server listen with queue size = 5\n");
    if ((listen(server_sock, 5)) != 0) { 
        printf("Listen failed\n"); 
        exit(0); 
    }
    printf("5. server at IP=%s port=%d\n", serverIP, serverPORT);
}
  
int main() 
{
    int n, length, created, fp, counter, file_sz;
    char line[MAX], buf[MAX]; 
    char command[MAX];
    char *path;
    init();  

    while(1){
       printf("server: chroot to /root\n");

       printf("server: try to accept a new connection\n");
       length = sizeof(caddr);
       client_sock = accept(server_sock, (struct sockaddr *)&caddr, &length);
       if (client_sock < 0){
          printf("server: accept error\n");
          exit(1);
       }
 
       printf("server: accepted a client connection from\n");
       printf("-----------------------------------------------\n");
       printf("    IP=%s  port=%d\n", "127.0.0.1", ntohs(caddr.sin_port));
       printf("-----------------------------------------------\n");

       // Processing loop
       while(1){
         printf("server ready for next request ....\n");
         n = read(client_sock, command, MAX);
         n = read(client_sock, line, MAX);
         path = "/";

         if(!strcmp(command, "mkdir"))
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
          else if(!strcmp(command, "cd"))
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
          else if(!strcmp(command, "get"))
          {
              fp = open(path, O_RDONLY);
              while(n = read(fp,buf, MAX))
              {
                  write(client_sock, buf, n);
              }
              close(fp);
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
                  n = read(client_sock,buf, MAX);
                  counter = counter + n;
                  write(fp, buf, n);
              }
              close(fp);
          }
          else if(!strcmp(command, "pwd"))
          {
              char *pathname = "/";
              getcwd(pathname, sizeof(pathname));
          }
          else
          {
              system(line);
          }


         if (n==0){
           printf("server: client died, server loops\n");
           close(client_sock);
           break;
         }
       }
    }
}


