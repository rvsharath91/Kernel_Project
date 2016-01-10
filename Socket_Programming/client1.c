#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#define PORT 21517   //static UDP port address of server 1

int main()
{
int sock_id,length,len,l1,n;
struct sockaddr_in server,client,cl;
struct hostent *hp;

char   b[512]="NULL";
int x=0;
int y;
int n1=0;
char k_v [24][100];
char k;
char a[124];
char f_n[]="NULL";
char *kc;
char *name;
char *search=" ";
int i=0;

//Function to open the file and read its contents
//fopen referenced from linux man page and strtok from stackoverflow
FILE *file = fopen ("client1.txt", "r");
if(file!=NULL)
{
char line[124];
while(fgets (line,124,file) !=NULL)
{
name=strtok(line,search);  //This separates the Search name in the given text file from its corresponding key
strcpy(k_v[i], name);
i=i+1;
kc=strtok(NULL,search);   //This provides us the key from the text file
strcpy(k_v[i], kc);
i=i+1;
}
fclose(file);
}
n1=i;

//Using gethostbyname to get the IP address of nunki.usc.edu. Referenced from rutgers
if((hp=gethostbyname("nunki.usc.edu")) == NULL)
{
perror("gethostbyname ()");
exit(1);
}

//creating a UDP socket
if((sock_id=socket(AF_INET,SOCK_DGRAM,0))==-1)
{
perror("Error in creating socket");
exit(1);
}
len=sizeof(server);
length=sizeof(client);
l1=sizeof(cl);
memset((char *)&client, 0, sizeof(client));
server.sin_family= AF_INET;

//making use of memcpy to copy IP address of nunki.usc.edu to the members server and client(cl)
//This is referenced from rutgers
memcpy(&(server.sin_addr), hp->h_addr, hp->h_length);
memcpy(&(cl.sin_addr), hp->h_addr, hp->h_length);
server.sin_port= htons(PORT);

p1:printf("Please enter your search (USC,UCLA etc):");
scanf("%s",&a);
for(i=0;i<n1;i++)
{
if(strcmp(a,k_v[i])==0)
{
printf("The client 1 has received a request with search word %s , which maps to %s\n",a,k_v[i+1]);
y=i+1;
x=1;
break;
}
}
if(x!=1)
{
printf("Enter the Search name again\n"); //Extra printing statement if the Search name is entered wrongly
goto p1;
}
if (sendto(sock_id, k_v[y], strlen(k_v[y]) ,0, (struct sockaddr *)&server, len)==-1)
{
perror("sending failed");
exit(1);
}

// making use of getsockname
getsockname(sock_id, (struct sockaddr *)&cl, &l1);
printf("The client 1 sends the request %s to the Server 1 with port number %d and IP address %s\n",k_v[y],(int) ntohs(server.sin_port),inet_ntoa(server.sin_addr));
printf("The Client1's port number is %d and the IP address is %s\n",(int) htons(cl.sin_port), inet_ntoa(cl.sin_addr));

if(recvfrom(sock_id,b,512,0,(struct sockaddr *)&client, &length)==-1)
{
perror("Error in receiving");
}
printf("The Client 1 received the value %s from the Server 1 with port number %d and IP address %s\n",b,(int) ntohs(server.sin_port), inet_ntoa(server.sin_addr));
printf("The Client1's port number is %d and IP address is %s\n\n",(int) htons(cl.sin_port), inet_ntoa(cl.sin_addr));
close(sock_id); //closing the UDP Socket
return 0;
}








