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
	HANDLE work = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "work");
	HANDLE close = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "close");
	HANDLE fileProjection = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "fileProjection");

	LPVOID pointer = MapViewOfFile(fileProjection, FILE_MAP_ALL_ACCESS, 0, 0, 0);

	char *stringRepresentation = static_cast<char*>(pointer);

	while (true) {
		WaitForSingleObject(work, INFINITE);
		if (WaitForSingleObject(close, constants::kDelay) == WAIT_OBJECT_0) {
			CloseHandle(work);
			CloseHandle(close);
			UnmapViewOfFile(pointer);
			CloseHandle(fileProjection);
			return;
		}

		cout << "Client got: " << stringRepresentation << endl;

		ReleaseSemaphore(work, 1, nullptr);
	}
}

void Server(char* input) {
	HANDLE work = CreateSemaphore(NULL, 0, 1, "work");

	HANDLE close = CreateSemaphore(NULL, 0, 1, "close");

	HANDLE fileProjection = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE,
		constants::kDwMaximumSizeLow, constants::kDwMaximumSizeHigh, "fileProjection");

	STARTUPINFO si;
	ZeroMemory(&si, sizeof si);
	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof pi);

	if (!CreateProcess(input, (LPSTR)"child process", NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
		cout << "Create Process failed" << GetLastError() << endl;
		system("pause");
		exit(EXIT_FAILURE);
	}

	LPVOID memoryMap = MapViewOfFile(fileProjection, FILE_MAP_ALL_ACCESS, 0, 0, 0);

	char *stringRepresentation = static_cast<char*>(memoryMap);

	int currentPosition = 0;
	bool readyForInput = true;
	string buffString;
	buffString.resize(constants::kMaxSize, '\0');
	string choice = " ";
	while (true) {
		if (readyForInput) {
			currentPosition = 0;
			cout << "Server: Please, enter the string" << endl;
			getline(cin, buffString);
			readyForInput = false;
		}

		string tempBuff;
		int newLength = 0;
		tempBuff.append(buffString, 0, constants::kMaxSize - 1);
		currentPosition = tempBuff.length();

		strcpy(stringRepresentation, const_cast<char*>(tempBuff.c_str()));
		stringRepresentation[tempBuff.length()] = '\0';

		tempBuff.clear();
		newLength = buffString.length() - currentPosition;
		if (newLength > 0) {
			tempBuff.append(buffString, currentPosition, newLength);
		}
		buffString = tempBuff;

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

			buffString.clear();

			cin.clear();
			cin.ignore(32767, '\n');
			cout << endl;
		}
	}
}