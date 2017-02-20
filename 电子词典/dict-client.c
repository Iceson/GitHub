#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sqlite3.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define  N  16
#define  R  1   //  user register
#define  L  2   //  user login
#define  Q  3   //  query word
#define  H  4   //  history record
#define  COL 50  //表示列数最大值

#define DATABASE "my.db" //创建的数据库文件

typedef struct 
{
	int type;          //类型  注册、登录、查询、查看历史记录
	char name[N];		//用户名字
	char data[256];   // password or word or remark
} MSG;
//注册
void do_register(int socketfd, MSG *msg)
{
	msg->type = R;//类型
	printf("input name : ");
	scanf("%s", msg->name);//名字
	printf("input password : ");
	scanf("%s", msg->data);//密码
	//发送刚才输入的信息
	send(socketfd, msg, sizeof(MSG), 0);
	//等待服务器传递信息，成功还是失败
	recv(socketfd, msg, sizeof(MSG), 0);
	printf("register : %s\n", msg->data);

	return;
}
//登陆操作
int do_login(int socketfd, MSG *msg)
{
	msg->type = L;
	printf("input name : ");
	scanf("%s", msg->name);
	printf("input password : ");
	scanf("%s", msg->data);
	
	send(socketfd, msg, sizeof(MSG), 0);
	//等待服务器端传输数据
	recv(socketfd, msg, sizeof(MSG), 0);
	if (strncmp(msg->data, "OK", 3) == 0) 
	{
		printf("login : OK\n");
		return 1;
	}
	//打印服务器端发送的信息
	printf("login : %s\n", msg->data);
	return 0;
}

void print(char *p)
{
	int count = 0, len;
	char *q;
	
	printf("   ");//打印三个空格
	while ( *p )
	{
		if (*p == ' ' && count == 0) p++;
		printf("%c", *p++);  //打印字符，输入的行缓冲区
		count++; //对读取的字符进行计数操作，最大数为50

		/* if a new word will begin */
		if (*(p-1) == ' ' && *p != ' ')
		{
			q = p;
			len = 0;//对单词的字符个数进行计数
			/* count the length of next word 查询单词的长度*/
			while (*q != ' ' && *q != '\0') 
			{
				len++;
				q++;
			}
			//判断读取的单词个数是否大于一行的剩余字符数
			//如果大于那就进行换行，否则继续写入
            if ((COL - count) < len) 
			{
				printf("\n   ");
				count = 0;
			}
		}
		if (count == 50)//判断是否到了50个字符，到一个新行中写入
		{
			count = 0;
			printf("\n   ");
		}
	}

	return;
}

void do_query(int socketfd, MSG *msg)
{
	msg->type = Q;
	while ( 1 )//循环查取单词
	{
		scanf("%s", msg->data);//输入要查询的单词
		if (strcmp(msg->data, "#") == 0) break;
		send(socketfd, msg, sizeof(MSG), 0);
		//等待服务器吧查询的单词的解释发送过来
		recv(socketfd, msg, sizeof(MSG), 0);
		
		print(msg->data);
		printf("\n");
	}

	return;
}
//历史记录
void do_history(int socketfd, MSG *msg)
{
	msg->type = H;
	send(socketfd, msg, sizeof(MSG), 0);
	while ( 1 )
	{
		recv(socketfd, msg, sizeof(MSG), 0);
		if (msg->data[0] == '\0') break;
		printf("%s\n", msg->data);
	}

	return;
}

int main(int argc, char *argv[])
{
	int socketfd ;
	struct sockaddr_in server_addr;
	MSG msg;
	if (argc < 3)
	{
		printf("Usage : %s <serv_ip> <serv_port>\n", argv[0]);
		exit(-1);
	}
	//套接口规则的设置
	if ((socketfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("fail to socket");
		exit(-1);
	}
	//信息结构体初始化
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = PF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	server_addr.sin_port = htons(atoi(argv[2]));
	if (connect(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("fail to connect");
		exit(-1);
	}
	int n;
	while ( 1 )
	{
		printf("************************************\n");
		printf("* 1: register   2: login   3: quit *\n");
		printf("************************************\n");
		printf("please choose : ");
		if (scanf("%d", &n) <= 0)
		{
			perror("scanf");
			exit(-1);
		}
		switch ( n )
		{
		case 1 :
			printf("\n");
			do_register(socketfd, &msg);//注册
			printf("\n");
			break;
		case 2 :
			printf("\n");
			//如果是1，表示成功调到next位置执行
			if (do_login(socketfd, &msg) == 1)//登陆
			{
				printf("\n");
				goto next;
			}
			printf("\n");
			break;
		case 3 :
			close(socketfd);    //退出
			exit(0);
		}
	}
next:
	while ( 1 )
	{
		printf("***********************************************\n");
		printf("* 1: query_word   2: history_record   3: quit *\n");
		printf("***********************************************\n");
		printf("please choose : ");
		
		if (scanf("%d", &n) < 0)
		{
			perror("scanf");
			exit(-1);
		}
		switch ( n )
		{
		case 1 :
			printf("\n");
			do_query(socketfd, &msg);//查询单词
			printf("\n");
			break;
		case 2 :
			printf("\n");
			do_history(socketfd, &msg);//查询历史记录
			printf("\n");
			break;
		case 3 :
			close(socketfd);
			exit(0);//结束进程函数
		}
	}

	return 0;
}
