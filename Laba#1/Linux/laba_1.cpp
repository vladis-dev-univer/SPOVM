#include<iostream>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<ctime>

int main(){
	using namespace std;
	pid_t processId = fork();
	int status;
	if(processId < 0) {
		cout<<"Failed to Create a new Process"<<endl;
		exit(1);
	} else if (processId == 0) {
		cout<<"\nCHILD: Start process child\n";
		cout<<"\tCHILD: My PID = "<<getpid()<<endl;
		cout<<"\tCHILD: PID my parent = "<<getpid()<<endl;
		time_t now = time(0);
		char *dt = ctime(&now);
		cout<<"CHILD: End of the process child -->Time and data: "<<dt;
		getchar();
		exit(0);
	} else if(processId > 0) {
		cout<<"PARENT: start process parent\n";
		cout<<"\tPARENT: My PID = "<<getpid()<<endl;
		cout<<"\tPARENT: PID of my child = "<<processId<<endl;
		wait(&status);
		time_t now = time(0);
		char *dt = ctime(&now);
		cout<<"PARENT: End of the process parent -->Time and data: "<<dt;
		getchar();
		exit(0);
	}
	return 0;
}

