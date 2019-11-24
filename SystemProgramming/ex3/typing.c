#include <stdio.h> 
#include <termios.h> 
#include <sys/types.h> 
#include <fcntl.h> 
#include <unistd.h> 
#include <time.h>

#define PASSWORDSIZE 12
#define size 3

int main(void) { 
   int typing_n = 0;
   time_t start;
   time_t end;
   int avg_time=0;
   int avg_typing=0;
   int i;
   int fd; 
   int nread, cnt=0, errcnt=0; 
   char ch; 
   char *text[size] = {"20153261JooHwano.","Hello World","System Lab3"}; 

   struct termios init_attr, new_attr;

   fd = open(ttyname(fileno(stdin)), O_RDWR); 
   tcgetattr(fd, &init_attr);
   new_attr = init_attr; 
   new_attr.c_lflag &= ~ICANON; 
   new_attr.c_lflag &= ~ECHO; 
   /* ~(ECHO | ECHOE | ECHOK | ECHONL); */

   new_attr.c_cc[VMIN] = 1; 
   new_attr.c_cc[VTIME] = 0; 

   if (tcsetattr(fd, TCSANOW, &new_attr) != 0) { 
      fprintf(stderr, "NO Terminal Set.\n"); 
   } 
   printf("input next sentence.\n");
   time(&start);
   for(i = 0; i<size; i++){
      printf("%s\n",text[i]); 
      while ((nread=read(fd, &ch, 1)) > 0 && ch != '\n') { 
         if (ch == text[i][cnt++]) 
            write(fd, &ch, 1); 
         else { 
            write(fd, "*", 1); 
            errcnt++; 
         } 
         typing_n++;
      } 
      printf("\n");
      cnt = 0;
   }
   time(&end);

   avg_time = (end-start);
   avg_typing = (typing_n-errcnt)*60/avg_time;
   
   printf("\nerror num : %d\n", errcnt); 
   printf("total typing num : %d\n",typing_n);
   printf("total typing time(s) : %d\n",avg_time);
   printf("avg typing(m) : %d\n",avg_typing);
   tcsetattr(fd, TCSANOW, &init_attr); 
   close(fd); 
}

