client:
	gcc -o client client.c

server:
	gcc -o server server.c

forwarder:
	gcc -o forwarder pf.c -lpthread


