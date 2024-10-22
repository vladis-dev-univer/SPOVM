#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <vector>

using namespace std;

int main(int argc, char *argv[])
{
  int reciever = 0;
  sigset_t set;                       //набор сигналов
  sigemptyset(&set);                  //очищаем набор сигналов на который указывает 'set'
  sigaddset(&set, SIGUSR1);           //добовляем сигнал SIGUSR1
  sigprocmask(SIG_BLOCK, &set, NULL); //получаем маcку сигнала текущего процесса
  int pid = getpid();                 //получаем PID
  char str[] = "This is process - ";
  while (true)
  {
    sigwait(&set, &reciever); //ждём сигнала
    cout << endl;
    for (int i = 0; i < strlen(str); i++)
    {
      usleep(20000);
      printf("%c", str[i]);
    }
    cout << getpid() << "\n\r";
    usleep(10000);
    //cout << "\nThis is process (" << getpid() << ")"<< "\n\r";
    //cout << "--------------------"<< "\n\r";
    kill(getppid(), SIGUSR2); // посылка сигнала процессу
    sleep(1);
  }
  return 0;
}
