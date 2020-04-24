#include "Header.h"

int main(int argc, char* argv[]) {

	if (argc > 1) {
		Client();
	} else {
		Server(argv[0]);
	}
	return 0;
}

void Client() {
	//открываем уже созданные семафоры.
	HANDLE work = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "work");
	HANDLE close = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "close");
	HANDLE fileProjection = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "fileProjection");
	//создает специальный объект, «файловую проекцию», выделяя область памяти, которая связывается с определенным файлом (fileProjection) 
	//и в дальнейшем может быть доступна глобально из других процессов.
	LPVOID pointer = MapViewOfFile(fileProjection, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	//берём из "pointer" сторку введённую в сервере
	char *stringRepresentation = static_cast<char*>(pointer);
	while (true) {
		WaitForSingleObject(work, INFINITE);
		//Ожидание, пока указанный объект не окажется в сигнальном состоянии или пока не истечет время ожидания.WAIT_OBJECT_0 - объекта сигнализируется.
		if (WaitForSingleObject(close, constants::kDelay) == WAIT_OBJECT_0) {
			CloseHandle(work);
			CloseHandle(close);
			UnmapViewOfFile(pointer); //отменяет отображение  представления файла из адресного пространства вызывающего процесса
			CloseHandle(fileProjection);
			return;
		}
		cout << "Client got: " << stringRepresentation << endl;
		//Изменение счётчика.
		ReleaseSemaphore(work, 1, nullptr);
	}
}

void Server(char* input) {
	//создание семафоров
	HANDLE work = CreateSemaphore(NULL, 0, 1, "work");
	HANDLE close = CreateSemaphore(NULL, 0, 1, "close");
	//создает специальный объект, «файловую проекцию», выделяя область памяти, которая связывается с определенным файлом 
	//и в дальнейшем может быть доступна глобально из других процессов.
	HANDLE fileProjection = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE,
		constants::kDwMaximumSizeLow, constants::kDwMaximumSizeHigh, "fileProjection");

	//создание процесса:
	STARTUPINFO si;
	ZeroMemory(&si, sizeof si);
	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof pi);

	if (!CreateProcess(input, (LPSTR)"child process", NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
		cout << "Create Process failed" << GetLastError() << endl;
		system("pause");
		exit(EXIT_FAILURE);
	}
	//процесс создан

	// для получеия адреса в памяти файлового отображения, которое было создано выше, и записать туда передаваемые данные,
	// надо создать объект "memoryMap" вызвать функцию "MapViewOfFile" (возвращает (LPVOID) - начальный адрес отображаемого представления)
	LPVOID memoryMap = MapViewOfFile(fileProjection, FILE_MAP_ALL_ACCESS, 0, 0, 0);

	char *stringRepresentation = static_cast<char*>(memoryMap);
	int currentPosition = 0;
	bool readyForInput = true;	//флаг
	string buffString;
	buffString.resize(constants::kMaxSize, '\0');
	string choice = " ";
	//Вводим сообщение
	while (true) {
		if (readyForInput) {
			currentPosition = 0;
			cout << "Server: Please, enter the string" << endl;
			getline(cin, buffString);
			readyForInput = false; 
		}
		//записываем сообщение в "tempBuff"
		string tempBuff;
		int newLength = 0;
		tempBuff.append(buffString, 0, constants::kMaxSize - 1);
		currentPosition = tempBuff.length();

		//копируем в "stringRepresentation"
		strcpy(stringRepresentation, const_cast<char*>(tempBuff.c_str()));
		stringRepresentation[tempBuff.length()] = '\0';

		//очищаем буфер, затем продолжнаем или выходим из программы
		tempBuff.clear();
		newLength = buffString.length() - currentPosition;
		if (newLength > 0) {
			tempBuff.append(buffString, currentPosition, newLength);
		}
		buffString = tempBuff;

		//ReleaseSemaphore - изменение счётчика. При успешном выполнении возвращаемое значение ненулевое.
		ReleaseSemaphore(work, 1, NULL);
		WaitForSingleObject(work, INFINITE);


		if (buffString.empty()) {
			readyForInput = true;
			cout << "\nExit? (1 - yes; 0 - continue)" << endl;
			cin >> choice;

			while (true) {
				if (choice == "0" || choice == "1") break;
				else {
					cout << "Erorr! Try again: ";
					cin >> choice;
				}
			}

			//Если выход, то очищаем и закрываем все процессы и потоки, и выходим из программы
			if (choice == "1") {
				ReleaseSemaphore(close, 1, NULL);
				ReleaseSemaphore(work, 1, NULL);
				UnmapViewOfFile(memoryMap);			
				CloseHandle(pi.hThread);
				CloseHandle(pi.hProcess);
				CloseHandle(close);
				CloseHandle(work);
				CloseHandle(fileProjection);
				return;
			}
			
			//иначе, продолжаем
			buffString.clear();
			cin.clear();
			cin.ignore(32767, '\n');
			cout << endl;
		}
	}
}