#ifndef _THREAD_SAFE
	#define _THREAD_SAFE
#endif
#ifndef _POSIX_C_SOURCE
	#define _POSIX_C_SOURCE 200112L
#endif
#define MAX_BUF_SIZE 1024
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h> //sleep
// for socket
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "function.h"
//for threads
#include <pthread.h> 
// to be protected
struct utilities{
    struct sockaddr_in server_addr;
    int server_socket_fd; // server socket file descriptor.
    int SERVER_PORT;
    ssize_t byteRecv;
    ssize_t byteSent;
    size_t msgLen;
    socklen_t serv_size;
    char ip_address[MAX_BUF_SIZE];
    char receivedData[MAX_BUF_SIZE];
    char sendData[MAX_BUF_SIZE];
};

void * r_data(void *arg){
    struct utilities * u = (struct utilities *) arg;
    while(1){
        u->byteRecv = recvfrom(u->server_socket_fd, u->receivedData, MAX_BUF_SIZE, 0, (struct sockaddr *)&(u->server_addr), &(u->serv_size));
        printf("server port: %d ", u->SERVER_PORT);
        printf("server ip: %s\n", u->ip_address);
        printData(u->receivedData, u->byteRecv);
        if(strcmp(u->receivedData, "goodbye") == 0)
            break;
    }
    pthread_exit(NULL);
}

void * s_data(void *arg){
    struct utilities * u = (struct utilities *) arg;
    while(1){
        fflush(stdin);
        fgets(u->sendData, MAX_BUF_SIZE , stdin);
        fflush(stdin);
        u->msgLen = countStrLen(u->sendData);
        u->byteSent = sendto(u->server_socket_fd, u->sendData, u->msgLen, 0, (struct sockaddr *)&(u->server_addr), sizeof(u->server_addr));
        if(!strcmp(u->sendData, "exit\n")){
            break;
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]){
    struct utilities *u;
    u = malloc(sizeof(struct utilities));
    u->server_socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(u->server_socket_fd < 0){
        perror("socket error");
        exit(EXIT_FAILURE);
    }
    fflush(stdin);
    printf("Insert server port: \n");
    scanf("%d", &(u->SERVER_PORT));
    fflush(stdin);
    printf("Insert ip address: ");
    fflush(stdin);
    scanf("%s",u->ip_address);
    u->server_addr.sin_family = AF_INET;
    u->server_addr.sin_port = htons(u->SERVER_PORT);
    u->server_addr.sin_addr.s_addr = inet_addr(u->ip_address);
    u->serv_size = sizeof(u->server_addr);

    pthread_t    th[2]; 
	int  rc, i;
	void *ptr; 
    rc = pthread_create(&(th[0]), NULL, s_data, u); 
		if (rc) {
        perror("pthread_create failed");
        exit(EXIT_FAILURE);
    }
	rc = pthread_create(&(th[1]), NULL, r_data, u); 
	if (rc) {
        perror("pthread_create failed");
        exit(EXIT_FAILURE);
    }
	for(i=0;i<2;i++) {
		rc = pthread_join(th[i], &ptr ); 
		if (rc) {
            perror("pthread join failed");
            exit(EXIT_FAILURE);
        }
	}
    return 0;      
}