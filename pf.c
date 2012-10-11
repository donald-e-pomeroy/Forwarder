#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 

struct queue * client_to_server_q;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

struct node{
	char * message;
	struct node * next;
};

struct queue{
	struct node * head;
	struct node * tail;
};

struct queue * init_queue(void)
{
	struct queue * my_queue= (struct queue*)malloc(sizeof(struct queue));
	
	if(my_queue == NULL){
		printf("bad malloc\n");
		return NULL;
	}

	my_queue->head = my_queue->tail = NULL;
	return my_queue;	
}

char * peek(struct queue * my_q){
	if(my_q == NULL)
	{
		return NULL;
	}

	if(my_q->head == NULL){
		return NULL;
	}

	return my_q->head->message;
	
} 


struct queue * add(char * message, struct queue * my_q){
		
	if(my_q == NULL)
	{	
		printf("null queue");
		return NULL;
	}	
	
	struct node * my_node = (struct node *)malloc(sizeof(struct node));
	my_node->message = (char *)malloc(sizeof(char));
	my_node->message = message;
	my_node->next = NULL;

	if((my_q->head == NULL)&&(my_q->tail==NULL)){
		my_q->head = my_q->tail = my_node;
	}else{
		my_q->tail->next = my_node;
		my_q->tail = my_node;
	}
	printf("return my queue");
	return my_q;	
}

struct queue * remove_from(struct queue * my_q){
	if(my_q == NULL){
		return NULL;
	}

	if((my_q->head == NULL)&&(my_q->tail==NULL)){
		return my_q;	
	}

	

	struct node * remove = (struct node *)malloc(sizeof(struct node));
	struct node * replace = (struct node *)malloc(sizeof(struct node));

	remove = my_q->head;
	replace = my_q->head->next;

	free(remove);
	my_q->head = replace;

	if(my_q->head == NULL){
		my_q->head == my_q->tail;
	}

	return my_q;
	
}



void * serverThreadFunc(void * arg){
	int listenfd = 0, connfd = 0;
	struct sockaddr_in serv_addr; 

	char sendBuff[1025];
	time_t ticks; 

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, '0', sizeof(serv_addr));
	memset(sendBuff, '0', sizeof(sendBuff)); 

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(5000); 

	bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

	listen(listenfd, 10); 

	while(1)
	{
		printf("sending\n");
        	connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
		while(connfd > -1){
        		//ticks = time(NULL);
			printf("peeking\n");
			//copy from the client to server q 
			if(peek(client_to_server_q)!=NULL){
				printf("writing");
				strcpy(sendBuff,peek(client_to_server_q));
				//locks
				//pthread_mutex_lock(&lock);
				client_to_server_q = remove_from(client_to_server_q);
				//pthread_mutex_unlock(&lock);			

	        		//snprintf(sendBuff, sizeof(sendBuff), "%.24s\r\n", ctime(&ticks));
				printf("writing\n");
        			int writeres =	write(connfd, sendBuff, strlen(sendBuff)); 
				if(writeres < 0){
					break;
				}
			}
			
		}
        
	}
     close(connfd);
}


void * clientThreadFunc(void * arg){
	int sockfd = 0, n = 0;
	char recvBuff[1024];
	struct sockaddr_in serv_addr; 

	char * ipaddr = (char*)malloc(sizeof(char));
	strcpy(ipaddr,arg);

	memset(recvBuff, '0',sizeof(recvBuff));
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Error : Could not create socket \n");
		return 1;
	} 

	memset(&serv_addr, '0', sizeof(serv_addr)); 

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(5000); 

	if(inet_pton(AF_INET, ipaddr, &serv_addr.sin_addr)<=0)
	{
        	printf("\n inet_pton error occured\n");
		return 1;
	} 

	if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
       		printf("\n Error : Connect Failed \n");
       		return 1;
	} 

	while(1){	
    		if( (n = read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0)
		{
			recvBuff[n] = 0;
			if(fputs(recvBuff, stdout) == EOF)
			{
				printf("\n Error : Fputs error\n");
			}
			//add to the client to server q	
			//pthread_mutex_lock(&lock);
			client_to_server_q = add(recvBuff,client_to_server_q);
			//pthread_mutex_unlock(&lock);
			
		} 
		printf("here\n");
		if(n < 0)
		{
			printf("\n Read error \n");
		} 
		
	}

    return 0;
}

int main(int argc, char * argv[]){
	pthread_t serverThread;
	pthread_t clientThread;
	
	client_to_server_q = init_queue();
	
	pthread_create(&serverThread,NULL,serverThreadFunc,NULL);
	pthread_create(&clientThread,NULL,clientThreadFunc,argv[1]);

	while(1){
		usleep(1);
	}

}
