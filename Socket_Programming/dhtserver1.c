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
#define server2PORT 22517     //Static TCP Port Address of server 2
#define PORT1 21517           //Static UDP port address of server 1


//Making use of Trimewhitespace to cut out garbage values or white spaces. Referenced from Stackoverflow
char *trimwhitespace(char *str)
{
  char *end;

  // Used to Trim leading space
  while(isspace(*str)) str++;

  if(*str == 0)  
    return str;

  // To Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
}

int main()
{
struct hostent *hp;
int j;
int a=1;
int n;
char line[128];
char   key_valuepair [24][100];
int y=0;
char f_n[]="NULL";
char *key;
char *value;
char *search=" ";
int i=0;
int sock_id1, sock_id2;
struct sockaddr_in server, client, server2, s1;
int length,len,len1,l1;
char store[512]; 
char buffer[512];
char * buffer_value;

//Function to open the file and read its contents
//fopen referenced from linux man page and strtok from stackoverflow
FILE *file = fopen ("server1.txt", "r");
if(file!=NULL)
{

while(fgets (line,128,file) !=NULL)
{
key=strtok(line,search);
strcpy(key_valuepair[i], key);
i=i+1;
value=strtok(NULL,search);
strcpy(key_valuepair[i], value);
i=i+1;
}
fclose(file);
}
n=i;

//create socket//

if ((sock_id1=socket(AF_INET, SOCK_DGRAM,0))==-1)
{
perror("Error creating socket\n");
exit(1);
}
if((sock_id2=socket(AF_INET, SOCK_STREAM,0))==-1)
{
perror("error creating socket\n");
exit(1);
}
len1=sizeof(server2);
length=sizeof(server);
len=sizeof(client);
l1=sizeof(s1);

//Making use of memset to clear before using
memset((char *) &server, 0, sizeof(server));
server.sin_family= AF_INET;

//Making use of gethostbyname. Referenced from rutgers
hp=gethostbyname("nunki.usc.edu");
if(hp==NULL)
{
perror("gethostbyname failed");
exit(1);
}
memcpy(&(server.sin_addr), hp->h_addr, hp->h_length);
server.sin_port= htons(PORT1);
server2.sin_family=AF_INET;
memcpy(&(server2.sin_addr), hp->h_addr, hp->h_length);
server2.sin_port= ntohs(server2PORT);
s1.sin_family=AF_INET;
memcpy(&(s1.sin_addr), hp->h_addr, hp->h_length);

//Binding the socket 
if(bind(sock_id1, (struct sockaddr *)&server, length)==-1)
{
perror("Error binding");
exit(1);
}
printf("The server 1 has UDP port number %d and IP address %s\n",(int) htons(server.sin_port), inet_ntoa(server.sin_addr));
if((connect(sock_id2, (struct sockaddr *)&server2, len1))==-1)
{
perror("error in connecting");
exit(1);
}

while(1)
{
n1:if((recvfrom(sock_id1,buffer,512,0,(struct sockaddr *)&client, &len))==-1) //receive the request from client
{
perror("error receiving");
exit(1);
}
if(a % 2==0)  //Used this condition to print the client number (1/2).
{
j=2;
}
else
{
j=1;
}
printf("The server 1 has received a request with key %s from client %d with port number %d and  IP address %s\n",buffer,j,(int) ntohs(client.sin_port), inet_ntoa(client.sin_addr));
buffer_value = trimwhitespace(buffer);
for(i=0;i<n;i=i+2)
{
if(strcmp(buffer_value,key_valuepair[i])==0)
{
y=i+1;
sendto(sock_id1, key_valuepair[y], strlen(key_valuepair[y]), 0, (struct sockaddr *)&client, len);  //Sending the value to client 1 
printf("The Server 1 sends the reply %s to Client %d with port number %d and IP address %s\n\n",key_valuepair[y],j,(int) ntohs(client.sin_port),inet_ntoa(client.sin_addr));
a=a+1;
goto n1;
}
}
y=i;
printf("The server 1 sends the request %s to the server 2\n",buffer);
if(send(sock_id2, buffer, strlen(buffer),0)==-1)  //sending the key to server 2 to get its corresponding value.
{
perror("error sending");
exit(1);
}

//making use of getsockname

getsockname(sock_id2, (struct sockaddr *)&s1, &l1);
printf("The TCP port number is %d and the IP address is %s\n",(int) htons(s1.sin_port),inet_ntoa(s1.sin_addr));
memset(store,0,sizeof(store));
if(recv(sock_id2, store,sizeof(store), 0)==-1)
{
perror("error in receiving");
exit(1);
}
printf("The server 1 received the value %s from the Server 2 with port number %d and IP address %s\n",store,(int) ntohs(server2.sin_port),inet_ntoa(server2.sin_addr));
printf("The Server 1 closed the TCP connection with Server 2\n");

sendto(sock_id1, store, strlen(store), 0, (struct sockaddr *)&client, len);
printf("The Server 1, sent reply %s to client %d with port number %d and IP address %s\n\n",store,j,(int) ntohs(client.sin_port),inet_ntoa(client.sin_addr));
strcpy(key_valuepair[y],buffer);      //store the key and the received value from server 2 in memory for future use
y=y+1;
strcpy(key_valuepair[y],store);
n=n+2;
memset(buffer,0,sizeof(buffer));
a++;
}
close(sock_id1); 
close(sock_id2); 
return 0;
}


