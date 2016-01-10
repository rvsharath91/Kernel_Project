all:
	gcc dhtserver1.c -o server1 -lsocket -lnsl -lresolv
	gcc dhtserver2.c -o server2 -lsocket -lnsl -lresolv
	gcc dhtserver3.c -o server3 -lsocket -lnsl -lresolv
	gcc client1.c -o client1 -lsocket -lnsl -lresolv
	gcc client2.c -o client2 -lsocket -lnsl -lresolv

clean:
	rm -f dhtserver1 dhtserver2 dhtserver3 client1 client2
