#include "Header.h"

int main(int argc, char *argv[])
{

    char *shm;
    key_t key;
    int pid;
    string choice = " ";
    key = shmget(UNIQENUMBER, SHMSIZE, 0666 | IPC_CREAT); // инициализация sharedMemory
	//0666 устанавливает права доступа для сегмента памяти, 
	//а IPC_CREAT говорит системе создать новый сегмент памяти для разделяемой памяти.
    if ((shm = (char *)shmat(key, (void *)0, 0)) == (char *)(-1))
    { // подключение sharedMemory к адресному пространству вызывающего процесса
        printf("Can't attach shared memory\n");
        exit(-1); // выводим сообщение и выходим из программы, если не удалось подключиться
    }
    char *s = shm; // выставляем указатель на sharedMemory
    while (true)
    {
        fflush(stdin);
        string str;
        printf("Enter string [PID: %d]: ", getpid());
        getline(cin, str);      // записываем строку в буфер
        strcpy(s, str.c_str()); // копируем из буфера в sharedMemory
        pid = fork();           // создаем новый процесс
        switch (pid)
        {
        case 0:
        {
            client(shm, key);
            printf("\nExit? (1-yes; 0-continue)\n");
            cin >> choice;
            while (true)
            {
                if (choice == "0" || choice == "1")
                    break;
                else
                {
                    cout << "Error! Try again: ";
                    cin >> choice;
                }
            }
            if (choice == "1")
            {
                kill(pid, SIGTERM); // поылаем сигнал для запроса завершения процесса.
                exit(0);            // завершаем серверный процесс
            }
            kill(getpid(), SIGTERM); // завершение дочернего процесса
            break;
        }
        case -1:
        { // не получилось создать процесс
            perror("ERROR");
            exit(0);
        }
        default:
        {
            wait(NULL); // приостанавливает  выполнение  текущего  процесса  до тех пор, пока дочерний процесс не прекратит выполнение.
                        //Если дочерний процесс к моменту вызова функции уже завершился (так  называемый  "зомби"),  то  функция  немедленно  возвращается.
                        //Системные ресурсы, связанные с дочерним процессом, освобождаются
            break;
        }
        }
    }
    return 0;
}

void client(char *sharedMemory, int key)
{
    key = shmget(UNIQENUMBER, SHMSIZE, 0);   // получение интендифекатора на sharedMemory
    sharedMemory = (char *)shmat(key, (void *)0, 0);     // получение указателя на sharedMemory
    printf("Child wrote [PID: %d][PPID: %d]: %s\n", getpid(), getppid(), sharedMemory); // вывод сообщения, которое записал пользователь в клентском процессе
    shmdt(sharedMemory);                                   // отключение сегмента sharedMemory
}