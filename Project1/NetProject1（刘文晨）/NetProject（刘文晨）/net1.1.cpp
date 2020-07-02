#include<winsock2.h> 
#include<stdio.h>
#include<windows.h>
#include<string.h>
#include<time.h>
#include<unistd.h>
#pragma comment(lib, "ws2_32.lib")
using namespace std;
#define rep(i,x,y) for(int i=x;i<=y;++i) 
typedef unsigned int uint;
#define MAX_BUFFER_SIZE 50000
char head[10]="01111110";
char end[10]="01111110";
char data[10];
char check[10];
char Frame[MAX_BUFFER_SIZE];//帧 
char RecFrame[MAX_BUFFER_SIZE];//接收帧 
char sendbuffer[MAX_BUFFER_SIZE];
char checkbuffer[MAX_BUFFER_SIZE];
char buffer[MAX_BUFFER_SIZE];  //缓冲区
char buf[MAX_BUFFER_SIZE];//缓冲区
char SourcePort[6];//源端口号
int len;
char mmmmmmm[MAX_BUFFER_SIZE]="111111111111111111111111111111";
char Broadcast[MAX_BUFFER_SIZE]="000000000000000000000000000000";
uint Num1[MAX_BUFFER_SIZE];
//编码、解码模块
namespace CODE {//UTF-8编码 
	int code(uint *num, int n, char *s)
	{
		if (n > MAX_BUFFER_SIZE) return -1;
		int pos = 0;
		rep(i, 1, n)
		{
			uint val = num[i];
			uint tmp = 1u << 7;
			rep(j, 1, 8)
			{
				s[pos++] = (tmp&val) ? '1' : '0';
				tmp >>= 1;
			}
		}
		s[pos + 1] = 0;
		return 0;
	}

	int decode(uint *num, char *s, int l)
	{
		int top = 0;
		for (int i = 0; i < l; i += 8)
		{
			++top; num[top] = 0;
			rep(j, 0, 7) {
				num[top] = (num[top] << 1) + (s[i + j] - '0');
			}
		}
		return top;
	}
}

//检查是否存在连续5个1，并返回出现的位置
int FindTarget(char *str){
	int i,j,cunt=0,target=0;
	rep(i,0,strlen(str)){   
		if(str[i]=='1'){
			rep(j,i,7){
				if(str[j]=='1'){
					cunt++;
				}	
				else{
					cunt=0;
					break;
				}			
				if(cunt>=5){
					target=j;
					break;
				} 
			}
		}
		if(target){
			break;//找到target便退出遍历 
		}			
	} 		
	return target;
}

//寻找帧
int FindFrame(char *str)
{
	int head=0;//找帧头 
	while(str[head]=='1')
	{
		head++;
	}
	while(str[head++]=='0')
	{
		int one=0;
		while(str[head]=='1')
		{
			one++;
			head++;
		} 
		if(one==6)
		{
			head++;
			break;
		}
	}
	int beginnum=head-8; 
	int tail=head;//找帧尾 
	while(str[tail]=='1')
	{
		tail++;
	} 
	while(str[tail++]=='0')
	{
		int one=0;
		while(str[tail]=='1')
		{
			one++;
			tail++;
		} 
		if(one==6)
		{
			tail-=8;
			break;
		}
	}
	int endnum=tail+8;
	int i;
	memset(RecFrame,0,sizeof(RecFrame));
	//将找出来的帧拷贝到接收帧中
	for(int i=beginnum;i<=endnum;i++) 
	{
		RecFrame[i]=str[i];
	}
} 

//帧封装、解封模块
namespace FRAME
{
	//帧封装
	int frame(char *str)
	{
        int target=FindTarget(str);
        printf("str'target is %d\n",target);
		char Str[10];
		memset(Str,0,sizeof(Str));
		if(target)
		{
			for(int i=0;i<=target;i++)
			{
				Str[i]=str[i];
			}
			Str[target+1]='0';
			if(target<7)
			{
		    	for(int i=target+1;i<=7;i++)
				{
				    Str[i+1]=str[i];
		    	}
		    }
		}
		else
		{
			strcpy(Str,str);
		} 
		memset(Frame,0,sizeof(Frame));
		strcat(Frame,head);
		strcat(Frame,Str);
		strcat(Frame,Str);
		strcat(Frame,end);
	}
	int deframe(char *str){//解封装
		int i,j=0;
		char data1[10];
		char check1[10];
		memset(data1, 0, sizeof(data1));
		memset(check1, 0, sizeof(check1));
		int len=strlen(str);
		rep(i,8,len/2-1){
			data1[j++]=str[i];
		}
		j=0;
		rep(i,len/2,len-9){
			check1[j++]=str[i];
		}
        int target1=FindTarget(data1);
        printf("target of data为：%d\n",target1);
        int target2=FindTarget(check1);
        printf("target of check为：%d\n",target2);
		memset(data, 0, sizeof(data));
		memset(check, 0, sizeof(check));
		if(target1){
			rep(i,0,target1){
			    data[i]=data1[i];
			}	
			if(target1<7){
			    rep(i,target1+2,len/2-9){
				    data[i-1]=data1[i];
			  }
			}	
		} 
		else{
			strcpy(data,data1);
		}
		if(target2){
		    rep(i,0,target2){
			    check[i]=check1[i];
			}	
			if(target2<7){
			    rep(i,target2+2,len/2-9){
				    check[i-1]=check1[i];
			   }
		    }	
		} 
		else{
			strcpy(check,check1);
		}
	}
}
int main(){
	printf("\n\n-----------------网络层------------------\n\n");
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData); 

	memset(sendbuffer, 0, sizeof(sendbuffer));
    memset(buffer, 0, sizeof(buffer));
	memset(buf, 0, sizeof(buf));
    memset(checkbuffer,0,sizeof(checkbuffer));

    
    srand((unsigned) time(NULL));
    int randnum=rand()%500+1; //产生随机数 
    
    //创建套接字
    SOCKET sock = socket(PF_INET, SOCK_DGRAM, 0);
    
    int PhyPort=12100,NetPort=12500,AppPort=12300;

    /*---------------------分割线-------------------*/

    //应用层信息
    printf("本应用层端口号：");
    printf("%d\n",AppPort);
    sockaddr_in AppAddr;
    memset(&AppAddr, 0, sizeof(AppAddr));
    int AppLen=sizeof(AppAddr);
    AppAddr.sin_family = PF_INET;
    AppAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    AppAddr.sin_port = htons(AppPort);

    /*---------------------分割线-------------------*/

    //网络层信息 
    printf("本网络层端口号：");
	printf("%d\n",NetPort);
    sockaddr_in NetAddr;
    memset(&NetAddr, 0, sizeof(NetAddr));
    NetAddr.sin_family = PF_INET;
    NetAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    NetAddr.sin_port = htons(NetPort);
    bind(sock, (SOCKADDR*)&NetAddr, sizeof(SOCKADDR)); //绑定端口 

    /*---------------------分割线-------------------*/

    //物理层信息
    printf("本物理层端口号：");
    printf("%d\n",PhyPort);
    sockaddr_in PhyAddr;
    memset(&PhyAddr, 0, sizeof(PhyAddr));
    int PhyLen=sizeof(PhyAddr);
    PhyAddr.sin_family = PF_INET;
    PhyAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    PhyAddr.sin_port = htons(PhyPort);

	/*---------------------分割线-------------------*/

	//测试与物理层的联通性
	sendto(sock,"connect",8,0,(sockaddr*)&PhyAddr,sizeof(PhyAddr));
	int retval = recvfrom(sock,buffer,MAX_BUFFER_SIZE,0,(sockaddr*)&PhyAddr,&PhyLen);
	if(retval == SOCKET_ERROR){
		printf("接口API关联失败\n");
		return 0;
	}
	//有接收，就可以认为关联成功
	printf("接口API连接成功！\n");
    memset(buffer, 0, sizeof(buffer));

    while(1){
    	uint num[32];
      	uint num1[32];
	    int len1=1;
    	int len2=0;
        char IsSend[2];
		int i,cunt=0,framenum=0,resendnum=0,rerecivenum=0,recivenum=0;
        memset(IsSend,0,sizeof(IsSend));
		printf("正在等待应用层的状态信息\n");
    	recvfrom(sock, IsSend, 2, 0, (SOCKADDR*)&AppAddr, &AppLen);

        if(IsSend[0]=='1'){
           	printf("\n\n------------正在接收应用层的消息------------\n\n") ; 

	        memset(buffer, 0, sizeof(buffer));
            recvfrom(sock, buffer, MAX_BUFFER_SIZE, 0, (SOCKADDR*)&AppAddr, &AppLen);
            printf("从应用层收到的信息为：%s\n",buffer);

			memset(buf,0,sizeof(buf));
			rep(i,0,4){              //提取目的端口号
				buf[i]=buffer[i];
			}
			char mybuf[9]="00000";
			if(!strcmp(buf,mybuf)){//对比目的端口号，判断是否为群发消息
				printf("该信息为群发消息！\n");
				itoa(PhyPort,SourcePort,10);//将本地端口号转化为字符串类型
				memset(buf, 0, sizeof(buf));
			    strcat(buf,SourcePort);
			    strcat(buf,buffer);

			    printf("处理后的信息为：%s\n正在与交换机建立连接.......\n",buf);
				memset(buffer,0,sizeof(buffer));
			    uint Num[MAX_BUFFER_SIZE],Len=1;

                //将源端口号和目的端口号发送至交换机,这里的目的端口号作为一个群发标志
			    rep(i,0,9){
                    buffer[i]=buf[i];
			    }
			    printf("%s\n",buffer);

                //将端口号信息转码后发往物理层
			    for(int i=0;i<strlen(buffer);i++){
	                Num[Len++]=buffer[i];
		        }
                memset(sendbuffer, 0, sizeof(sendbuffer));
		        CODE::code(Num,Len-1,sendbuffer);//转码操作
			    printf("%s\n",sendbuffer);

                //将整个比特流进行帧封装
			    char Buffer[MAX_BUFFER_SIZE];
			    memset(Buffer,0,sizeof(Buffer));
			    strcat(Buffer,head);
			    strcat(Buffer,sendbuffer);
			    strcat(Buffer,end);
			    sendto(sock, Buffer, strlen(Buffer), 0, (SOCKADDR*)&PhyAddr, sizeof(PhyAddr));//
			    memset(buffer,0,sizeof(buffer));
                retval=recvfrom(sock, buffer, MAX_BUFFER_SIZE, 0, (SOCKADDR*)&PhyAddr, &PhyLen);
			    if(retval){
				    printf("%s\n",buffer);//连接反馈信息
				    printf("连接成功！即将进行转码操作......\n\n");
			    }
			    else{
				    printf("连接失败！\n");
			    }

				int number=1;
                //将字符转为ASCII码 
                for(int i=0;i<strlen(buf);i++){
	                num[len1++]=buf[i];
		        }
		        memset(sendbuffer, 0, sizeof(sendbuffer));
		        CODE::code(num,len1-1,sendbuffer);//转码操作
				memset(buffer,0,sizeof(buffer));
				strcat(buffer,head);
				strcat(buffer,sendbuffer);
				strcat(buffer,end);
				sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR*)&PhyAddr, sizeof(PhyAddr));
				printf("群发成功！\n");
				printf("断开连接成功！\n");
			}
			else{
				printf("该信息为点到点信息！\n");
			    memset(buf, 0, sizeof(buf));

                itoa(PhyPort,SourcePort,10);//将端口号转化为字符串类型
			    memset(buf, 0, sizeof(buf));
			    strcat(buf,SourcePort);
			    strcat(buf,buffer);
			    printf("处理后的信息为：%s\n正在与交换机建立连接.......\n",buf);
			    memset(buffer,0,sizeof(buffer));
			    uint Num[MAX_BUFFER_SIZE],Len=1;

                //将源端口号和目的端口号发送至交换机
			    rep(i,0,9){
                    buffer[i]=buf[i];
			    }
			    printf("%s\n",buffer);
			    for(int i=0;i<strlen(buffer);i++){
	                Num[Len++]=buffer[i];
		        }
                memset(sendbuffer, 0, sizeof(sendbuffer));
		        CODE::code(Num,Len-1,sendbuffer);//转码操作
			    printf("%s\n",sendbuffer);
			    char Buffer[MAX_BUFFER_SIZE];
			    memset(Buffer,0,sizeof(Buffer));
			    strcat(Buffer,head);
			    strcat(Buffer,sendbuffer);
			    strcat(Buffer,end);
			    sendto(sock, Buffer, strlen(Buffer), 0, (SOCKADDR*)&PhyAddr, sizeof(PhyAddr));
			    memset(buffer,0,sizeof(buffer));
                retval=recvfrom(sock, buffer, MAX_BUFFER_SIZE, 0, (SOCKADDR*)&PhyAddr, &PhyLen);
			    if(retval){
				    printf("%s\n",buffer);
				    printf("连接成功！即将进行转码操作......\n\n");
			    }
			    else{
				    printf("连接失败！\n");
			    }
                int number=1;
                //将字符转为ASCII码 
                for(int i=0;i<strlen(buf);i++){
	                num[len1++]=buf[i];
		        }
		        memset(sendbuffer, 0, sizeof(sendbuffer));
		        CODE::code(num,len1-1,sendbuffer);//转码操作
		        printf("转码成功！比特流为:%s\n即将成帧发送！.............\n",sendbuffer);

			    while(1){	//每隔0.5秒发送一次请求信息	
				    printf("正在请求发送.........\n"); 
                    sendto(sock, "11111111", 8, 0, (SOCKADDR*)&PhyAddr, sizeof(PhyAddr));
                    memset(buffer, 0, sizeof(buffer));
                    recvfrom(sock, buffer, MAX_BUFFER_SIZE, 0, (SOCKADDR*)&PhyAddr, &PhyLen);
                    if(buffer[1]=='0'){
        	            printf("对方忙.....将稍后发送\n");
			            Sleep(500); 
		            }
				    else{	
			            printf("可发送信息...\n");
					    break;
				    }
			    } 
			
			    //发送长度信息
			    char STRLEN[MAX_BUFFER_SIZE];
			    memset(STRLEN,0,sizeof(STRLEN));
			    itoa(strlen(sendbuffer)/8,STRLEN,10);
			    printf("%s\n",STRLEN);
			    Len=1;
			    for(int i=0;i<strlen(STRLEN);i++){
	                Num[Len++]=STRLEN[i];
		        }
			    memset(buffer, 0, sizeof(buffer));
		        CODE::code(Num,Len-1,buffer);//转码操作
                memset(buf,0,sizeof(buf));
			    strcat(buf,head);
			    strcat(buf,buffer);
			    strcat(buf,end);
			    printf("发送的长度信息为：%s\n",buf);
			    printf("一共要发送%d帧\n",strlen(sendbuffer)/8);
			    sendto(sock, buf, strlen(buf), 0, (SOCKADDR*)&PhyAddr, sizeof(PhyAddr));
			    memset(buf,0,sizeof(buf));
			    recvfrom(sock, buf,MAX_BUFFER_SIZE, 0, (SOCKADDR*)&PhyAddr, &PhyLen);
			    if(buf[0]=='1'){
				    printf("对方接收成功！\n");
			    }
			    else{
				    printf("对方接收失败！\n");
			    }

                char str[9];
                memset(str, 0, sizeof(str));

                rep(i,0,strlen(sendbuffer)){//成帧发送
        	        str[cunt++]=sendbuffer[i];
        	        if(cunt>=8){
        		        cunt=0;//重置计数器      

        	            FRAME::frame(str);
        	            printf("第%d帧已封装完毕！封装帧为：%s\n即将发送\n",++framenum,Frame);
        	            while(1){		
							char TFrame[MAX_BUFFER_SIZE];
							memset(TFrame,0,sizeof(TFrame));
							strcpy(TFrame,Frame);				
							randnum=rand()%500+1;
						    if(randnum<10){
							    if(TFrame[16]=='0')TFrame[16]='1';
								else TFrame[16]='0';
						    }
        	                sendto(sock, TFrame, strlen(TFrame), 0, (SOCKADDR*)&PhyAddr, sizeof(PhyAddr));
        	                printf("发送完毕，正在等待确认.......\n");
        	                recvfrom(sock, checkbuffer,MAX_BUFFER_SIZE, 0, (SOCKADDR*)&PhyAddr, &PhyLen);
        	                if(checkbuffer[5]=='1'){
        	    	            printf("确认成功！第%d帧发送完毕\n",framenum);
        	    	            break;
			    	        }
				            printf("在发送过程中出现了一个小错误，即将重发此帧\n");
				            resendnum++;//重发次数+1 
            	        }
			        }
		        }
		        printf("发送完毕！一共发送%d帧，重发%d次\n",framenum,resendnum); 
			    printf("正在与交换机断开连接\n");
			    sendto(sock, mmmmmmm, strlen(mmmmmmm), 0, (SOCKADDR*)&PhyAddr, sizeof(PhyAddr));
			    printf("断开连接成功！\n");
            }
		}
        else{
			memset(sendbuffer, 0, sizeof(sendbuffer));
		    printf("\n\n---------正在从物理层接收信息-----------\n\n");
			while(1){//接收请求信息
			    memset(buf,0,sizeof(buf));
				recvfrom(sock, buf, MAX_BUFFER_SIZE, 0, (SOCKADDR*)&PhyAddr, &PhyLen);
				if(!strcmp(buf,Broadcast)){
					printf("即将接收一个群发信息！\n");
					memset(buffer,0,sizeof(buffer));
                    recvfrom(sock, buffer, MAX_BUFFER_SIZE, 0, (SOCKADDR*)&PhyAddr, &PhyLen);
					memset(sendbuffer,0,sizeof(sendbuffer));
                    int j=0;
					//提取有效信息
                    rep(i,8,strlen(buffer)-9){
                        sendbuffer[j++]=buffer[i];
                    }
					printf("%s\n",sendbuffer);
                    int n=CODE::decode(Num1,sendbuffer,strlen(sendbuffer));//解封
                    len2=0;
                    memset(buffer,0,sizeof(buffer));
                    for(int j=1;j<=n;j++){
		               buffer[len2++]=char(Num1[j]);
	                }
					printf("转码成功！信息为：%s\n正在将信息发往应用层！..............\n",buffer); 
                    sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR*)&AppAddr, sizeof(AppAddr));
					break;
				}
				else{
					printf("接收到了接收请求...\n");		
		            randnum=rand()%500+1;
		            if(randnum%2==0){//通过产生一个随机整数，然后通过奇偶性来决定是否可以接收信息
		                sendto(sock, "00000000", strlen("00000000"), 0, (SOCKADDR*)&PhyAddr, sizeof(PhyAddr));
		                printf("正忙....无法接收信息\n");
		            }
		            else{
		                sendto(sock, "11111111", strlen("11111111"), 0, (SOCKADDR*)&PhyAddr, sizeof(PhyAddr));
				        printf("可接受信息.......");
					    break;
		            }
				}    	   
			}
			if(!strcmp(buf,Broadcast)){//收到群发消息，则退出该层循环
				continue;
			}
			uint Num1[MAX_BUFFER_SIZE];
			memset(Num1,0,sizeof(Num1));
            printf("正在接收长度信息\n");
			memset(buffer, 0, sizeof(buffer));
            recvfrom(sock, buffer, MAX_BUFFER_SIZE, 0, (SOCKADDR*)&PhyAddr, &PhyLen);
			sendto(sock, "111111111", 9, 0, (SOCKADDR*)&PhyAddr, sizeof(PhyAddr));
			int j=0;
			memset(sendbuffer,0,sizeof(sendbuffer));
			rep(i,8,strlen(buffer)-9){
            sendbuffer[j++]=buffer[i];
            }
			printf("%s\n",sendbuffer);
			memset(buffer,0,sizeof(buffer));
            int ll=CODE::decode(Num1,sendbuffer,strlen(sendbuffer));//解封
			memset(sendbuffer,0,sizeof(sendbuffer));
			len2=0;
			printf("%d\n",ll);
			for(int j=1;j<=ll;j++){
		    buffer[len2++]=char(Num1[j]);
	        }
			printf("%s\n",buffer);
			int num11=atoi(buffer);
			printf("即将接受帧长度%d的信息\n",num11);
			i=1;
            while(1){ 
				    printf("正在接收第%d帧\n",i++);
       	            recivenum++;
       	            while(1){
       	                memset(buffer, 0, sizeof(buffer));
                        recvfrom(sock, buffer, MAX_BUFFER_SIZE, 0, (SOCKADDR*)&PhyAddr, &PhyLen);
                        printf("接收成功！即将进行解封......\n");
                        FindFrame(buffer);//寻找帧 
                        FRAME::deframe(RecFrame);//解封帧 
                        printf("解封后的数据为：%s  校验数据为：%s",data,check);
                        printf("解封成功！正在进行校验....\n");
                        if(!strcmp(data,check)){
                            printf("校验成功！");
                            sendto(sock, "111111111", strlen("111111111"), 0, (SOCKADDR*)&PhyAddr, sizeof(PhyAddr));
                            strcat(sendbuffer,data);
                            break;
			            }
			            printf("校验失败！即将发送错误信息........\n");
			            sendto(sock, "000000000", strlen("000000000"), 0, (SOCKADDR*)&PhyAddr, sizeof(PhyAddr));
			            rerecivenum++;//重新接收信息次数+1 
			            printf("发送成功！即将重新接收信息\n");
                   }
				   if(i>num11){
					   break;
				   }
		    }
            printf("共接收%d帧，重新接收%d次\n",recivenum,rerecivenum);
            printf("整个bit流为：%s\n即将进行转码",sendbuffer);
            //重置信息 
            memset(buffer, 0, sizeof(buffer));

            int n=CODE::decode(num1,sendbuffer,strlen(sendbuffer));//解封
			len2=0;
            //将ASCII码转为字符类型 
	        for(int j=1;j<=n;j++){
		    buffer[len2++]=char(num1[j]);
	        }
			
	        printf("转码成功！信息为：%s\n正在将信息发往应用层！..............\n",buffer); 
            sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR*)&AppAddr, sizeof(AppAddr));   
        }
	}
    closesocket(sock);
    WSACleanup();
    return 0;
}
