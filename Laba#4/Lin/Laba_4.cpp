#include <stdlib.h>
#include <stdio.h>
#include <stack>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h> // для работы с терминалом
#include <pthread.h>

using namespace std;

struct ThreadInfo
{
    pthread_t thread; // дескриптор потока
};

struct MutexInfo
{
    pthread_mutex_t mutex; // мьютекс
};

void *printID(void *arg);
void init(struct MutexInfo *);
struct ThreadInfo createNewThread(struct MutexInfo *);
void deleteOneThread(struct ThreadInfo);
void deleteAll(stack<struct ThreadInfo> &s);
int _getch();

int main()
{
    stack<struct ThreadInfo> threads; // стэк с дескипторами потоков

    struct MutexInfo mInfo; // мьютекс
    init(&mInfo);           // инициализация мьютекса

    printf("Enter symbol ('+', '-' or 'q'):\n");
    while (true)
    {
        switch (_getch())
        {
        case '+':
        {
            printf("-> Added to top\n");
            threads.push(createNewThread(&mInfo));
            break;
        }
        case '-':
        {
            if (threads.size())
            {
                printf("-> Deleted first\n");
                deleteOneThread(threads.top());
                threads.pop();
            }
            break;
        }
        case 'q':
        {
            deleteAll(threads);
            return 0;
        }
        }
    }
    return 0;
}

int _getch()
{
    struct termios Old, New;
    char ch;
    tcgetattr(0, &Old);              // сохраняем текущее состояние терминала
    New = Old;                       // копируем его в struct termios New
    New.c_lflag &= ~(ICANON | ECHO); // устанавливаем флаги !(ICANON (Enable canonical mode) + ECHO (Echo input characters))
    tcsetattr(0, TCSANOW, &New);     // устанавливаем новый режим терминалу
    ch = getchar();                  // считываем символ без эхо и блокировки
    tcsetattr(0, TCSANOW, &Old);     // восстанавливаем к начальному состоянию терминал
    return ch;                       // возвращаем считанный символ
}

void *printID(void *arg)
{
    while (true)
    {
        pthread_mutex_lock((pthread_mutex_t *)arg);   // блокировка мьютекса
        printf("THREAD: %lu\n", pthread_self());      // вывод сообщения с индентификатором потока
        pthread_mutex_unlock((pthread_mutex_t *)arg); // разблокировка мьютекса
        usleep(1500000);                              // приостановка работы потока, в котором она была вызвана. Значение в микросекундах
    }
}

void deleteOneThread(struct ThreadInfo info)
{
    pthread_cancel(info.thread); // удаляем поток по дескриптору
}

struct ThreadInfo createNewThread(struct MutexInfo *mInfo)
{
    struct ThreadInfo info;
    pthread_create(&info.thread,           // адрес для хранения идентификатора потока
                   NULL,                   // атрибуды потока (NULL - по умолчанию)
                   printID,                // указатель на функцию
                   (void *)&mInfo->mutex); // аргументы, которые принимает функция
    return info;
}

void init(struct MutexInfo *info)
{
    pthread_mutex_init(&info->mutex, NULL); // инициализация мьютекса по умолчанию
}

void deleteAll(stack<struct ThreadInfo> &threads)
{
    while (threads.size())
    {                                         // пока стек не пуст (size != 0)
        pthread_cancel(threads.top().thread); // извлекаем из верхушки стека дескриптор у уничтожаем поток
        threads.pop();                        // удаляем верхушку стека
    }
}