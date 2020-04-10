#include <windows.h>
#include <conio.h>
#include <iostream>
#define _CRT_SECURE_NO_WARNINGS
#define CREATE_NEW_PROCESS '+'
#define DESTROY_LAST_PROCESS '-' 
#define QUIT 'q'
#define MAX_COUNT 10

//Количество наших процессов
char s_numbers[][MAX_COUNT] = { 
	{"One"}, {"Two"}, {"Three"}, {"Four"}, {"Five"}, {"Six"}, {"Seven"}, {"Eight"},
	{"Nine"}, {"Ten"}
};

using namespace std;
PROCESS_INFORMATION CreateNewProcess(char* patch, char *commandline);//создание процесса
void Managment(char *patch); //определение работы
void PrintNumbers(int ProccessNumber); //вывод наших процессов на экран в бесконечном цикле

int main(int argc, char* argv[])
{
	if (argc == 2)//если создан не один процесс(т.е. создался новый процесс)
	{
		PrintNumbers(atoi(argv[1]));
	}
	else //если не созданно ни одного процесса (управление созданием, удалением процессов и выходом)
		Managment(argv[0]);
	return 0;
}

void Managment(char *patch)
{
	int activ = 0; //rjk-dj frnbdys[ ghjwtccjd
	HANDLE hCanWriteEvent = CreateEvent(NULL, FALSE, TRUE, "CanWriteProcess");//создаем дискриптор hCanWriteEvent. Инициализируем(создаём) его объектом событий(т.е. создаём событие)
																			 //(атрибут защиты по умолч., тип сброса - автоматический, сигнальное состояние, имя)
	char buf[10];
	char ch = 'x';
	PROCESS_INFORMATION mas[MAX_COUNT]; //создаём массив структур mas на 10 элементов, который содержит информацию о процессе и его основном потоке
	HANDLE hCanClose[MAX_COUNT]; //создаем массив дискрипторов hCanClose на 10 элементов 
	cout << "Enter '+', '-' or 'q':\n";
	while (ch = _getch())//пока мы ввели символ
	{
		if (ch == QUIT) break; //если симвом - q, то выходим и удаляем все процессы
		if (ch == DESTROY_LAST_PROCESS) //если симвом - -, то удаляем последний процесс
			if (activ)
			{
				WaitForSingleObject(hCanWriteEvent, INFINITE); //ждём пока объект hCanWriteEvent не окажется в сигнальном(отмеченном) состоянии
				SetEvent(hCanClose[activ]);//устанавливаем событие для объекта hCanClose на сигнальное состояние
				SetEvent(hCanWriteEvent);//устанавливаем событие для объекта hCanWriteEvent на сигнальное состояние
				activ--;//уменьшаем размер
			}
		if ((ch == CREATE_NEW_PROCESS) && (activ <= MAX_COUNT)) //если символ - + и не превышено макисмальное кол-во процессов, то создаем новый процесс
		{
			activ++;//увеличиваем размер
			sprintf_s(buf, " %d", activ);//записываем в буфер номер процесса
			hCanClose[activ] = CreateEvent(NULL, FALSE, FALSE, buf); //Инициаллизируем элемент массива hCanClose и создаем событие
			mas[activ] = CreateNewProcess(patch, buf);//создаём новый процесс (см. строку 74)
		}
		Sleep(100);
	}
	if (activ) //выходит из программы
	{
		while (activ > 0)
		{
			WaitForSingleObject(hCanWriteEvent, INFINITE);//ждём пока объект hCanWriteEvent не окажется в сигнальном(отмеченном) состоянии
			SetEvent(hCanClose[activ]);//устанавливаем событие для объекта hCanClose на сигнальное состояние
			SetEvent(hCanWriteEvent);//устанавливаем событие для объекта hCanWriteEvent на сигнальное состояние
			activ--;//уменьшаем размер
		}
	}
	system("pause");
}

PROCESS_INFORMATION CreateNewProcess(char* patch, char *commandline)
{
	STARTUPINFO cif; //создаём структуру для определения сфойств главного окна
	ZeroMemory(&cif, sizeof(STARTUPINFO));
	PROCESS_INFORMATION pi; //создаём структуру с информацией о процессе и его основном потоке
	CreateProcess(patch, commandline, NULL, NULL, TRUE, NULL, NULL, NULL, &cif, &pi); //создаем процесс
	return pi;
}
void PrintNumbers(int num_proccess)
{
	HANDLE hCanWriteEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, "CanWriteProcess"); //создаем дискриптор hCanWriteEvent. С помощью OpenEvent получаем доступ к ранее созданному событию
	char buf[30];
	sprintf_s(buf, " %d", num_proccess); //записываем в буфер номер нашего процесса
	HANDLE hCanClose = OpenEvent(EVENT_ALL_ACCESS, FALSE, buf); //создаем дискриптор hCanClose. С помощью OpenEvent получаем доступ к ранее созданному событию

	while (true)
	{
		if (WaitForSingleObject(hCanWriteEvent, INFINITE) == WAIT_OBJECT_0) //ждём, когда состояние объекта hCanWriteEvent станет сигнальным(отмеченным)
		{
			if (WaitForSingleObject(hCanClose, 100) == WAIT_OBJECT_0) //ждём 100 мс, когда состояние объекта hCanClose станет сигнальным(отмеченным)
			{
				SetEvent(hCanWriteEvent); //перевод в сигнальное состояние
				return;
			}
			for (int i = 0; i < strlen(s_numbers[num_proccess - 1]); i++)
			{
				
				printf("%c", s_numbers[num_proccess - 1][i]);//выводим посимвольно наш новый процесс
				Sleep(200);
			}
			printf(" \n|-> ");
			SetEvent(hCanWriteEvent); //перевод в сигнальное состояние
		}
	}
	return;
}