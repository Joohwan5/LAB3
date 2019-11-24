/*
  * simple-c-shell.c
  * 
 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>
#include "util.h"

#define LIMIT 256 // 명령에 대한 최대 토큰 수
#define MAXLINE 1024 // 사용자 입력의 최대 문자 수


void init(){
      // 대화식으로 실행 중인지 확인
        GBSH_PID = getpid();
        // STDIN이 터미널 인 경우 쉘은 대화식입니다.    
        GBSH_IS_INTERACTIVE = isatty(STDIN_FILENO);  

      if (GBSH_IS_INTERACTIVE) {         
         while (tcgetpgrp(STDIN_FILENO) != (GBSH_PGID = getpgrp()))
               kill(GBSH_PID, SIGTTIN);             
                 
                 
           // SIGCHILD 및 SIGINT에 대한 신호 핸들러 설정
         act_child.sa_handler = signalHandler_child;
         act_int.sa_handler = signalHandler_int;         
         
         
         sigaction(SIGCHLD, &act_child, 0);
         sigaction(SIGINT, &act_int, 0);
         
         // 자체 프로세스 그룹에 참여

         setpgid(GBSH_PID, GBSH_PID); 
         GBSH_PGID = getpgrp();
         if (GBSH_PID != GBSH_PGID) {
               printf("Error, the shell is not process group leader");
               exit(EXIT_FAILURE);
         }
         // 터미널 제어

         tcsetpgrp(STDIN_FILENO, GBSH_PGID);  
         
         // 쉘의 기본 터미널 속성 저장

         tcgetattr(STDIN_FILENO, &GBSH_TMODES);

         // 다른 방법으로 사용될 현재 디렉토리를 얻는다

         currentDirectory = (char*) calloc(1024, sizeof(char));
        } else {
                printf("Could not make the shell interactive.\n");
                exit(EXIT_FAILURE);
        }
}

/**
 * 쉘의 시작 화면을 인쇄하는 데 사용되는 방법

 */
void welcomeScreen(){
        printf("\n\t============================================\n");
        printf("\t               Simple C Shell\n");
        printf("\t--------------------------------------------\n");
        printf("\t               team4  고라파덕\n");
        printf("\t       20153245김덕훈, 20153261주환오\n");
        printf("\t============================================\n");
        printf("\n\n");
}


/**
 * SIGCHLD를위한 시그널 핸들러

 */
void signalHandler_child(int p){
   // 모든 죽은 프로세스를 기다립니다.
   while (waitpid(-1, NULL, WNOHANG) > 0) {
   }
   printf("\n");
}

/* *
 * SIGINT를위한 시그널 핸들러
 */
void signalHandler_int(int p){
   // We send a SIGTERM signal to the child process
   if (kill(pid,SIGTERM) == 0){
      printf("\nProcess %d received a SIGINT signal\n",pid);
      no_reprint_prmpt = 1;         
   }else{
      printf("\n");
   }
}

/* *
 * 쉘 프롬프트를 표시합니다
 */
void shellPrompt(){
   // We print the prompt in the form "<user>@<host> <cwd> >"
   char hostn[1204] = "";
   gethostname(hostn, sizeof(hostn));
   printf("%s@%s %s > ", getenv("LOGNAME"), hostn, getcwd(currentDirectory, 1024));
}

//디렉토리 변경 방
int changeDirectory(char* args[]){
   // 경로를 쓰지 않으면 ( 'cd'만 해당) 홈 디렉토리로 이동하십시오.

   if (args[1] == NULL) {
      chdir(getenv("HOME")); 
      return 1;
   }

   else{ 
      if (chdir(args[1]) == -1) {
         printf(" %s: no such directory\n", args[1]);
            return -1;
      }
   }
   return 0;
}


 // 다른 환경 변수를 관리하는 데 사용되는 방법 
int manageEnviron(char * args[], int option){
   char **env_aux;
   switch(option){
      // Case 'environ': 환경 변수와 함께 인쇄합니다
      case 0: 
         for(env_aux = environ; *env_aux != 0; env_aux ++){
            printf("%s\n", *env_aux);
         }
         break;
      // Case 'setenv': 환경 변수를 값으로 설정
      case 1: 
         if((args[1] == NULL) && args[2] == NULL){
            printf("%s","Not enought input arguments\n");
            return -1;
         }
         
         // 새로운 변수와 덮어 쓴 변수에 다른 출력을 사용합니다.
         if(getenv(args[1]) != NULL){
            printf("%s", "The variable has been overwritten\n");
         }else{
            printf("%s", "The variable has been created\n");
         }
         
         // 변수에 값을 지정하지 않으면 ""로 설정합니다

         if (args[2] == NULL){
            setenv(args[1], "", 1);
         // 변수를 주어진 값으로 설정

         }else{
            setenv(args[1], args[2], 1);
         }
         break;
      // Case 'unsetenv': 환경 변수를 삭제합니다

      case 2:
         if(args[1] == NULL){
            printf("%s","Not enought input arguments\n");
            return -1;
         }
         if(getenv(args[1]) != NULL){
            unsetenv(args[1]);
            printf("%s", "The variable has been erased\n");
         }else{
            printf("%s", "The variable does not exist\n");
         }
      break;
         
         
   }
   return 0;
}
 

// 프로그램을 시작하는 방법. 백그라운드에서 실행할 수 있습니다 
void launchProg(char **args, int background){    
    int err = -1;
    
    if((pid=fork())==-1){
       printf("Child process could not be created\n");
       return;
    }
    // pid == 0은 다음 코드가 자식 프로세스와 관련이 있음을 나타냅니다.

   if(pid==0){
      // signalHandler_int로 처리하는 프로세스)   
      signal(SIGINT, SIG_IGN);
      
      // parent = <pathname> / simple-c-shell을 환경 변수로 설정합니다.
      setenv("parent",getcwd(currentDirectory, 1024),1);   
      
      // 존재하지 않는 명령을 실행하면 프로세스가 종료됩니다
      if (execvp(args[0],args)==err){
         printf("Command not found");
         kill(getpid(),SIGTERM);
      }
    }
    
    
    // 프로세스가 백그라운드에서 요청되지 않은 경우 기다립니다.
    if (background == 0){
       waitpid(pid,NULL,0);
    }else{
       // 백그라운드 프로세스를 만들려면 현재 프로세스
       // 대기 호출을 건너 뛰어야합니다. SIGCHILD 핸들러
       // signalHandler_child는 반환 값을 처리합니다.
       printf("Process created with PID: %d\n",pid);
    }    
}
 
/* *
* I / O 리디렉션 관리에 사용되는 방법
*/ 
void fileIO(char * args[], char* inputFile, char* outputFile, int option){
    
   int err = -1;
   
   int fileDescriptor; // between 0 and 19, describing the output or input file
   
   if((pid=fork())==-1){
      printf("Child process could not be created\n");
      return;
   }
   if(pid==0){
      // 옵션 0 : 출력 리디렉션
      if (option == 0){
         // 쓰기 전용으로 0에서 파일을 자르는 파일을 열거 나 만듭니다.
         fileDescriptor = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600); 
         // 표준 출력을 적절한 파일로 바꿉니다.
         dup2(fileDescriptor, STDOUT_FILENO); 
         close(fileDescriptor);
      // 옵션 1 : 입력 및 출력 리디렉션
      }else if (option == 1){
         // 파일을 읽기 전용으로 엽니 다 (STDIN입니다).
         fileDescriptor = open(inputFile, O_RDONLY, 0600);  
         // 표준 입력을 적절한 파일로 바꿉니다.
         dup2(fileDescriptor, STDIN_FILENO);
         close(fileDescriptor);
         // 출력 파일과 동일
         fileDescriptor = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600);
         dup2(fileDescriptor, STDOUT_FILENO);
         close(fileDescriptor);       
      }
       
      setenv("parent",getcwd(currentDirectory, 1024),1);
      
      if (execvp(args[0],args)==err){
         printf("err");
         kill(getpid(),SIGTERM);
      }       
   }
   waitpid(pid,NULL,0);
}

/* *
* 파이프 관리에 사용되는 방법.
*/ 
void pipeHandler(char * args[]){
   // 파일 기술자
   int filedes[2];  // 위치. 0 출력, pos. 파이프 1 입력
   int filedes2[2];
   
   int num_cmds = 0;
   
   char *command[256];
   
   pid_t pid;
   
   int err = -1;
   int end = 0;
   
   // 다른 루프에 사용되는 변수
   int i = 0;
   int j = 0;
   int k = 0;
   int l = 0;
   
   // 먼저 명령 수를 계산합니다
   while (args[l] != NULL){
      if (strcmp(args[l],"|") == 0){
         num_cmds++;
      }
      l++;
   }
   num_cmds++;
   
   while (args[j] != NULL && end != 1){
      k = 0;
      // 명령을 저장하기 위해 보조 포인터 배열을 사용합니다.
      while (strcmp(args[j],"|") != 0){
         command[k] = args[j];
         j++;   
         if (args[j] == NULL){
            // 프로그램 종료를 막는 데 사용되는 'end'변수   
            end = 1;
            k++;
            break;
         }
         k++;
      }

      command[k] = NULL;
      j++;      
      
      // 반복인지 아닌지에 따라 파이프 입력에 대해 다른 설명자를 설정합니다.
      if (i % 2 != 0){
         pipe(filedes); 
      }else{
         pipe(filedes2); 
      }
      
      pid=fork();
      
      if(pid==-1){         
         if (i != num_cmds - 1){
            if (i % 2 != 0){
               close(filedes[1]); 
            }else{
               close(filedes2[1]); 
            } 
         }         
         printf("Child process could not be created\n");
         return;
      }
      if(pid==0){
         if (i == 0){
            dup2(filedes2[1], STDOUT_FILENO);
         }
         // 마지막 명령인지 여부에 따라
         // 홀수 또는 짝수 위치에 배치됩니다.
         else if (i == num_cmds - 1){
            if (num_cmds % 2 != 0){ 
               dup2(filedes[0],STDIN_FILENO);
            }else{ 
               dup2(filedes2[0],STDIN_FILENO);
            }
         // 두 개의 파이프를 사용해야합니다. 하나는 입력 용이고 다른 하나는 입력 용입니다.
         }else{ 
            if (i % 2 != 0){
               dup2(filedes2[0],STDIN_FILENO); 
               dup2(filedes[1],STDOUT_FILENO);
            }else{ 
               dup2(filedes[0],STDIN_FILENO); 
               dup2(filedes2[1],STDOUT_FILENO);               
            } 
         }
         
         if (execvp(command[0],command)==err){
            kill(getpid(),SIGTERM);
         }      
      }
            
      // 부모의 설명자 닫기
      if (i == 0){
         close(filedes2[1]);
      }
      else if (i == num_cmds - 1){
         if (num_cmds % 2 != 0){               
            close(filedes[0]);
         }else{               
            close(filedes2[0]);
         }
      }else{
         if (i % 2 != 0){               
            close(filedes2[0]);
            close(filedes[1]);
         }else{               
            close(filedes[0]);
            close(filedes2[1]);
         }
      }
            
      waitpid(pid,NULL,0);
            
      i++;   
   }
}
/* *
* 표준 입력을 통해 입력 된 명령을 처리하는 데 사용되는 방법
*/ 
int commandHandler(char * args[]){
   int i = 0;
   int j = 0;
   
   int fileDescriptor;
   int standardOut;
   
   int aux;
   int background = 0;
   
   char *args_aux[256];
   
   // 특수 문자를 찾아서 명령 자체를 분리합니다.
   // 인수를위한 새로운 배열
   while ( args[j] != NULL){
      if ( (strcmp(args[j],">") == 0) || (strcmp(args[j],"<") == 0) || (strcmp(args[j],"&") == 0)){
         break;
      }
      args_aux[j] = args[j];
      j++;
   }
   
   // 'exit'명령은 쉘을 종료합니다
   if(strcmp(args[0],"exit") == 0) exit(0);
   // 'pwd'명령은 현재 디렉토리를 인쇄합니다
    else if (strcmp(args[0],"pwd") == 0){
      if (args[j] != NULL){
         // 파일 출력을 원한다면
         if ( (strcmp(args[j],">") == 0) && (args[j+1] != NULL) ){
            fileDescriptor = open(args[j+1], O_CREAT | O_TRUNC | O_WRONLY, 0600); 
            // 표준 출력을 적절한 파일로 바꿉니다.
            standardOut = dup(STDOUT_FILENO);    
                                       
            dup2(fileDescriptor, STDOUT_FILENO); 
            close(fileDescriptor);
            printf("%s\n", getcwd(currentDirectory, 1024));
            dup2(standardOut, STDOUT_FILENO);
         }
      }else{
         printf("%s\n", getcwd(currentDirectory, 1024));
      }
   } 
    // 'clear' command clears the screen
   else if (strcmp(args[0],"clear") == 0) system("clear");
   // 디렉토리를 변경하는 'cd'명령
   else if (strcmp(args[0],"cd") == 0) changeDirectory(args);
   // 환경 변수를 나열하는 'environ'명령
   else if (strcmp(args[0],"environ") == 0){
      if (args[j] != NULL){

         if ( (strcmp(args[j],">") == 0) && (args[j+1] != NULL) ){
            fileDescriptor = open(args[j+1], O_CREAT | O_TRUNC | O_WRONLY, 0600); 

            standardOut = dup(STDOUT_FILENO);    
                                       
            dup2(fileDescriptor, STDOUT_FILENO); 
            close(fileDescriptor);
            manageEnviron(args,0);
            dup2(standardOut, STDOUT_FILENO);
         }
      }else{
         manageEnviron(args,0);
      }
   }

   else if (strcmp(args[0],"setenv") == 0) manageEnviron(args,1);

   else if (strcmp(args[0],"unsetenv") == 0) manageEnviron(args,2);
   else{

      while (args[i] != NULL && background == 0){
         // 백그라운드 실행이 요청 된 경우 (마지막 인수 '&')
         // 루프를 종료합니다
         if (strcmp(args[i],"&") == 0){
            background = 1;

         }else if (strcmp(args[i],"|") == 0){
            pipeHandler(args);
            return 1;
            // '<'가 감지되면 입력 및 출력 재지향이 있습니다.   
         
         }else if (strcmp(args[i],"<") == 0){
            aux = i+1;
            if (args[aux] == NULL || args[aux+1] == NULL || args[aux+2] == NULL ){
               printf("Not enough input arguments\n");
               return -1;
            }else{
               if (strcmp(args[aux+1],">") != 0){
                  printf("Usage: Expected '>' and found %s\n",args[aux+1]);
                  return -2;
               }
            }
            fileIO(args_aux,args[i+1],args[i+3],1);
            return 1;
         }
         // '>'가 감지되면 출력 재지향이 있습니다.

         else if (strcmp(args[i],">") == 0){
            if (args[i+1] == NULL){
               printf("Not enough input arguments\n");
               return -1;
            }
            fileIO(args_aux,NULL,args[i+1],0);
            return 1;
         }
         i++;
      }

      args_aux[i] = NULL;
      launchProg(args_aux,background);
      

   }
return 1;
}


int main(int argc, char *argv[], char ** envp) {
   char line[MAXLINE]; // 사용자 입력을위한 버퍼
   char * tokens[LIMIT]; // 명령에서 다른 토큰에 대한 배열
   int numTokens;
      
   no_reprint_prmpt = 0;       // 셸이 인쇄되지 않도록
                  
   pid = -10; 
   
   // 초기화 방법과 시작 화면을 호출합니다.
   init();
   welcomeScreen();
    

   environ = envp;
   
   // shell = <pathname> / simple-c-shell을 환경 변수로 설정합니다.
   setenv("shell",getcwd(currentDirectory, 1024),1);
   
   // 사용자 입력을 읽을 수있는 메인 루프와 프롬프트
   while(TRUE){
      
      if (no_reprint_prmpt == 0) shellPrompt();
      no_reprint_prmpt = 0;
      
      // 라인 버퍼를 비 웁니다
      memset ( line, '\0', MAXLINE );

      // 사용자 입력을 기다립니다
      fgets(line, MAXLINE, stdin);
   
      // 아무 것도 쓰지 않으면 루프가 다시 실행됩니다.
      if((tokens[0] = strtok(line," \n\t")) == NULL) continue;
      
      // 입력의 모든 토큰을 읽고 인수로 사용되는 commandHandler
      numTokens = 1;
      while((tokens[numTokens] = strtok(NULL, " \n\t")) != NULL) numTokens++;
      
      commandHandler(tokens);
      
   }          

   exit(0);
}
