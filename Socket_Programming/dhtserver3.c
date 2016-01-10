#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#define PORT 23517        //Static TCP port number of server 3



//Making use of Trimewhitespace to cut out garbage values or white spaces. Referenced from Stackoverflow
char *trimwhitespace(char *str)
{
  char *end;

  // Used to Trim leading space
  while(isspace(*str)) str++;

  if(*str == 0)  
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
}
int main()
{

//variables 

struct hostent *hp;
char *host = "nunki.usc.edu";
int n2=0;
int rval;
char line[128];
char   key_val [16][100];
int y=0;
char f_n[]="NULL";
char *k;
char *v;
char *search=" ";
int i=0;
int sock_id;
int mysock;
struct sockaddr_in s3, s2;
int length,len;
char buff[512]="NULL";
char * buff_val;

//Function to open the file and read its contents
//fopen referenced from linux man page and strtok from stackoverflow
FILE *file = fopen ("server3.txt", "r");
if(file!=NULL)
{

while(fgets (line,128,file) !=NULL)
{
k=strtok(line,search);        //This separates the key in the given text file from its corresponding value
strcpy(key_val[i], k);
i=i+1;
v=strtok(NULL,search);        //This provides us the value from the text file
strcpy(key_val[i], v);
i=i+1;
}
fclose(file);
}
n2=i;

//Making use of gethostbyname. Referenced from Rutgers
if((hp=gethostbyname(host)) == NULL)
{
perror("gethostbyname failed()");
exit(1);
}

//create socket

sock_id=socket(AF_INET, SOCK_STREAM, 0);
if(sock_id==-1)
{
perror("Error Creating socket");
exit(1);
}
s3.sin_family= AF_INET;
memcpy(&(s3.sin_addr), hp->h_addr, hp->h_length);
memset(&(s3.sin_zero), '\0', 8);
s3.sin_port= htons(PORT);
len=sizeof(struct sockaddr_in);

//binding

if( bind(sock_id, (struct sockaddr *)&s3, sizeof(s3))==-1)
{
perror("Error binding socket");
exit(1);
}

printf("The Server 3 has TCP port number %d and IP address %s\n",(int) htons(s3.sin_port),inet_ntoa(s3.sin_addr));

//listening to possible incoming connections.

if((listen(sock_id,20))==-1)
{
perror("error in listening");
}
while(1)
{
if((mysock=accept(sock_id, (struct sockaddr *)&s2, (socklen_t*)&len))==-1) //get the IP address and port number of server 2
{
perror("Error accepting");
exit(1);
}
else
{
memset(buff,0,sizeof(buff));
n2:if((rval=recv(mysock, buff, sizeof(buff),0))==-1)  //receive the key from server 2
{
perror("Error reading");
}
else
{ 
printf("The Server 3 has received a request with key %s from the Server 2 with port number %d and IP address %s\n",buff,(int) ntohs(s2.sin_port),inet_ntoa(s2.sin_addr));
}
}
buff_val = trimwhitespace(buff);
for(i=0;i<n2;i=i+2)
{
if(strcmp(buff_val,key_val[i])==0)
{
y=i+1;
break;
}
}
if(send(mysock, key_val[y], strlen(key_val[y]), 0)==-1)  //Send the corresponding value back to server 2
{
perror("error sending");
}
printf("Server 3 sends the reply %s to the Server 2 with port number %d and IP address %s\n\n",key_val[y],(int) ntohs(s2.sin_port),inet_ntoa(s2.sin_addr));
goto n2;
}
close(sock_id);
return 0;
}









