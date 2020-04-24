#pragma once
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <stdio.h>
#include <iostream>
using namespace std;

#define SHMSIZE 20
#define UNIQENUMBER 2009
void client(char *, int);