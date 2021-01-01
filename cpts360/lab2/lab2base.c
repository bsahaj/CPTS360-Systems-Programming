#include <stdio.h>             // for I/O
#include <stdlib.h>            // for lib functions
#include <libgen.h>            // for dirname()/basename()
#include <string.h>

typedef struct node{
          char  name[64];       // node's name string
          char  type;
   struct node *child, *sibling, *parent;
}NODE;


NODE *root, *cwd, *start;
char command[16], pathname[64];

char *gpath;               // global gpath[] to hold token strings
char *name[64] = {'\0'};               // token string pointers
int  n=0;                        // number of token strings

char dname[64], bname[64];     // dirname, basename of pathname

//               0       1      2    
char *cmd[] = {"mkdir", "ls", "quit", "pwd", "cd",  0};

int findCmd(char *command)
{
   int i = 0;
   while(cmd[i]){
     if (strcmp(command, cmd[i])==0)
         return i;
     i++;
   }
   return -1;
}

NODE *search_child(NODE *parent, char *name)
{
  NODE *p;
  printf("search for %s in parent DIR\n", name);
  p = parent->child;
  if (p==0)
    return 0;
  while(p){
    if (strcmp(p->name, name)==0)
      return p;
    p = p->sibling;
  }
  return 0;
}

int insert_child(NODE *parent, NODE *q)
{
  NODE *p;
  printf("insert NODE %s into parent child list\n", q->name);
  p = parent->child;
  if (p==0)
    parent->child = q;
  else{
    while(p->sibling)
      p = p->sibling;
    p->sibling = q;
  }
  q->parent = parent;
  q->child = 0;
  q->sibling = 0;
}

void tokenize(char *pathname)
{
   int i = 0;
  n = 0;
   //Calculating number of tokens in string
   while(pathname[i] != '\0')
   {
     if(pathname[i] == '/')
     {
       n++;
     }
     i++;
   }

   printf("number of token strings: %d\n", n+1 );

   int j=0;
   gpath = strtok(pathname, "/");
   while (gpath != NULL)
   {
      name[j] = strdup(gpath);
      printf("name[%d] = %s\n", j, name[j]);
      j++;
       gpath= strtok(NULL, "/");
   }
}

int mkdir_single(char *name)
{
  NODE *p, *q, node;
  printf("mkdir: name=%s\n", name);

  if (name[0]=='/')
    start = root;
  else
    start = cwd;

  printf("check whether %s already exists\n", name);
  p = search_child(start, name);
  if (p){
    printf("name %s already exists, mkdir FAILED\n", name);
    return -1;
  }
  printf("--------------------------------------\n");
  printf("ready to mkdir %s\n", name);
  q = (NODE *)malloc(sizeof(NODE));
  q->type = 'D';
  strcpy(q->name, name);
  insert_child(start, q);
  printf("mkdir %s OK\n", name);
  printf("--------------------------------------\n");
    
  return 1;
}


NODE *path2node(char *pathname)
{
   // return pointer to the node of pathname, or NULL if invalid
   if (pathname[0] == '/')
    {  start = root; }
   else
    {  start = cwd;}

   tokenize(pathname);
   printf("n value is %d\n", n+1);
   int num = n+1;

   if (num == 1)
   {
     mkdir_single(name[0]);
     return 0;
   }
   else if (n == 0)
   {
     printf("Invalid pathname");
   }
   else
   {
  for(int k=0; k<num; k++)
     {
       int o = mkdir_single(name[k]);
       if(o == 1 || o==-1)
       {
         cd(name[k]);
       }
     }
   }
   for(int i=0; i<num; i++)
   {
     cd("..");
   }
   return 1;

} 


/***************************************************************
 This mkdir(char *name) makes a DIR in the current directory
 You MUST improve it to mkdir(char *pathname) for ANY pathname
****************************************************************/

int mkdir(char *pathname)
{
  NODE *p, *q, node;
  printf("mkdir: name=%s\n", pathname);

  //write YOUR code to not allow mkdir of /, ., ./, .., or ../
  if(strcmp(pathname, "") == 0)
  {
    printf("err: Pathname cannot be empty\n");
    return 0;
  }
  else
  {
    path2node(pathname);
  }
    
  return 1;
}

// This ls() list CWD. You MUST improve it to ls(char *pathname)
int ls()
{
  NODE *p = cwd->child;
  printf("cwd contents = ");
  while (p){
    printf("[%c %s] ", p->type, p->name);
    p = p->sibling;
  }
  printf("\n");
}

int quit()
{
  printf("Program exit\n");
  exit(0);
  
  // improve quit() to SAVE the current tree as a Linux file
  // for reload the file to reconstruct the original tree
}

int initialize()  // create / node, set root and cwd pointers
{
    root = (NODE *)malloc(sizeof(NODE));
    strcpy(root->name, "/");
    root->parent = root;
    root->sibling = 0;
    root->child = 0;
    root->type = 'D';
    cwd = root;
    printf("Root initialized OK\n");
}

void pwd(NODE *node)
{
    if (pathname[0] == '/')
    {
      printf("%s ", node->name);
      pwd(node->parent);
    }
    else
    {
      printf("%s ", node->name);
    }
}

NODE *cd_tokenizer(char *pathname, NODE *parent, NODE *child)
{
  char *path_token = strtok(pathname, "/");
  while(path_token != NULL)
  {
      if(strcmp(path_token,"..") == 0){
        parent = parent->parent;
        break;
      }
      if(child == NULL)
      {
        printf("err: Invalid pathname\n");
        return 0;
      }
      if(strcmp(child->name, path_token) == 0 && child->type == 'D')
      {
        parent = child;
        break;
      }
      child = child->sibling;
      child = parent->child;
      path_token = strtok(NULL, "/");
  }
  return parent;

}

int cd(char *pathname)
{
  NODE *parent, *child;
  if(pathname[0] == '/' || pathname[0] == '\0') 
  {
    parent = root;
  }
  else 
  {
    parent = cwd;
    child = parent->child;
    
  }
  parent = cd_tokenizer(pathname, parent, child);
  cwd = parent;
   
}

int main()
{
  int index;
  char line[128];
  
  initialize();

  printf("NOTE: commands = [mkdir|ls|quit]\n");

  while(1){
      printf("Enter command line : ");
      fgets(line, 128, stdin);
      line[strlen(line)-1] = 0;

      sscanf(line, "%s %s", command, pathname);
      printf("command=%s pathname=%s\n", command, pathname);
      
      if (command[0]==0) 
         continue;

      index = findCmd(command);

      switch (index){
        case 0: mkdir(pathname); break;
        case 1: ls();            break;
        case 2: quit();          break;
        case 3: pwd(cwd);        break;
        case 4: cd(pathname);    break;
      }
  }
}


