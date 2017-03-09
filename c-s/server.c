#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <sys/stat.h>

#define NAME_SIZE 512
#define BUFF_SIZE 1024

int main(int argc, char *argv[])
{
	int sockfd, new_fd;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	int sin_size, portnumber;
	struct stat st;

	if(argc != 2)
	{
		fprintf(stderr, "Usage:%s portnumber\a\n", argv[0]);
		exit(1);
	}

	if((portnumber = atoi(argv[1])) < 0)
	{
		fprintf(stderr, "Usage:%s portnumber\a\n", argv[0]);
		exit(1);
	}

	//服务器开始建立socket描述符
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		fprintf(stderr, "Socket error: %s\n\a", strerror(errno));
		exit(1);
	}

	//服务器端填充sockaddr结构
	bzero(&server_addr, sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(portnumber);

	//捆绑sockfd描述符
	if(bind(sockfd, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr)) == -1)
	{
		fprintf(stderr, "Bind error:%s\n\a", strerror(errno));
		exit(1);
	}

	//监听sockfd描述符
	if(listen(sockfd, 5) == -1)
	{
		fprintf(stderr, "Listen error:%s\n\a", strerror(errno));
		exit(1);
	}

	while(1)
	{
		//服务器阻塞,直至客户端建立连接
		sin_size = sizeof(struct sockaddr_in);
		if((new_fd = accept(sockfd, (struct sockaddr *)(&client_addr), &sin_size)) == -1)
		{
			fprintf(stderr, "Accept error:%s\n\a", strerror(errno));
			exit(1);
		}

		//recv函数接收数据到缓存区buffer中
		char buffer[BUFF_SIZE];
		bzero(buffer, BUFF_SIZE);
		if(recv(new_fd, buffer, BUFF_SIZE, 0) < 0)
		{
			perror("Server Recieve Data Failed:");
			break;
		}

		//然后从buffer(缓冲区)拷贝到file_name中
		char file_name[NAME_SIZE + 1];
		bzero(file_name, NAME_SIZE + 1);
		strncpy(file_name, buffer, strlen(buffer)>NAME_SIZE ? NAME_SIZE : strlen(buffer));
		printf("%s\n", file_name);

#if 0
		//打开文件并读取文件数据
		int fp = open(file_name, O_RDONLY);
		if(fp < 0)
		{
			printf("File:%s Not Found\n", file_name);
			exit(1);
		}

		stat(file_name, &st); //获取文件大小
		int len = st.st_size;

		if(sendfile(sockfd, fp, 0, len) < 0)
		{
			perror("send error");
			exit(1);
		}
		
		printf("file:%s transfer successful!\n", file_name);
		close(fp);

#endif

#if 1
		FILE *fp = fopen(file_name, "r");
		if(fp == NULL)
		{
			printf("file:%s not found\n", file_name);
		}
		else
		{
			bzero(buffer, BUFF_SIZE);
			int length = 0;
			//每读取一段数据，便将其发送给客户端，循环直到文件读完为止
			while((length = fread(buffer, sizeof(char), BUFF_SIZE, fp)) > 0)
			{
				if(send(new_fd, buffer, length, 0) < 0)
				{
					printf("Send file:%s failed.\n", file_name);
					break;
				}
				bzero(buffer, BUFF_SIZE);
			}

			fclose(fp);
			printf("file:%s transfer successful!\n", file_name);
		}

#endif
		close(new_fd); 	//关闭与客户端的连接
	}
	close(sockfd);
	exit(0);

}
