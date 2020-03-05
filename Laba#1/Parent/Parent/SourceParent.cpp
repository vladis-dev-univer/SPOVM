#define _CRT_SECURE_NO_WARNINGS
#include<windows.h>
#include<process.h>
#include<stdio.h>
#include <conio.h>
#include<ctime>


int main() {
	PROCESS_INFORMATION procInfo;
	STARTUPINFO startupInfo;
	ZeroMemory(&startupInfo, sizeof(startupInfo));
	ZeroMemory(&procInfo, sizeof(procInfo));
	printf("PARENT: Process start parent\n");
	if (!CreateProcessA("D:\\SPOVM_PRJ\\Laba#1\\Child\\Debug\\Child.exe",
		NULL,
		NULL,
		NULL,
		FALSE,
		/*CREATE_NEW_CONSOLE*/0,
		NULL,
		NULL,
		&startupInfo,
		&procInfo
	))
	{
		printf("Error creating process\n");
		_getch();
		return -1;
	}
	WaitForSingleObject(procInfo.hProcess, INFINITE);

	CloseHandle(procInfo.hProcess);
	CloseHandle(procInfo.hThread);

	time_t now = time(0);
	char *dt = ctime(&now);
	int ppid;
	ppid = _getpid();

	printf("\n\tPARENT: This is my pid -> %d\n\tTime and date: %s\nEnd of process parent... ", ppid, dt);
	printf("\n\n");
	system("pause");
	return 0;
}