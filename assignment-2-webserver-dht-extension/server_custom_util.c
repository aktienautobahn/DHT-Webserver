#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "server_custom_util.h"
#include <sys/socket.h>
#include <arpa/inet.h>	
#include <openssl/sha.h>


	/*Create UDP port under given IP-Adress and port*/
	/*Returns FD of socket, -1 on error*/
int create_UDP_port(char* IP_adress, char* port){
	/*Convert port to int*/
	int port_int = atoi(port);

	/*Error handling for negative ports if neccesarry*/

	/*Create UDP socket*/
	int soc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	/*Bind the socket*/
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port_int);
	server_addr.sin_addr.s_addr = inet_addr(IP_adress);
	memset(server_addr.sin_zero, '\0', sizeof(server_addr.sin_zero));

	/*Error Handling if binding was unsuccessfull*/
	if(bind(soc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
		return -1;
	}

	return soc;
}


	/*Define the hash function
uint16_t hash(const char* str){
	uint8_t digest[SHA256_DIGEST_LENGTH];
	SHA256((uint8_t*)str, strlen(str), digest);
	return htons(*((uint16_t*)digest));
}
*/


	/*Takes the Hash-Room-Values of the server and its predicessor and decides, wheither the server
	responsible for the data
bool is_responsible(char* path, int own_value, int pred_value){
	Hash the path
	uint16_t hashValue = hash(path);

	Decide, wheither the server is responsible for the data
	if (hashValue > pred_value && hashValue <= own_value){
		return true;
	}
	return false;
}*/
