#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <signal.h>
#include <ncurses.h>

using namespace std;

int main(int argc, char *argv[])
{
    int reciever = 0;
    initscr();                          //инициализация экрана (ncurses.h)
    sigset_t set;                       //набор сигналов
    sigemptyset(&set);                  //очищаем набор сигналов на который указывает 'set'
    sigaddset(&set, SIGUSR2);           //добовляем сигнал SIGUSR2
    sigprocmask(SIG_BLOCK, &set, NULL); //получаем маку сигнала текущего процесса
    vector<int> pids;
    char answer;

    nodelay(stdscr, true); //устанавливаем экран в режим без задержки(true)
    cout << "Enter '+', '-' or 'q' " << endl;
    while (answer != 'q')
    {
        answer = wgetch(stdscr); //получить однобайтовый символ из терминала
        switch (answer)
        {
        case '+': //если был нажат +
        {
            int pid = fork(); //создаём процесс
            switch (pid)
            {
            case -1: //ошибка создания процесса
            {
                cout << "!!!\t\tError with creating child process" << endl;
                exit(-1);
            }
            case 0: //создание дочернего процесса
            {
                execv("./child", argv); //вызываем скомпилированную
                                        //программу childProcess
                exit(0);
            }
            default: //создание родительского процесса
            {
                pids.push_back(pid); //добавить в конец элемент
                sleep(1);
            }
            break;
            }
        }
        break;
        case '-': //если был нажат -
        {
            if (pids.size() != 0)
            {
                kill(pids[pids.size() - 1], SIGKILL); //удаляем элемент посылая сигнал процессу
                pids.pop_back();                      //удалить последний элемент
            }
            else
            {
                cout << "!!!\t\tYou doesn't have process" << endl;
            }
        }
        break;
        case 'q': //если был нажат q
        {
            for (int i = 0; i < pids.size(); i++)
            {
                kill(pids[i], SIGKILL); // удаляем все элементы посылая сигнал процессу
            }
            pids.clear(); //очистка вектора
        }
        break;
        }
        cout << "Number of the processes: " << pids.size() << "\n\r"
             << "\n\r";
        for (int i = 0; i < pids.size(); i++)
        {
            kill(pids[i], SIGUSR1);   // посылка сигнала процессу
            sigwait(&set, &reciever); //приостанавливает
                                      //работу потока и ждёт сигнала
        }
    }
    return 0;
}
