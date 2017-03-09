#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFF_SIZE 1024
#define NAME_SIZE 512

int main(int argc, char *argv[])
{
	int sockfd;
	struct sockaddr_in server_addr;
	struct hostent *host;
	int nbytes, portnumber;


	if(argc != 3)
	{
		fprintf(stderr, "Usage: %s hostname portnumber\a\n", argv[0]);
		exit(1);
	}

	if((host = gethostbyname(argv[1])) == NULL)
	{
		fprintf(stderr, "Gethostname error\n");
		exit(1);
	}

	if((portnumber = atoi(argv[2]))<0)
	{
		fprintf(stderr, "Usage:%s hostname portnumber\a\n", argv[0]);
		exit(1);
	}

	//客户程序开始建立sockfd描述符
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0))==-1)
	{
		fprintf(stderr, "Socket Erroe.%s\a\n", strerror(errno));
		exit(1);
	}

	//客户程序填充服务端的资料
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(portnumber);
	server_addr.sin_addr = *((struct in_addr *)host->h_addr);

	//客户程序发起连接请求
	if(connect(sockfd, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr)) == -1)
	{
		fprintf(stderr, "Connect Error:%s\a\n", strerror(errno));
		exit(1);
	}

	//输入文件名,并放到缓冲区buffer中等待发送
	char file_name[NAME_SIZE + 1];
	char file_name_save[NAME_SIZE + 1];
	bzero(file_name, NAME_SIZE + 1);
	printf("Please input file name on server:\t");
	scanf("%s", file_name);

	char buffer[BUFF_SIZE];
	bzero(buffer, BUFF_SIZE);
	strncpy(buffer, file_name, strlen(file_name)>BUFF_SIZE ? BUFF_SIZE : strlen(file_name));
	
	//向服务器发送buffer中的数据
	if(send(sockfd, buffer, BUFF_SIZE, 0) < 0)
	{
		perror("send file name failed:");
		exit(1);
	}

	//输入要保存的地址
	printf("please input file name and location to save:\t");
	scanf("%s", file_name_save);

#if 0
	FILE *fp = fopen(file_name_save, "w");
	if(fp == NULL)
	{
		printf("file:\t%s cannot open to write\n", file_name_save);
		exit(1);

	}

	int rn = 0;
	while(1)
	{
		if(rn = recv(sockfd, buffer, BUFF_SIZE, 0) < 0)
		{
			printf("cannot receive file!\n");
			exit(1);
		}
		buffer[rn] = '\0';
		if(rn != 0)
		{
			fwrite(buffer, sizeof(char), BUFF_SIZE, fp);
			bzero(buffer, sizeof(buffer));
		}
		else
		{
			printf("receive over.\n");
			break;
		}
	}
#endif


#if 1
	//打开文件，准备写入
	FILE *fp = fopen(file_name_save, "w");
	if(fp == NULL)
	{
		printf("file:\t%s cannot open to write\n", file_name_save);
		exit(1);
	}

	//从服务器接收数据到buffer中
	//每接受一段数据，便将其写入文件中，循环直到文件接收完并写完为止
	bzero(buffer, BUFF_SIZE);
	int length = 0;
	while((length = recv(sockfd, buffer, BUFF_SIZE, 0)) > 0)
	{
		if(fwrite(buffer, sizeof(char), length, fp) < length)
		{
			printf("Fiile:\t%s write failed\n", file_name);
			break;
		}
		bzero(buffer, BUFF_SIZE);
	}

	//接收成功后，关闭文件
	printf("receive file:\t%s from server IP successful!\n", file_name);

#endif
	fclose(fp);
	close(sockfd); //结束通讯
	exit(0);

}
