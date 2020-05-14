#include "head.h"

int w = 0;                          //логический указатель для записи (1 - пишет, 0 - ожидает)
int r = 0;                          //логический указатель для чтения (1 - читает, 0 - ожидает)
int size = 0;                       //счетчик прочитанных файлов
 
struct Data {
	pthread_t pThread[2];           //создаем массив из для хранения дескрипторов 2-ух потоков
	struct Array *arr;              //данное поле хранит имя файла
	
	int semID;                      //переменная для хранения дескриптора семафора
	struct sembuf semBuffRead;      //определяет, какие именно будут выполнятьс над семафором чтения
	struct sembuf semBuffWrite;     //определяет, какие именно будут выполнятьс над семафором записи
};

struct Array {                      //стрктура для хранения имени файла
	char fileName[100];
};

struct Data* createInfoStruct() {   //функция для создания структуры Data
	return malloc(sizeof(struct Data));	
}


typedef void (*ReadFromFile)(int, char*);   //объявяем указатель на функцию
typedef void (*WriteInFile)(int, char*);    //объявляем указатель на функцию

void Search(struct Data *data) {    //функция поиска файла в директории
	DIR *dir;                       //указатель на директорию
    struct dirent *entry;           //находим первы файл. Информацию о нём сохраняем в структуре
	
    dir = opendir("./Files/");      //получаем указатель на директорию

    if (!dir) {                     //удалось ли открыть?
        perror("diropen");          //если нет, завершаем работу программы
        exit(1);
    };
  
    while ((entry = readdir(dir)) != NULL) {    //считываем файлы из директории
        if (!strcmp(entry->d_name, "All.txt")) {//если дошли до результирующего файла, то пропускаем его
                continue;
        } if (!strcmp(entry->d_name, "..")) {   //если дошли до родительского каталога, то пропускаем его
            continue;
        } if (!strcmp(entry->d_name, ".")) {    //если дошли до текущего каталога, то пропускаем его
            continue;
        } if (size == 0) {                      //если это первый файл
            data->arr = malloc(sizeof(struct Array));   //выделяем под него память
        } else {
            data->arr = realloc(data->arr, (size + 1) * sizeof(struct Array)); //перевыделяем память для добавления нового файла в массив
        }
        strcpy(data->arr[size].fileName, "./Files/");   //формируем относительный путь к файлу
        strcat(data->arr[size].fileName, entry->d_name);    //записываем имя + путь файла
        size++;
    }   //продолжаем поиск файлов

    size--;
    closedir(dir);                              //закрыли директорию
		
}


void* ThreadReader(void* fdata) {
		
	struct Data *data = (struct Data*)fdata;
	char *error;
	int op1;
	
	ReadFromFile readFromFile = (void (*)())(intptr_t)dlsym(library_handler, "ReadFromFile");   //из dll выгружаем функцию чтения из файла
	
    if ((error = dlerror()) != NULL) {          //если возникли ошибки при попытке получения описателя dll, то
        fprintf (stderr, "%s\n", error);        //выводим в stderr сообщение с ошибкой
        exit(1);                                //завершаем программу
    }

	while (1) {
		if (size == -1) {                       //если нечего читать, то выходим из цикла
			break;
		}

		op1 = open(data->arr[size].fileName, O_RDONLY);     //получаем описать файла с разрешением только на чтение
		(readFromFile)(op1, buffer);                        //начинаем читеть в буфер при помощи функции ReadFromFile
		printf("Reader: Content to'%s'\n",data->arr[size].fileName);    //уведомляем пользовтеля о том, что мы читаем
		w = 1;                                  //разрешаем запись в файл All.txt
		while (r != 1);                         //ожидаем разрешения, чтобы продолжить чтение
		r = 0;                                  //после получения выставляем флаг в 0
		close(op1);                             //зарываем файл по описателю
	}
	
	return 0;
}

void* ThreadWriter(void* fdata) {
	struct Data *data = (struct Data*)fdata;
	char *error;
	
	WriteInFile writeInFile = (void (*)())(intptr_t)dlsym(library_handler, "WriteInFile");  //из dll выгружаем функцию записи в файл

    if ((error = dlerror()) != NULL) {          //если возникли ошибки при попытке получения описателя dll, то
        fprintf (stderr, "%s\n", error);        //выводим в stderr сообщение с ошибкой
        exit(1);                                //завершаем программу
    }

    int op2;                                    //объявляем дескриптор файла
	remove("./Files/All.txt");                  //удаляем файл, если таковой имеется
	op2 = open("./Files/All.txt", O_WRONLY | O_CREAT);      //открываем/создаем файл с доступом только к записи

	do {
	    while (w != 1);                         //ожидаем разрешение на запись
		w = 0;                                  //сбрасываем флаг на доступ к записи
		(writeInFile)(op2, buffer);             //получаем функцию
		printf("Writer: Content to '%s'\n",data->arr[size].fileName);   //уведомляем пользовтеля о том, что мы пишем
		size--;
		r = 1;                                  //разрешаем чтение

	} while (size != -1);                       //пока файлы не закончились
	close(op2);                                 //закрываем файл записи по описателю
	return 0;
}


void CreateThreads(struct Data *data) {         //ф-ия создания потоков
	library_handler = dlopen("./libj", RTLD_LAZY);  //получаем указатель на dll(загружаем)
	
	if (!library_handler) {                     //если dll не удалось открыть,
        fputs (dlerror(), stderr);              //то выводим сообщение об ошибке и
        exit(1);                                //выходим из программы
    }
	
	
	pthread_create(&(data->pThread[0]), NULL, &ThreadReader, data);     //создаем и сразу же запускаем поток чтения
	pthread_create(&(data->pThread[1]), NULL, &ThreadWriter, data);	    //создаем и сразу же запускаем поток записи
}



void CreateSem(struct Data *data) {                                     //ф-ия создания сем.
	int key = ftok("server.c", 's');  				//преобразование сущь. полное имя и целочисленный индикатор в значение типа key_t (ключ IPC)			
	data->semID = semget(key, 2, 0666 | IPC_CREAT); //возвращает идентификатор набора семафоров, связанный с аргументом key
	semctl(data->semID, 0, SETVAL, (int)0);         //устанавливаем значения семафоров
	semctl(data->semID, 1, SETVAL, (int)0);         //устанавливаем значения семафоров
	data->semBuffRead.sem_num = 0;                  //настраимваем семафор для записи
	data->semBuffRead.sem_op = 1;
	data->semBuffWrite.sem_num = 1;                 //настраиваем семафор для чтения
	data->semBuffWrite.sem_op = 1;
}

void CloseApp(struct Data *data) {                  //ф-ия зафершения работы
	pthread_join( data->pThread[0], NULL);          //закрывем поток чтения
    pthread_join( data->pThread[1], NULL);          //закрываем поток записи

	
	printf("End\n");                                //уведомляем пользователя
	dlclose(library_handler);                       //закрывем dll
}