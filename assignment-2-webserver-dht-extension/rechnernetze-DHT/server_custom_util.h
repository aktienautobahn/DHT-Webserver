#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/sha.h>


	/*Create UDP port under given IP-Adress and port*/
	/*Returns FD of socket, -1 on error*/
int create_UDP_port(char* IP_adress, char* port);


	/*Takes the Hash-Room-Values of the server and its predicessor and decides, wheither the server
	responsible for the data*/
/*bool is_responsible(char* path, int own_value, int pred_value);*/
