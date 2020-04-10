#include"epoll.h"
//#include"deamon.h"
int main()
{
	//init_deamon();
	MYSQL conn;
	my_connect(&conn);

	int sockfd = sock_inet();//sockfd=socket(AF_INET,SOCK_STREAM,0) 
	struct sockaddr_in seraddr = addr_inet();//family, port, ip
	sockfd = sock_listen(sockfd,seraddr);//判断绑定是否成功

	Info *head =NULL,*temp =NULL;  
	Pack rpa;    //读包
	int port;
	char ip[64];

//	int ep_count;
//	int clifd,fd;
	struct sockaddr_in cliaddr;
	socklen_t  addrlen =sizeof(cliaddr);

	int epfd = epoll_create(512);
	struct epoll_event evs[10];
	epoll_inet(epfd,sockfd);

	pthread_mutex_init(&mutex,NULL);//初始化线程互斥锁

	while(1)
	{
		int ep_count = epoll_wait(epfd,evs,10,-1);
		for(int i=0;i<ep_count;i++)
		{
			int fd =evs[i].data.fd;
			if(fd == sockfd)
			{
				int clifd = accept(sockfd,(struct sockaddr *)&cliaddr,&addrlen);//接收客户端连接
				port = ntohs(cliaddr.sin_port);
				inet_ntop(AF_INET,&cliaddr.sin_addr.s_addr,ip,sizeof(cliaddr));

				char buff[128];
				memset(buff,0x00,sizeof(buff));
				sprintf(buff,"\tclifd %d [%s | %d]链接\t",clifd,ip,port);
				printf("clifd %d [%s | %d]链接!\n",clifd,ip,port);

				write_file(buff);//写日志文件

				add_list(&head,clifd,ip,port);//添加到链表

				epoll_inet(epfd,clifd);//监听客户文件描述符
			}
			else
			{

				temp = head;
				while(temp !=NULL)
				{
					if(temp->clifd == fd)
					{
						int ret =read(temp->clifd,&rpa,sizeof(Pack));
						//printf("ret =%d,rpa.type=%d\n",ret,rpa.type);
						if(ret <0)
						{
							read_perror("read");
						}
						else if(ret ==0)
						{
							read_close(temp,head);
						}
						else
						{
							Arg arg;
							arg.temp = temp;
							arg.head = head;

							pthread_t pid;
							ret = pthread_create(&pid,NULL,(void *)heart,(void *)&arg);
							my_perror(ret,"pthread");

							//printf("rpa.type =%d\n",rpa.type);
							read_choose(temp,head,&conn,epfd,rpa.type);//选择注册，登录，读信息...
						}
					}

					temp =temp->next;
				}
			}
		}
	}

	return 0;
}



