//Синхронизация с помощью критической секции
#include<Windows.h>
#include<iostream>
#include <conio.h>

#define MAX_COUNT 15
using namespace std;
CRITICAL_SECTION criticalSection;
size_t id = 0;

DWORD WINAPI print(LPVOID LParam) {
	cout << "\nCreate thread: " << GetCurrentThreadId() << endl; // выводим сообщение о создании нового процесса
	while (id <= MAX_COUNT && id != NULL) {
		EnterCriticalSection(&criticalSection); // вход в критическую секцию
		cout << "THREAD " << GetCurrentThreadId() << endl; // вывод сообщения
		LeaveCriticalSection(&criticalSection); //выход из секции
		WaitForSingleObject(GetCurrentThread(), 1000); // если передать сюда Handle потока,
		//то функция остановит выполнения того потока, в рамках которого она запущена, до завершения работы того потока,
		//Handle которого ей передали - но не дольше указанного временного интервала.
	}
	ExitThread(0);
}

int main() {
	InitializeCriticalSection(&criticalSection);
	HANDLE threads[MAX_COUNT];
	char userChoice;
	cout << "Enter symbol('+', '-' or 'q')\n";
	while (true) {
		if (id == 0) cout << "-> ";
		userChoice = _getch();
		if (userChoice == 'q') {
			for (size_t i = 0; i < id; i++) {
				TerminateThread(threads[id - 1], 0); // удаляем все потоки
			}
			DeleteCriticalSection(&criticalSection); //убераем критическую секцию
			id = NULL;
			break;
		}
		else if (userChoice == '+') {
			if (id < MAX_COUNT) {
				threads[id] = CreateThread(nullptr, //может ли создаваемый поток быть унаследован дочерним процессом 
					0, //значение по умолчанию (1МБ)
					print, //адрес функции, которая будет выполняться потоком
					nullptr, //указатель на переменную, которая будет передана в поток
					CREATE_SUSPENDED, //флаги создания --- поток не активен
					nullptr); //указатель на переменную, куда будет сохранен индентификатор потока
				ResumeThread(threads[id++]); // запустить созданный поток
			}
		}
		else if (userChoice == '-') {
			if (id > 0) {
				cout << "The thread will be destroyed (" << GetThreadId(threads[--id]) << ")" << endl; //выводим сообщение о том, что поток будет удален
				TerminateThread(threads[id], 0); // удаляем поток
			}
			else {
				cout << "No threads" << endl; //если нет потоков, выводим сообщение
			}
		}
	}
	cout << "\n\tEnd of program!\n";
	system("pause");
	return 0;
}