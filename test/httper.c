#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
	if(argc!=2){
		return -1;
	}
	const char *ip=argv[1];
	struct sockaddr_in addr;
	addr.sin_family=AF_INET;
	inet_aton(ip,&(addr.sin_addr));
	addr.sin_port=htons(80);

	int sockfd=socket(AF_INET,SOCK_STREAM,0);
	connect(sockfd,(struct sockaddr*)&addr,sizeof(addr));

	const char *request="GET / HTTP/1.1\r\n\r\n";

	write(sockfd,request,strlen(request));

	char buff[1024];
	int n=read(sockfd,buff,1024);

	buff[n]='\0';
	printf("%s",buff);

	close(sockfd);

	return 0;
}
