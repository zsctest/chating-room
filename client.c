#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#define MAXLINE 1000
#define SERV_PORT 5555
#define SERVERIP "1.15.231.208"


char buf[MAXLINE];
char content[MAXLINE];

void *pthread_sendMsg(void *arg)
{
	int socketClient = *(int *)arg;
	int sendLength;
	while (1)
	{
		if (fgets(content, 999, stdin))
		{
			sendLength = send(socketClient, content, strlen(content), 0);
			if (sendLength <= 0)
			{
				close(socketClient);
				break;
			}
		}
	}
}

int main(int argc, char *argv[])
{
	struct sockaddr_in servaddr;    
	int sockfd, n;
	pthread_t tid;


	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &servaddr.sin_addr);
	servaddr.sin_port = htons(SERV_PORT);

	connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	int result = pthread_create(&tid, NULL, pthread_sendMsg, (void *)&sockfd);
	if (result != 0)
	{
		fprintf(stderr, "pthread_create error:%s\n", strerror(result));
		exit(1);
	}
	while (1)
	{
		int recvLength = recv(sockfd, buf, MAXLINE, 0);
		if (recvLength <= 0)
		{
			close(sockfd);
			printf("Server has been closed!\n");
			return 0;
		}
		else
		{
			buf[recvLength] = '\0';
			printf("%s\n", buf);
		}
	}
	return 0;
}
