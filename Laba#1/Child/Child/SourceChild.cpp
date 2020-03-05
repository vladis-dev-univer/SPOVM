#define _CRT_SECURE_NO_WARNINGS
#include<ctime>
#include<stdio.h>
#include<stdlib.h>
#include<conio.h>
#include <process.h>


int main()
{
	printf("CHILD: start process child\n");
	time_t now = time(0);
	char *data = ctime(&now);
	int pid, ppid;

	pid = _getpid();

	printf("\tCHILD: Mypid = %d\n\tTime and date: %s\nEnd of process child... ", pid, data);
	printf("\n\n");
	system("pause");
	return 0;
}