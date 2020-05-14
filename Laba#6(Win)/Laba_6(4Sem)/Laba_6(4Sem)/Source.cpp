#include <windows.h>
#include <windowsx.h>
#include <iostream>

void* _allocate(int size);
void* _malloc(int size);
void  _free(void* ptr);
void* _realloc(void* ptr, int newSize);

int main() {
	char* tmp;
	tmp = (char*)_malloc(20);
	std::cout << "Local Size: " << LocalSize(tmp - sizeof(HLOCAL)) << std::endl; //LocalSize(текущий размер указанного объекта локальной памяти в байтах)
																		//Возвращаемое значение является размером указанного объекта локальной памяти в байтах
	std::cout << "Type size: " << sizeof(tmp) << std::endl;

	tmp = (char*)_realloc(tmp, 10);
	std::cout << "Local Size after reallocation memory: " << LocalSize(tmp - sizeof(HLOCAL)) << std::endl;
	std::cout << "Type size after reallocation memory: " << sizeof(tmp) << std::endl;
	_free(tmp);


	system("pause");
	return 0;
}

void* _allocate(int size) {
	HLOCAL hMemLocal = LocalAlloc(GHND,
		size + sizeof(HLOCAL));  // Выделяет из локальной кучи память под блок локальной памяти.
									   // Фактический pазмеp может быть больше, чем указанный.
									   // (GHND = GMEM_MOVEABLE | GMEM_ZEROINIT, Количество байтов для выделения)

	if (hMemLocal != nullptr) {	//если память выделилась
		void* pLocal = LocalLock(hMemLocal);	// Для получения доступа к полученному блоку памяти
												// его необходимо зафиксировать, функция возвращает указатель на первый байт блока памяти объекта.
		if (pLocal != nullptr) {	// Если память зафиксирована
			pLocal = hMemLocal;		//устанавливаем дескриптор памяти вначало
			pLocal = (char*)pLocal + sizeof(HLOCAL);// установка начала памяти на следующий после дескриптора байт
			return pLocal;
		}

		std::cout << std::endl << "Error in locking block";

	}
	else {
		std::cout << std::endl << "Not enough memory";
	}

	return nullptr;
}

void* _malloc(int size) {

	void* pLocal = _allocate(size); //получаем дескриптор памяти 

	if (pLocal == nullptr) {		//если выделить не получилось, то пробуем дефрагментировать свободное
									//пространство и выделить ее снова
		UINT uMaxFreeMem = LocalCompact(size); //Генеpиpует свободный блок pазмеpом не менее size. 
											   //Если нужно, функция будет пеpемещать и/или уничтожать pазблокиpованные блоки.
											   //Функция возвращает размер самого большого доступного
											   //непрерывного блока в локальной области памяти, причем,
											   //если параметр не равен 0 или -1, выполняется
											   //дефрагментация памяти и удаление блоков, отмеченных как
											   //удаляемые. Если параметр функции указан как 0, функция
											   //не выполняет дефрагментацию памяти, но возвращает
											   //правильное значение с учетом возможного выполнения
											   //дефрагментации. 
		std::cout << "Trying to defragmentate memory" << std::endl;
		pLocal = _allocate(size);	//повторяем выделение 

		if (pLocal == nullptr) {	//если выделить память не получилось, то уведомляем пользователя
			std::cout << "Not enough memory" << std::endl;
			return nullptr;
		}
	}

	return pLocal;
}

void  _free(void* ptr) {
	HLOCAL hMemLocal = (char*)ptr - sizeof(HLOCAL); // получение дескриптора памяти
	LocalUnlock(hMemLocal);	//расфиксируем блок памяти перед удалением. Функция не влияет на объекты памяти, выделенные с помощью LMEM_FIXED
	if (LocalFree(hMemLocal) != nullptr) { // Освобождает указанный объект локальной памяти и делает недействительным его дескриптор
		std::cout << std::endl << "Error in making memory _free";
	}
	else {	// возвращает NULL при успешном освобождении памяти
		std::cout << std::endl << "Memory is _free";
	}

	ptr = nullptr;
}

void* _realloc(void* ptr, int newSize) {

	HLOCAL hMemLoDiscard = (char*)ptr - sizeof(HLOCAL);	//получаем дескриптор памяти

	hMemLoDiscard = LocalReAlloc(hMemLoDiscard, newSize + sizeof(HLOCAL), NULL);	//перевыделяем память
//(Дескриптор объекта локальной памяти, который будет перераспределен, новый размер блока памяти в байтах, варианты перераспределения 

	if (hMemLoDiscard == nullptr) {	//если память не перевыделилась 
		std::cout << std::endl << "Error in locking block";
		return nullptr;
	}

	void* startMem = LocalLock(hMemLoDiscard); //заново фиксируем память, т.к. мы ее удалили,
												//изменили размер и восстановили

	if (startMem == nullptr) {			//если произошла ошибка, сообщаем это пользователю
		std::cout << std::endl << "Error in locking block";
		std::cout << std::endl << GetLastError();

	}

	startMem = hMemLoDiscard;

	return (char*)startMem + sizeof(HLOCAL); // возвращаем адрес начала памяти 
}