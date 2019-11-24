#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>

#define PORT 4160
#define BUFSIZE 1024

void signalHandler(int signo);

int sockfd;

int main(int argc, char *argv[])
{
   struct sockaddr_in servAddr;
   char sendBuffer[BUFSIZE];
   char recvBuffer[BUFSIZE];
   int iRet;
   pid_t pid;
   fd_set readfds;
   int iMaxSock;

   if(argc!=2)
   {
      fprintf(stderr, "Usage:%s [IP_addr]\n", argv[0]);
      exit(1);
   }

   //소켓생성
   if((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1)
   {
      perror("sock failed");
      exit(1);
   }

   memset(&servAddr, 0, sizeof(servAddr));

   servAddr.sin_family = AF_INET;
   servAddr.sin_addr.s_addr = inet_addr(argv[1]);
   servAddr.sin_port = htons(PORT);

   //서버에 연결요청
   if(connect(sockfd,(struct sockaddr *)&servAddr, sizeof(servAddr)) == -1)
   {
      perror("connext failed");
      exit(1);
   }

   //종료 signal이 발생하면 signalHandler 함수 실행
   signal(SIGINT,signalHandler);

   while(1)
   {
      //readfds 초기화, select 감시 시작
      FD_ZERO(&readfds);
      FD_SET(sockfd, &readfds);
      FD_SET(0, &readfds);
      iMaxSock = sockfd +1;
      select(iMaxSock, &readfds, 0,0,0);

      //sockfd 에서 읽기가 발생했나?
      if(FD_ISSET(sockfd, &readfds)!=0)
      {
         //server에서 보낸 내용을 읽음
         iRet = read(sockfd,recvBuffer,BUFSIZE);

         if(iRet == -1)
         {
            perror("recv failed");
            exit(1);
         }
         recvBuffer[iRet]='\0';
         printf("server : %s\n", recvBuffer);
      }
      //0(표준입력)에서 읽기가 발생했나?
      else if(FD_ISSET(0, &readfds)!=0)
      {
         fgets(sendBuffer,BUFSIZE,stdin);
         
         //입력받은 내용이 quit이면 종료
         if(!strncmp(sendBuffer,"quit",4))
         {
            kill(getpid(),SIGINT);
         }

         //입력받는 내용을 server에 보냄
         iRet = write(sockfd,sendBuffer,strlen(sendBuffer));
            
         if(iRet!=strlen(sendBuffer))
         {
            perror("send failed");
            exit(1);
         }
      }
   }
   close(sockfd);

   return 0;
}

void signalHandler(int signo)
{
   close(sockfd);
   exit(0);
}


