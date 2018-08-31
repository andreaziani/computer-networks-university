#ifndef _THREAD_SAFE
	#define _THREAD_SAFE
#endif
#ifndef _POSIX_C_SOURCE
	#define _POSIX_C_SOURCE 200112L
#endif
#include <stdio.h>
#include <string.h>
#include <unistd.h> //sleep
#include <errno.h> //error
#include <stdlib.h>
#include <sys/types.h>
#include <string.h> 
#include <sys/socket.h> // socket
#include <arpa/inet.h> // inet
#include <netinet/in.h>
#include <pthread.h> // threads
#include "function.h"
#include "DBGpthread.h"
#define MAX_BUF_SIZE 1024
struct utilities
{
    struct sockaddr_in server_addr; // struct that contains server's address.
    struct sockaddr_in client_addr; // client address
    struct sockaddr_in first_client;
    int server_socket_fdescriptor;
    size_t msgLen;
    ssize_t byteRecv; // byte received.
    ssize_t byteSent; // byte sended.
    socklen_t cli_size; //legth of client's socket.    
    char receivedData [MAX_BUF_SIZE];
    char sendData [MAX_BUF_SIZE];
    int sendGoodBye; // protected from mutex
    char goodbye[8];
    ssize_t goodbyeMessage;
};
// queue implementation
struct Node 
{
	char * data;
	struct Node* next;
};

// Two global variables to store address of front and rear nodes. 
struct Node* front = NULL;
struct Node* rear = NULL;

// To enqueue a node
void enqueue(char *x, size_t len) 
{
	struct Node* temp = (struct Node*)malloc(sizeof(struct Node));
    temp->data = malloc(sizeof(char[MAX_BUF_SIZE]));
	strncpy(temp->data, x, len); 
	temp->next = NULL;
	if(front == NULL && rear == NULL)
    {
		front = rear = temp;
		return;
	}
	rear->next = temp;
	rear = temp;
}

// To dequeue a node.
void dequeue() 
{
	struct Node* temp = front;
	if(front == NULL)
    {
		return;
	}
	if(front == rear) 
		front = rear = NULL;
	else 
		front = front->next;
	free(temp);
}

char * p_front() 
{
	if(front == NULL)
		return NULL;
	return front->data;
}

//thread variables.
pthread_mutex_t  mutex;
pthread_cond_t cond;

void * s_data(void *arg)
{
    struct utilities * u = (struct utilities *) arg;
    while(1)
    {
        fgets(u->sendData, MAX_BUF_SIZE , stdin);
        fflush(stdin);
        u->msgLen = countStrLen(u->sendData); 
        enqueue(u->sendData, u->msgLen);          
        if(strcmp(inet_ntoa(u->first_client.sin_addr),"0.0.0.0"))
        {
            DBGpthread_mutex_lock(&mutex, "s_data");
            while(p_front() != NULL)
            {
                u->byteSent = sendto(u->server_socket_fdescriptor, p_front(), countStrLen(p_front()), 0, (struct sockaddr *)&(u->first_client), sizeof(u->client_addr));
                dequeue();
            }
            DBGpthread_mutex_unlock(&mutex, "s_data");
        }
    }
    pthread_exit(NULL);
}

void * s_goodbye(void *arg)
{
    struct utilities * u = (struct utilities *) arg;
    while(1)
    {
        DBGpthread_mutex_lock(&mutex,"s_goodbye");
        while(u->sendGoodBye == 0)
        {
            DBGpthread_cond_wait(&cond, &mutex, "s_goodbye");
        }
        u->sendGoodBye = 0;
        u->byteSent = sendto(u->server_socket_fdescriptor, u->goodbye, u->goodbyeMessage, 0, (struct sockaddr *)&u->client_addr, sizeof(u->client_addr));
        if(u->byteSent != u->goodbyeMessage) 
        {
            perror("send goodbye");
            exit(EXIT_FAILURE);
        }
        DBGpthread_mutex_unlock(&mutex, "s_goodbye");
    }
    pthread_exit(NULL);
}
void * r_data(void *arg)
{
    struct utilities * u = (struct utilities *) arg;
    int count = 0;
    while(1)
    {
        if(count == 0)
            u->byteRecv = recvfrom(u->server_socket_fdescriptor, u->receivedData, MAX_BUF_SIZE, 0, (struct sockaddr *) &(u->first_client), &(u->cli_size));
        else 
            u->byteRecv = recvfrom(u->server_socket_fdescriptor, u->receivedData, MAX_BUF_SIZE, 0, (struct sockaddr *) &(u->client_addr), &(u->cli_size));
        if(u->byteRecv == -1)
        {
            perror("error in received from");
            exit(EXIT_FAILURE);
        }
        
        // check if message arrives from first client
        // if the messagge is from another client the server do nothing.
        if((!strcmp(inet_ntoa(u->client_addr.sin_addr), inet_ntoa(u->first_client.sin_addr)) && u->first_client.sin_port == u->client_addr.sin_port) || count == 0)
        {
            count = 1;
            if(strncmp(u->receivedData, "exit\n", u->byteRecv) == 0)
            {
                DBGpthread_mutex_lock(&mutex, "r_data");
                u->sendGoodBye = 1;
                DBGpthread_cond_signal(&cond,"r_data");
                DBGpthread_mutex_unlock(&mutex, "r_data");
            } else {
                char ip_client[INET_ADDRSTRLEN];
                char port_client[11];
                sprintf(port_client, "%d", u->client_addr.sin_port); // convert number in string.
                printf("client port: %s ", port_client); 
                inet_ntop(AF_INET, &(u->client_addr.sin_addr.s_addr), ip_client, INET_ADDRSTRLEN); // convert ip address in string.
                printf("client ip: %s \n", ip_client);
                printData(u->receivedData, u->byteRecv);
            }
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    int bindResult;
    int SERVER_PORT;
    char ip_address[33];
    struct utilities * u;
    u = malloc(sizeof(struct utilities));
    u->sendGoodBye = 0;
    strcpy((u->goodbye), "goodbye");
    u->goodbyeMessage = sizeof(u->goodbye);
    pthread_t    th[3]; // three threads
	int  rc, i;
	void *ptr;
    // socket creation
    u->server_socket_fdescriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(u->server_socket_fdescriptor < 0)
    {
        perror("socket creation error");
        exit(EXIT_FAILURE);
    }
    // user inseriment of ip and port 
    printf("Insert server port: \n");
    scanf("%d", &(SERVER_PORT));
    fflush(stdin);
    printf("Insert ip address: ");
    fflush(stdin);
    scanf("%s",ip_address);
    u->server_addr.sin_family = AF_INET; // fam address -> internet domain
    u->server_addr.sin_port = htons(SERVER_PORT);
    u->server_addr.sin_addr.s_addr = inet_addr(ip_address); //bind to a specific address
    bindResult = bind(u->server_socket_fdescriptor, (struct sockaddr*)&(u->server_addr), sizeof(u->server_addr));
    if(bindResult < 0){
        perror("error bind");
        exit(EXIT_FAILURE);
    }
    u->cli_size = sizeof(u->client_addr);
    
    //mutex and cond initialization.
    DBGpthread_cond_init(&cond, NULL, "main");
	DBGpthread_mutex_init(&mutex, NULL, "main"); 
    // thread creation
    rc = pthread_create(&(th[0]), NULL, s_data, u); 
    if(rc)
    {
        perror("pthread join failed");
        exit(EXIT_FAILURE);
    }
	rc = pthread_create(&(th[1]), NULL, r_data, u); 
	if(rc)
    {
        perror("pthread join failed");
        exit(EXIT_FAILURE);
    }
    rc = pthread_create(&(th[2]), NULL, s_goodbye, u); 
    if(rc)
    {
        perror("pthread join failed");
        exit(EXIT_FAILURE);
    }
    // thread join
	for(i = 0; i < 3; i++) 
    {
		rc = pthread_join(th[i], &ptr ); 
		if(rc)
        {
            perror("pthread join failed");
            exit(EXIT_FAILURE);
        }
	}
    free(u);
    // mutex and cond destroy
	DBGpthread_mutex_destroy(&mutex, "main"); 
    DBGpthread_cond_destroy(&cond, "main");
    return 0;
}