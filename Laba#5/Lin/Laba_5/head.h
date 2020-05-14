#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <inttypes.h>
#include <dlfcn.h>	//библиот. для dll

#include <sys/wait.h>

#include <termios.h>
#include <pthread.h>

#include <fcntl.h>
void* ThreadReader(void* fdata);
void* ThreadWriter(void* fdata);


#define buffer_SIZE 512
#define MESSAGE_SIZE 100

char buffer[buffer_SIZE];


struct Data;
void *library_handler;
struct Data *createInfoStruct();
void CreateSem(struct Data *data);
void CreateThreads(struct Data *data);
void Search(struct Data *data);
void CloseApp(struct Data *data);
