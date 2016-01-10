README
======
     /*****************************************************************************************************************/
     #               NAME: SHARATH RUDRAPATNA VENKATESH                                                                #
     #         STUDENT ID: 3050566517                                                                                  #
     #  PROJECT OBJECTIVE: TO CREATE A CLIENT-SERVER PROGRAM TO SIMULATE A DISTRIBUTED HASH TABLE or DATABASE SYSTEM.  #
     #                                                                                                                 #
     #        OVERALL AIM: THE PROJECT AIMS TO FAMILIARIZE US WITH SOCKET PROGRAMMING                                  #
     /*****************************************************************************************************************/

     Contents of this file:
     ----------------------
       1. README.txt (this file)
       2. Makefile
       3. client1.c
       4. client2.c
       5. dhtserver1.c
       6. dhtserver2.c
       7. dhtserver3.c
       8. client1.txt
       9. client2.txt
      10. server1.txt
      11. server2.txt
      12. server3.txt
      13. client1, client2, server1, server2, server3.

     Description about the files:
     ----------------------------
       1. README.TXT: This particular file.
       2. Makefile: This containes the make rules for compiling the programs.
       3. client1.c: Contains the code for the client 1. 
                     1. Makes use of UDP Socket to Communicate with Server 1 to request the value for the corresponding Search term.
       4. client2.c: Contains the code for the client 2.
                     1. Makes use of UDP Socket to Communicate with Server 1 to request the value for the corresponding Search term.
       5. dhtserver1.c: Contains the code for Server 1(Designated server).
                     1. Makes use of UDP socket to receive request from client 1 and 2.
                     2. Makes use of TCP socket to send request to Sever 2 to obtain the value of the request from client 1 and/or client 2.
       6. dhtserver2.c: Contains the code for Server 2.
                     1. Makes use of TCP Socket to receive request from Server 1.
                     2. Makes use of TCP Socket to send request to Server 2 to obtain the value of the request from client 1 and/or client 2.
       7. dhtserver3.c: Contains the code for server 3.
                     1. Makes use of TCP Socket to receive request from Server 2.
       8. client1.txt: Contains a list of Search Terms along with their corresponding key values.
       9. client2.txt: Contains a list of Search Terms along with their corresponding key values.
      10. server1.txt: Contains a list of key and value pair.
      11. server2.txt: Contains a list of key and value pair.
      12. server3.txt: Contains a list of key and value pair.
      13. client1, client2: These are the output file names of the two clients.
      14. server1, server2, server3: These are the output file names of the three servers.

     Program Compilation:
     --------------------
       1. makefile can be used to compile all the programs. Type make to compile the makefile.
                        or
       2. The following steps must be followed for all servers and clients, if makefile is not used to compile.
          Type gcc <outputfilename> <filename.c> -lsocket -lnsl -lresolv 
          Ex: gcc server1 dhtserver1.c -lsocket -lnsl -lresolv
          The server program should compile without errors or warning.
          The client program should compile without any errors, but might throw warning which can be ignored and go ahead with execution.
      

     Program Execution: 
     ------------------
      Execution of program should take place: server3--->server2--->server1--->client(1/2)
     
      Open five terminals on nunki, on the first window type the command for running the server 3 --> ./server3
      On the second window type ./server2
      On the third window type ./server1
      On the fourth window type ./client1 
      After terminating client 1, you can either terminate all the servers and start again compiling 
      or you can terminate client 1 and start the same process for client 2.  

      User interaction will be there only on client 1 and client 2.
      After executing client 1, an on screen message for search term will appear. All the search terms must be in BOLD letters.

     Salient Features of client 1 and client 2 programs:
     ---------------------------------------------------
     1. Both clients initially load the contents of the file onto memory. 
     2. Both clients create a UDP socket on "nunki.usc.edu" and a random port number.
     3. Clients ask the user to enter a search term whos value is to be retrieved from the server(s).
     4. gethostbyname is used to resolve name to IP address of nunki.usc.edu.
     5. getsockbyname is used to get the portnumber and IP address assigned to the socket.

     Documentation:
     1. Code is thoroughly commented for easy understanding.

     References:
     1. This code contains some parts taken from BEEJ's guide. 
     2. Command to read file from the system using fopen() function from Linux man page and stackoverflow.
     3. some commands are referenced from rutgers.

    Salient Features of Server 1, Server 2 and Server 3:
    ----------------------------------------------------
  Server 1:

     1. Server 1 opens a UDP socket to receive request from clients on a Static port 21517 and IP address of "nunki.usc.edu".
     2. Checks the key in its memory, if a match is found then its corresponding value is sent to client which requested.
     3. If a match is not found then it opens up a TCP socket in order to communicate with server 2 to get the value.
     4. After receiving the value from server 2, it sends it back to client over UDP socket.
     5. Keeps its UDP and TCP sockets alive/open for further requests from client 1/2.

  Server 2:
   
     1. Server 2 opens a TCP socket to receive request from server 1 on a Static port 22517 and IP address of "nunki.usc.edu".
     2. Loads the key-value pair text file onto its memory.
     3. Upon receiving the request from server 1, it compares the key with the contents in its memory. If a match is found then it sends back the value to server 1.
     4. If match is not found then it opens a TCP socket with server 3 inorder to get the value of the key.
     5. After receiving the value form server 3, it sends it back to server 1.
     6. Both the TCP sockets are kept open/alive for further requests from client 1 via server 1.

 Server 3:
 
     1. Server 3 opens a TCP socket to receive request from server 2 on a Static port 23517 and IP address of "nunki.usc.edu".
     2. Value corresponding to the particular key is sent back to server 2.
     3. Keeps its TCP socket alive/open for future requests.

     Documentation:
     1. Code is thoroughly commented for easy understanding.

     References:
     1. Contains some parts taken from BEEJ's guide.
     2. Contains file open function fopen() from linux man page and stackoverflow.
     3. Some parts of the code are also referenced from rutgers.

     Assumptions:
     1. It is assumed that client 1 first contacts server 1 and after tearing down the connections between client 1 and server 1, 
       client 2 makes request to server 1. 
     2. It is assumed that the user will terminate the Servers by pressing cntrl c. and clients will terminate after each request.
     3. It is assumed that multiple requests are not made from the same client back to back. i.e there will be only one request from any client at any time.
        flow of request is: client1--->client2--->client1--->client2---
     

    

     