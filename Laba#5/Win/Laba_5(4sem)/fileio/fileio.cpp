// fileio.cpp : Определяет экспортированные функции для приложения DLL.
//
#include "pch.h"

#define NUM_OF_SYMBOLS 10
#define AMOUNT_OF_FILES 4 //кол-во фалов для чтения
#define STR_SIZE 64		//кол-во файлов
#define MAX_FILESIZE 65535	//максимальный размер

char file_str[MAX_FILESIZE]; // буфер файла
HANDLE event_output;
HANDLE event_input;
HANDLE event_close;


extern "C" __declspec(dllexport) void read() { //что функция read будет видна вне DLL
	//создаем события
	event_output = CreateEvent(nullptr, false, false, nullptr);//автомат. сброс, несигнальное сост.
	event_input = CreateEvent(nullptr, false, true, nullptr);//автомат. сброс, сигнальное сост.
	event_close = CreateEvent(nullptr, false, false, nullptr);//автомат. сброс, несигнальное сост.

	wchar_t files[AMOUNT_OF_FILES][STR_SIZE] = { // массив имен файлов для чтения
		{ L"t1.txt" },
		{ L"t2.txt" },
		{ L"t3.txt" },
		{ L"t4.txt" },
	};

	char buffer[NUM_OF_SYMBOLS + 1]; // объявляем буфер 
	DWORD bytes_in_file; // объявляем переменную, которая будет хранить кол-во прочитанных байт в файле
	OVERLAPPED overlapped; //содержит информацию, используемую в асинхронном вводе/выводе данных

	//Дескриптор события, которые должно быть установлено в сигнальное состояние, когда операция завершилась.
	overlapped.hEvent = CreateEvent(nullptr, false, true, nullptr);//автомат.сброс, сигнальное сост.

	//reading
	for (int i = 0; i < AMOUNT_OF_FILES; i++) {
		// ожидаем бесконечно долго сигнала, который разрешит запись в буфер
		WaitForSingleObject(event_input, INFINITE);
		overlapped.Offset = 0; // Местоположение файла, в котором начинается передача
		overlapped.OffsetHigh = 0; //  Этот член структуры используется только тогда, когда устройство - файл.
								   // В противном случае, этот член структуры должен быть нуль.
		strcpy_s(file_str, sizeof(file_str), ""); // очищаем file_str
		HANDLE handler_file = CreateFile(files[i], // имя файла
			GENERIC_READ, // открыть для чтения
			0, // не использовать Режим совместного доступа
			nullptr, // нет атрибутов защиты
			OPEN_EXISTING, // открыть существующий файл 
			0, // без аттрибутов
			nullptr); //] Дескриптор файла шаблона с правом доступа GENERIC_READ. Должен быть nullptr

		do {
			memset(buffer, '\0', sizeof(buffer)); // заполнить буфер символами '\0'
			if (ReadFile(handler_file, // дескриптор файла, который следует прочесть
				buffer, // Указатель на буфер, который принимает прочитанные данные из файла. 
				NUM_OF_SYMBOLS, // Число байтов, которые читаются из файла.
				&bytes_in_file, // Указатель на переменную, которая получает число прочитанных байтов
				&overlapped) == 0) { // Указатель на структуру OVERLAPPED
				break; // если функция вернула 0
			}
			WaitForSingleObject(overlapped.hEvent, INFINITE); //ожидаем прочтения данных из файла
			strcat_s(file_str, sizeof(file_str), buffer); // записываем прочитанное в буфер файла
			overlapped.Offset += bytes_in_file; // добавляем отступ, равный количеству прочтенных байт

		} while (bytes_in_file == NUM_OF_SYMBOLS);
		SetEvent(event_output); // утанавливаем событие, разрешающее запись итоговый в файл
		CloseHandle(handler_file); // закрываем описатель 
	}
	SetEvent(event_close); // устанавливаем событие, которое сигнализирует о завершении работы
	CloseHandle(overlapped.hEvent); // закрываем описатель 

}

extern "C" __declspec(dllexport) void write() { //что функция write будет видна вне DLL
	//create result file
	TCHAR file[STR_SIZE] = L"result.txt"; // имя итогового файла
	// открываем файл для записи, если таковой имеется
	HANDLE handler_file = CreateFile(file,
		GENERIC_WRITE,
		0,
		nullptr,
		OPEN_EXISTING,
		0,
		nullptr);
	DWORD bytes_in_file; // число записанных байтов
	OVERLAPPED overlapped; //содержит информацию, используемую в асинхронном вводе/выводе данных

	overlapped.hEvent = CreateEvent(nullptr, true, true, nullptr);
	overlapped.Offset = 0;
	overlapped.OffsetHigh = 0;

	while (true) {

		if (WaitForSingleObject(event_output, 0) == WAIT_OBJECT_0) { // ожидаем разрешения на запись

			WriteFile(handler_file, // описатель файла
				&file_str, // буфер данных, которые нужно записать 
				strlen(file_str), // число байтов для записи
				&bytes_in_file, // сколько реально байт записано 
				&overlapped);
			//ожидаем записи данных из файла
			WaitForSingleObject(overlapped.hEvent, INFINITE);
			overlapped.Offset += bytes_in_file; // добавляем отступ, равный количеству записанных байт
			SetEvent(event_input); // утанавливаем событие, разрешающее чтения файла в буфер
		}
		if (WaitForSingleObject(event_close, 0) == WAIT_OBJECT_0) { // ожидаем сигнала о завершении
			break;
		}
	}

	// закрываем описатели 
	CloseHandle(handler_file);
	CloseHandle(event_input);
	CloseHandle(event_output);
	CloseHandle(event_close);
}