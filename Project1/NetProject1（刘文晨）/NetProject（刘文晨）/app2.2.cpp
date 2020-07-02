#include<winsock2.h> 
#include<stdio.h>
#include<windows.h>
#include<string.h>
#include<time.h>
#include<unistd.h>
#pragma comment (lib, "ws2_32.lib")
#define MAX_BUFFER_SIZE 50000 //缓冲的最大大小
int main()
{
	printf("------------------应用层-----------------\n\n"); 
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2),&wsaData);
    int AppPort=14302,NetPort=14502;
    char TargetPort[6];//目的端口号
    char buf[50000];
    char buffer[50000];
    memset(TargetPort,0,sizeof(TargetPort));
    memset(buffer,0,sizeof(buffer));
    memset(buf,0,sizeof(buf));
    //创建sock
    SOCKET sock=socket(AF_INET,SOCK_DGRAM,0);
    //绑定应用层端口
	printf("本应用层的端口号：") ;
	printf("%d\n",AppPort); 
    sockaddr_in AppAddr;
    memset(&AppAddr,0,sizeof(AppAddr));
    AppAddr.sin_family=PF_INET;
    AppAddr.sin_addr.s_addr=inet_addr("127.0.0.1"); 
    AppAddr.sin_port=htons(AppPort);
    bind(sock,(SOCKADDR*)&AppAddr,sizeof(SOCKADDR)); 
    //网络层信息 
	sockaddr_in NetAddr;
    memset(&NetAddr,0,sizeof(NetAddr));
    int NetLen=sizeof(NetAddr);
    NetAddr.sin_family=PF_INET;
    NetAddr.sin_addr.s_addr=inet_addr("127.0.0.1");
	printf("本网络层的端口号：") ;
    printf("%d\n\n",NetPort);
    NetAddr.sin_port=htons(NetPort);
    while(1)
	{
        //定义当前状态
        printf("选择状态：\n1.发送信息\n2.接收信息\n\n");
        char IsSend[2];//状态信息标志
        memset(IsSend,0,sizeof(IsSend));
        gets(IsSend);
        //同时将状态信息发送到网络层
        sendto(sock,IsSend,strlen(IsSend),0,(struct sockaddr*)&NetAddr,sizeof(NetAddr));
        if(IsSend[0]=='1')
		{   
            printf("选择发送形式：\n1.广播信息 \n2.点对点发送\n");
            char IsPTP[2];//广播标志
            memset(IsPTP,0,sizeof(IsPTP));
            gets(IsPTP);
            if(IsPTP[0]=='1')
			{
                printf("\n\n------------------发送信息-------------------\n\n");
                memset(buffer,0,sizeof(buffer));
                memset(buf,0,sizeof(buf));
                printf("输入你要发送的的信息: \n");
                gets(buf); 
                char mybuf[9]="00000";
                strcat(buffer,mybuf);
                strcat(buffer,buf);
                printf("%s\n",buffer);
                sendto(sock,buffer,strlen(buffer),0,(struct sockaddr*)&NetAddr,sizeof(NetAddr));
                printf("发送成功！请按任意键继续......\n");
                getchar();
            }
            else
			{
                memset(TargetPort,0,sizeof(TargetPort));
                memset(buffer,0,sizeof(buffer));
                memset(buf,0,sizeof(buf));
                printf("\n\n------------------发送信息-------------------\n\n");
                printf("输入目的端口号：");//这里只需要输入目的端的物理层端口号即可
                gets(TargetPort);
                printf("输入你要发送的的信息: ");
                gets(buf);
                //合并信息
                strcat(buffer,TargetPort);
                strcat(buffer,buf);
                printf("%s\n",buffer);
                sendto(sock,buffer,strlen(buffer),0,(struct sockaddr*)&NetAddr,sizeof(NetAddr));
                printf("发送成功！请按任意键继续......\n");
                getchar();
                }    
        }
        else
		{ 
            memset(TargetPort,0,sizeof(TargetPort));
            memset(buffer,0,sizeof(buffer));
            memset(buf,0,sizeof(buf));
            printf("\n\n------------------接受信息--------------------\n\n");
            recvfrom(sock,buffer,50000,0,(SOCKADDR*)&NetAddr,&NetLen);
            for(int i=0;i<5;i++)
			{//提取源端口号
                TargetPort[i]=buffer[i];
            }
            printf("端口号%s发来了一条信息！\n",TargetPort);
            int i=0;
            for(int j=10;j<strlen(buffer);j++)//提取信息
			{
                buf[i++]=buffer[j];
            }
            printf("收到的消息为: %s\n", buf);
            printf("请按任意键继续。。。。。\n");
            getchar();
        }
    }
    closesocket(sock);
    WSACleanup();
    return 0;
}
