#include <stdio.h>   
#include <unistd.h>  
#include <stdlib.h>  
#include <errno.h>  
#include <string.h>  
#include <sys/socket.h>  
#include <sys/types.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <sys/select.h>  
 
#define SERV_PORT 8080  
#define LIST 20                //服务器最大接受连接  
#define MAX_FD 10              //FD_SET支持描述符数量  
  
  
int main(int argc, char *argv[])  
{  
    int sockfd;  
    int err;  
    int i;  
    int connfd;  
    int fd_all[MAX_FD]; //保存所有描述符，用于select调用后，判断哪个可读  
      
    //下面两个备份原因是select调用后，会发生变化，再次调用select前，需要重新赋值  
    fd_set fd_read;    //FD_SET数据备份  
    fd_set fd_select;  //用于select  
  
    struct timeval timeout;         //超时时间备份  
    struct timeval timeout_select;  //用于select  
      
    struct sockaddr_in serv_addr;   //服务器地址  
    struct sockaddr_in cli_addr;    //客户端地址  
    socklen_t serv_len;  
    socklen_t cli_len;  
      
    //超时时间设置  
    timeout.tv_sec = 10;  
    timeout.tv_usec = 0;  
      
    //创建TCP套接字  
    sockfd = socket(AF_INET, SOCK_STREAM, 0);  
    if(sockfd < 0)  
    {  
        perror("fail to socket");  
        exit(1);  
    }  
      
    // 配置本地地址  
    memset(&serv_addr, 0, sizeof(serv_addr));  
    serv_addr.sin_family = AF_INET;         // ipv4  
    serv_addr.sin_port = htons(SERV_PORT);  // 端口， 8080  
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // ip  
  
    serv_len = sizeof(serv_addr);  
      
    // 绑定  
    err = bind(sockfd, (struct sockaddr *)&serv_addr, serv_len);  
    if(err < 0)  
    {  
        perror("fail to bind");  
        exit(1);  
    }  
  
    // 监听  
    err = listen(sockfd, LIST);  
    if(err < 0)  
    {  
        perror("fail to listen");  
        exit(1);  
    }  
      
    //初始化fd_all数组  
    memset(fd_all, -1, sizeof(fd_all));  
  
    fd_all[0] = sockfd;   //第一个为监听套接字  
      
    FD_ZERO(&fd_read);  // 清空  
    FD_SET(sockfd, &fd_read);  //将监听套接字加入fd_read  
  
    int maxfd = fd_all[0];  //监听的最大套接字  
      
    while(1){  
      
        // 每次都需要重新赋值，fd_select，timeout_select每次都会变  
        fd_select = fd_read;  
        timeout_select = timeout;  
          
        // 检测监听套接字是否可读，没有可读，此函数会阻塞  
        // 只要有客户连接，或断开连接，select()都会往下执行  
        err = select(maxfd+1, &fd_select, NULL, NULL, NULL);  
        //err = select(maxfd+1, &fd_select, NULL, NULL, (struct timeval *)&timeout_select);  
        if(err < 0)  
        {  
                perror("fail to select");  
                exit(1);  
        }  
  
        if(err == 0){  
            printf("timeout\n");  
        }  
          
        // 检测监听套接字是否可读  
        if( FD_ISSET(sockfd, &fd_select) ){//可读，证明有新客户端连接服务器  
              
            cli_len = sizeof(cli_addr);  
              
            // 取出已经完成的连接  
            connfd = accept(sockfd, (struct sockaddr *)&cli_addr, &cli_len);  
            if(connfd < 0)  
            {  
                perror("fail to accept");  
                exit(1);  
            }  
              
            // 打印客户端的 ip 和端口  
            char cli_ip[INET_ADDRSTRLEN] = {0};  
            inet_ntop(AF_INET, &cli_addr.sin_addr, cli_ip, INET_ADDRSTRLEN);  
            printf("----------------------------------------------\n");  
            printf("client ip=%s,port=%d\n", cli_ip,ntohs(cli_addr.sin_port));  
              
            // 将新连接套接字加入 fd_all 及 fd_read  
            for(i=0; i < MAX_FD; i++){  
                if(fd_all[i] != -1){  
                    continue;  
                }else{  
                    fd_all[i] = connfd;  
                    printf("client fd_all[%d] join\n", i);  
                    break;  
                }  
            }  
              
            FD_SET(connfd, &fd_read);  
              
            if(maxfd < connfd)  
            {  
                maxfd = connfd;  //更新maxfd  
            }  
          
        }  
          
        //从1开始查看连接套接字是否可读，因为上面已经处理过0（sockfd）  
        for(i=1; i < maxfd; i++){  
            if(FD_ISSET(fd_all[i], &fd_select)){  
                printf("fd_all[%d] is ok\n", i);  
                  
                char buf[1024]={0};  //读写缓冲区  
                int num = read(fd_all[i], buf, 1024);  
                if(num > 0){  
  
                    //收到 客户端数据并打印  
                    printf("receive buf from client fd_all[%d] is: %s\n", i, buf);  
                      
                    //回复客户端  
                    num = write(fd_all[i], buf, num);  
                    if(num < 0){  
                        perror("fail to write ");  
                        exit(1);  
                    }else{  
                        //printf("send reply\n");  
                    }  
                      
                }  
                else if(0 == num){ // 客户端断开时  
                    //客户端退出，关闭套接字，并从监听集合清除  
                    printf("client:fd_all[%d] exit\n", i);  
                    FD_CLR(fd_all[i], &fd_read);  
                    close(fd_all[i]);  
                    fd_all[i] = -1;  
                      
                    continue;  
                }  
                  
            }else {  
                //printf("no data\n");                    
            }  
        }  
    }  
      
    return 0;  
}  
