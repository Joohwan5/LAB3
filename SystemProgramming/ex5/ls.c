#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifdef WIN32
#define popen _popen
#define pclose _plose
#endif

int main(int argc, char* argv[]) {
	char Command[100];
	size_t readSize = 0;
	FILE *fp = NULL;
	char Buff[1024];
	
	sprintf(Command, "%s %s", argv[1], argv[2]);
	fp = popen(Command, "r");

	if (!fp) {
		printf("error[%d: %s]\n", errno, strerror(errno));
		return -1;
	}

	readSize = fread((void*)Buff,sizeof(char),1024-1, fp);

	if (readSize == 0) {
		pclose(fp);
		printf("error[%d: %s]\n", errno, strerror(errno));
		return -1;
	}
	
	pclose(fp);
	Buff[readSize] = 0;

	printf("%s\n", Buff);
	return 0;
}

