#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#define PORT 4160
#define MAXPENDING 5
#define BUFSIZE 1024

int main()
{
   int servSockfd;
   int clntSockfd;
   struct sockaddr_in servAddr;
   struct sockaddr_in clntAddr;
   char recvBuffer[BUFSIZE];
   char sendBuffer[BUFSIZE];
   int clntLen;
   int iRet;
   int iMaxSock;
   fd_set readfds;

   // socket make
   
   if((servSockfd = socket(AF_INET, SOCK_STREAM,0))==-1)
   {
      perror("sock failed");
      exit(1);
   }

   //servAddr reset
   
   memset(&servAddr,0,sizeof(servAddr));
   servAddr.sin_family = AF_INET;
   servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
   servAddr.sin_port = htons(PORT);

   if(bind(servSockfd,(struct sockaddr *)&servAddr,sizeof(servAddr))==-1)
   {
      perror("bind failed");
      exit(1);
   }
   // client wait
   
   if(listen(servSockfd, MAXPENDING)==-1)
   {
      perror("listen failed");
      exit(1);
   }

   clntSockfd = 0;
   iMaxSock = servSockfd + 1;

   while(1)
   {
      //readfds reset, select start

      FD_ZERO(&readfds);
      FD_SET(servSockfd, &readfds);
      FD_SET(0, &readfds);
      FD_SET(clntSockfd, &readfds);
      select(iMaxSock, &readfds, 0, 0, 0);

      //client Massage?

      if(FD_ISSET(clntSockfd, &readfds) !=0)
      {
         //client meassage receive
         iRet = read(clntSockfd,recvBuffer,BUFSIZE);
         if(iRet == -1)
         {
            perror("read failed");
            exit(1);
         }

         //client messagd not exit
         if(iRet == 0)
         {
            break;
         }

         recvBuffer[iRet] = '\0';
         printf("client : %s\n", recvBuffer);
      }
      // keybord input ?
      else if(FD_ISSET(0, &readfds) != 0)
      {
         //0 receive client 89
         fgets(sendBuffer,BUFSIZE,stdin);

         iRet = write(clntSockfd,sendBuffer,strlen(sendBuffer));
         if(iRet != strlen(sendBuffer))
         {
            perror("sent failed");
            exit(1);
         }
      }
      //client wjqthr dhkTs? 99
      else if(FD_ISSET(servSockfd, &readfds) != 0)
      {
         clntLen = sizeof(clntAddr);

         //clent access
         clntSockfd = accept(servSockfd,(struct sockaddr *)&clntAddr,&clntLen);
         if(clntSockfd == -1)
         {
            perror("accept failed");
            exit(1);
         }
         printf("client is in\n");
               // 113
         iMaxSock = clntSockfd+1;
         }
      }

      close(clntSockfd);
      close(servSockfd);

      return 0;
}

