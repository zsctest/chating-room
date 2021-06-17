#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<ctype.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<pthread.h>
#define MAXLINE 1000           //消息的最大长度
#define SERV_PORT 5555        //服务器端口
#define MAXLINKING 5          //最大连接数

/**
 * 正在连接的用户数据结构 
 */
struct User{
	int socketid;
	char userIP[16];
	int used;	
}userTable[MAXLINKING];    //连接用户记录表

/**
 * 打印userTable信息
 * 仅用于测试
 */
void readUserTable(){
	for(int i=0;i<MAXLINKING;++i){
		printf("%s %d %d\n",userTable[i].userIP,userTable[i].socketid,userTable[i].used);
	}
}

/**
 * 广播信息
 */ 
void broadMsg(char *str,int length){
	int i;
	for(i=0;i<MAXLINKING;++i){
		if(userTable[i].used){
			send(userTable[i].socketid,str,length,0);
		}
	}	
}


/**
 * 注册连线用户
 */ 
void *regist(int socketid,const char *ip){
	int i = 0;
	for(i=0;i<MAXLINKING;++i){
		if(!userTable[i].used){
			userTable[i].socketid = socketid;
			//ip是指针，应该用memcpy函数进行复制
			memset(userTable[i].userIP,0,sizeof(userTable[i].userIP));
			memcpy(userTable[i].userIP,ip,strlen(ip));
			userTable[i].used = 1;
			return &userTable[i];
		}
	}
	printf("too much client!\n");
	return NULL;
}

/**
 * 删除掉线用户
 */ 
void unRegist(int socketid){
	int i;
	char Msg[30];
	for(i=0;i<MAXLINKING;++i){
		if(userTable[i].socketid == socketid && userTable[i].used ){
			sprintf(Msg,"%s leave!",userTable[i].userIP);
			broadMsg(Msg,sizeof(Msg));
			userTable[i].used=0;
			printf("%s\n",Msg);
			break;
		}
	}
	close(socketid);
}
			


void *pthread_dealWithMsg(void *arg){
	int recvLength;
	struct User *client = (struct User*)arg;
	char recvBuf[MAXLINE];
	char title[30];
	int i;
	memset(title,0,30);
	sprintf(title,"%s said:",client->userIP);
	while(1){
		recvLength = recv(client->socketid,recvBuf,999,0);
		if(recvLength <= 0){
			unRegist(client->socketid);
			break;
		}else{
			broadMsg(title,strlen(title));
			recvBuf[recvLength] = '\0';
			broadMsg(recvBuf,recvLength);
			printf("%s %s",title,recvBuf);
			
		}
	}	
}


int main(void){
	struct sockaddr_in servaddr,cliaddr;        //定义服务器与客户端地址结构体
	socklen_t cliaddr_len;                      //客户端地址长度
	int listenfd,connfd;
	char str[INET_ADDRSTRLEN];
	int i,ret;
	struct User *user = NULL;
	pthread_t tid;
	//创建服务器端套接字文件
	listenfd=socket(AF_INET,SOCK_STREAM,0);		
	//初始化服务器端口地址
	bzero(&servaddr,sizeof(servaddr));          //将服务器端口地址清0
	servaddr.sin_family = AF_INET;				
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);	
	servaddr.sin_port = htons(SERV_PORT);
	//将套接字文件与服务器端口服务器端口地址绑定
	ret = bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
	if(-1 == ret){
		printf("bind error!\n");
		return -1;
	}
	//监听
	ret = listen(listenfd,MAXLINKING);
	if(-1 == ret){
		printf("listen error!\n");
		return -1;
	}
	printf("Accepting connections---\n");
	//接收数据并处理请求
	while(1){
		cliaddr_len = sizeof(cliaddr);
		connfd = accept(listenfd,(struct sockaddr*)&cliaddr,&cliaddr_len);
		
		if(-1 != connfd){
			inet_ntop(AF_INET,&cliaddr.sin_addr,str,sizeof(str));
			user = regist(connfd,str);
			if(NULL==user){
				printf("regist error!\n");
				continue;
			}
			printf("received from %s at PORT %d,%d\n",
			inet_ntop(AF_INET,&cliaddr.sin_addr,str,sizeof(str)),
			ntohs(cliaddr.sin_port),connfd);
			pthread_create(&tid,NULL,pthread_dealWithMsg,(void*)user);
			pthread_detach(tid);
		}
	} 
	return 0;
} 
