#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 7777   /* 포트 번호 */
#define MAXPENDING 5   /* 클라이언트 요구가 대기하는 큐의 크기 */
#define BUFSIZE 1024   /* recvBuffer의 크기 */

int main()
{
	
    int servSockfd;
    int clntSockfd;
    struct sockaddr_in servAddr;
    struct sockaddr_in clntAddr;
    char recvBuffer[BUFSIZE];
    int clntLen;
    int recvLen;
    pid_t processID;

    /* 소켓 생성 */
    if((servSockfd=socket(AF_INET, SOCK_STREAM, 0)) == -1) {
       perror("sock failed");
       exit(1);
   }

    /* servAddr를 0으로 초기화 */
    memset(&servAddr, 0, sizeof(servAddr));
    /* servAddr에 IP 주소와 포트 번호를 저장 */
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(PORT);

    /* servSockfd 소켓에 주소 정보 연결 */
    if(bind(servSockfd, (struct sockaddr*)&servAddr, sizeof(servAddr)) == -1) {
       perror("bind failed");
       exit(1);
    }

    /* servSockfd으로 들어오는 클라이언트 요청 기다림 */
    if(listen(servSockfd, MAXPENDING) == -1) {
       perror("listen failed");
       exit(1);
    }

    /* 무한 반복 */
    while(1) {
       clntLen = sizeof(clntAddr);
       /* 요청을 받아들이고 새로운 clntSockfd 소켓이 클라이언트와 연결 */
       if((clntSockfd=accept(servSockfd, (struct sockaddr*)&clntAddr, &clntLen)) == -1) {
          perror("accept failed");
          exit(1);
       }

       /* 자식 프로세스 생성 */
       if((processID=fork()) == -1) {
          perror("fork failed");
          exit(1);
       /* 자식 프로세스는 */
       } else if(processID == 0) {
          /* servSockfd 소켓 닫음 */
          close(servSockfd);
          /* 임의의 클라이언트에 대해 반복 */
          while(1) {
             /* clntSockfd 소켓으로 들어오는 데이터를 받아 recvBuffer에 저장 */
             if((recvLen=recv(clntSockfd, recvBuffer, BUFSIZE, 0)) == -1) {
                perror("recv failed");
                exit(1);
             }
             /* 클라이언트가 연결을 끊으면 recv는 0을 반환 */
             if(recvLen == 0)
                break;

             recvBuffer[recvLen] = '\0';
             /* 받은 데이터를 출력 */
             printf("Recevied: %s\n", recvBuffer);

             /* 받은 데이터를 클라이언트에게 보냄 */
             if(send(clntSockfd, recvBuffer, recvLen, 0) != recvLen) {
                perror("send failed");
                exit(1);
             }
          }
          /* clntSockfd 소켓 닫음 */
          close(clntSockfd);
          /* 자식 프로세스 종료 */
          exit(0);
       }
 
       /* 부모 프로세스는 clntSockfd 소켓 닫고 while 문 시작 부분으로 이동 */
       close(clntSockfd);
    }
}

