#include "head.h"

int main(void) {
	struct Data *data;
	data = createInfoStruct();
	
    Search(data);
	printf("Waiting...\n");

	CreateSem(data);
	CreateThreads(data);
	CloseApp(data);

	return 0;
}	

	
