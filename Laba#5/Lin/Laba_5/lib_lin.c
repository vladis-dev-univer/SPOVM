#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>



#include <fcntl.h>

#define BUFFER_SIZE 512


void WriteInFile(int hFile, char* buff) {
	write(hFile, buff, strlen(buff));	                    //запись в файл
}


void ReadFromFile(int hFile, char* buff) {
	int n;
	memset(buff, 0, BUFFER_SIZE * sizeof(char));            //очищаем буфер
	char temp[BUFFER_SIZE];                                 //создаем временный буфер
	do {
		memset(temp, 0, BUFFER_SIZE * sizeof(char));        //очищаем фременный буфер

		n = read(hFile, temp, sizeof(char));                //читаем из файла в буфер
		strcat(buff, temp);                                 //конкатинируем буфер с временным буфером (объединяем)
	} while (n != 0);                                       //читаем пока не кончится файл
}