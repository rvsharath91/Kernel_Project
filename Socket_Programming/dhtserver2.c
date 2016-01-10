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
#define server3PORT 23517    //Static TCP Port Address of server 3
#define myPORT 22517       //Static TCP port address of server 2


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
int sen;
int rvalc;
int mysock2;
int n;
char line[128];
char   ky_vp [24][100];
int y=0;
char f_n[]="NULL";
char *ky;
char *va;
char *search=" ";
int i=0;
int sock_id1, sock_idc;
struct sockaddr_in server2, server1, server3, s1;
int length,len,len2,l1;
char str[512]; 
char buf[512];
char * b_v;

//Function to open the file and read its contents
//fopen referenced from linux man page and strtok from stackoverflow
FILE *file = fopen ("server2.txt", "r");
if(file!=NULL)
{

while(fgets (line,128,file) !=NULL)
{
ky=strtok(line,search);
strcpy(ky_vp[i], ky);
i=i+1;
va=strtok(NULL,search);
strcpy(ky_vp[i], va);
i=i+1;
}
fclose(file);
}
n=i;

//create TCP Socket to Communicate with server 1

if ((sock_id1=socket(AF_INET, SOCK_STREAM,0))==-1)
{
perror("Error creating socket\n");
exit(1);
}
//Create TCP Socket to Communicate with Server 3
if((sock_idc=socket(AF_INET, SOCK_STREAM,0))==-1)
{
perror("error creating socket\n");
exit(1);
}
len2=sizeof(server3);
length=sizeof(server2);
len=sizeof(server1);
l1=sizeof(s1);

memset((char *) &server2, 0, sizeof(server2));
server2.sin_family= AF_INET;

//Making use of gethostbyname. Referenced from rutgers
hp=gethostbyname("nunki.usc.edu");
if(hp==NULL)
{
perror("gethostbyname failed");
exit(1);
}
memcpy(&(server2.sin_addr), hp->h_addr, hp->h_length);
server2.sin_port= htons(myPORT);
server3.sin_family=AF_INET;
memcpy(&(server3.sin_addr), hp->h_addr, hp->h_length);
server3.sin_port= ntohs(server3PORT);
s1.sin_family=AF_INET;
memcpy(&(s1.sin_addr), hp->h_addr, hp->h_length);


//Binding the socket
if(bind(sock_id1, (struct sockaddr *)&server2, length)==-1)
{
perror("Error binding");
exit(1);
}
printf("The Server 2 has TCP port number %d and the IP address %s\n",(int) htons(server2.sin_port),inet_ntoa(server2.sin_addr));

//Listening for possible connection request
if((listen(sock_id1,20))==-1)
{
perror("error in listening");
}
if((connect(sock_idc, (struct sockaddr *)&server3, len2))==-1)
{
perror("error in connecting");
exit(1);
}
while(1)
{
if((mysock2=accept(sock_id1, (struct sockaddr *)&server1, (socklen_t*)&len))==-1) //Receive the IP address, Port number of the incoming connection
{
perror("Error accepting");
exit(1);
}
else
{
memset(buf,0,sizeof(buf));
n1:if((rvalc=recv(mysock2, buf, sizeof(buf),0))==-1)    //receive the key and store it in buf(memory location)
{
perror("Error reading");
}
else
{ 
printf("The Server 2 has received a request with key %s from the Server 1 with port number %d and IP address %s\n",buf,(int) ntohs(server1.sin_port),inet_ntoa(server1.sin_addr));
}
}
b_v = trimwhitespace(buf);
for(i=0;i<n;i=i+2)
{
if(strcmp(b_v,ky_vp[i])==0)
{
y=i+1;
if(send(mysock2, ky_vp[y], strlen(ky_vp[y]), 0)==-1)    //Send the value corresponding to the request back to server 1
{
perror("error sending");
exit(1);
}
printf("The Server 2 sends the reply %s to the Server 1 with port number %d and IP address %s\n\n",ky_vp[y],ntohs(server1.sin_port),inet_ntoa(server1.sin_addr));
goto n1;
}
}
y=i;
printf("Ther Server 2 sends the request %s to the Server 3\n",buf);
if(send(sock_idc, buf, strlen(buf),0)==-1)     //Send the request to server 3 for the value of the requested key
{
perror("error sending");
exit(1);
}

//Making use of getsockname
getsockname(sock_idc, (struct sockaddr *)&s1, &l1);
printf("The TCP port number is %d and the IP address is %s\n",(int) htons(s1.sin_port),inet_ntoa(s1.sin_addr));
memset(str,0,sizeof(str));

if(rvalc=recv(sock_idc, str, sizeof(str),0)==-1) //receive the value from server 3 and store it in str.
{
perror("Error reading\n");
exit(1);
}

printf("The Server 2 received the value %s from the Server 3 with port number %d and IP address %s\n",str,(int) ntohs(server3.sin_port),inet_ntoa(server3.sin_addr));
printf("The Server 2 closed the TCP connection with the Server 3\n");

if(sen=send(mysock2, str, strlen(str),0)==-1)  //send the received value back to server 1
{
perror("error sending\n");
exit(1);
}

printf("The Server 2, sent reply %s to the Server 1 with port number %d and IP address %s\n\n",str,(int) ntohs(server1.sin_port),inet_ntoa(server1.sin_addr));
strcpy(ky_vp[y],buf);     //store the received value from server 3 in memory
y=y+1;
strcpy(ky_vp[y],str);
n=n+2;
memset(buf,0,sizeof(buf));
goto n1;
}
close(sock_id1);
close(sock_idc);
return 0;
}




