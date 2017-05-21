#include <string.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <sys/select.h>  
#include <sys/time.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <pthread.h>  
/*
一个线程收、另一个线程发
*/
//接收线程：负责接收消息并显示  
void *recv_thread(void* arg)  
{  
    int udpfd = (int)arg;  
    struct sockaddr_in addr;  
    socklen_t addrlen = sizeof(addr);  
      
    bzero(&addr,sizeof(addr));  
    while(1)  
    {  
        char buf[200]  = "";  
        char ipbuf[16] = "";  
        recvfrom(udpfd, buf, sizeof(buf), 0, (struct sockaddr*)&addr, &addrlen);  
        printf("\r\033[31m[%s]:\033[32m%s\n",inet_ntop(AF_INET,&addr.sin_addr,ipbuf,sizeof(ipbuf)),buf);  
        write(1,"UdpQQ:",6);  
    }  
    return NULL;  
}  
  
int main(int argc,char *argv[])  
{  
    char buf[100] = "";  
    int  udpfd = 0;  
    pthread_t tid;  
    struct sockaddr_in addr;  
    struct sockaddr_in cliaddr;  
      
    //对套接字地址进行初始化  
    bzero(&addr,sizeof(addr));  
    addr.sin_family = AF_INET;  
    addr.sin_port   = htons(8000);  
    addr.sin_addr.s_addr = htonl(INADDR_ANY);  
  
    bzero(&cliaddr,sizeof(cliaddr));      
    cliaddr.sin_family = AF_INET;  
    cliaddr.sin_port   = htons(8000);  
  
    //创建套接口  
    if( (udpfd = socket(AF_INET,SOCK_DGRAM, 0)) < 0)  
    {  
        perror("socket error");  
        exit(-1);  
    }  
      
    //设置端口  
    if(bind(udpfd, (struct sockaddr*)&addr, sizeof(addr)) < 0)  
    {  
        perror("bind error");  
        close(udpfd);         
        exit(-1);  
    }  
      
    printf("input: \"sayto 192.168.221.X\" to sendmsg to somebody\n");  
    //创建接收线程  
    pthread_create(&tid, NULL, recv_thread, (void*)udpfd); //创建线程  
    printf("\033[32m"); //设置字体颜色  
    fflush(stdout);  
      
    while(1)  
    {     
        //主线程负责发送消息  
        write(1,"UdpQQ:",6);//1 表示标准输出  
        fgets(buf, sizeof(buf), stdin); //等待输入  
        buf[strlen(buf) - 1] = '\0';    //确保输入的最后一位是'\0'  
        if(strncmp(buf, "sayto", 5) == 0)  
        {  
            char ipbuf[INET_ADDRSTRLEN] = "";  
            inet_pton(AF_INET, buf+6, &cliaddr.sin_addr);//给addr套接字地址再赋值.  
            printf("\rconnect %s successful!\n",inet_ntop(AF_INET,&cliaddr.sin_addr,ipbuf,sizeof(ipbuf)));  
            continue;  
        }  
        else if(strncmp(buf, "exit",4) == 0)  
        {  
            close(udpfd);  
            exit(0);  
        }  
          
        sendto(udpfd, buf, strlen(buf),0,(struct sockaddr*)&cliaddr, sizeof(cliaddr));  
    }  
      
    return 0;  
}  
