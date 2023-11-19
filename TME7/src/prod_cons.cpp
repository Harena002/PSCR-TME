#include "Stack.h"
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <vector>
#include <sys/mman.h>
#include <signal.h>
#define NBPROD 2
#define NBCONS 3

using namespace std;
using namespace pr;
bool doIt = true;
void hand_SIGINT(int n){
	//Why with valgrind, a Ctrl-C kills everyone whereas without valgrind I need no to do Ctrl-C twice
	cout<<"pid : "<<getpid()<<" received the signal"<<endl;
	doIt = false;
}

void set_handler(){
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = hand_SIGINT;
	sigaction(SIGINT,&sa,NULL);
}

void producteur (Stack<char> * stack) {
	char c ;
	while (doIt) {
		cin.get(c);
		//cout<<"before push"<<endl;
		stack->push(c);
		//cout<<"after push"<<endl;
		//cout<< "doit : "<<doIt<<endl;
	}
}

void consomateur (Stack<char> * stack) {
	while (doIt) {
		char c = stack->pop();
		cout << c <<endl <<flush ;
		cout<<"doit : "<<doIt<<endl;
	}
}

int main () {
	set_handler();
	//For his child, 'space' will be at same memory space (physically) as him when one of them attempt to use it
	//the page table of each process mapping that shared region will have entries pointing to the same physical pages
	void *space = mmap(0,sizeof(Stack<char>),PROT_READ|PROT_WRITE,MAP_ANONYMOUS | MAP_SHARED,-1,0);
	if(space == MAP_FAILED){
		perror("MMAP FAILED");
	}
	//To create the class into the shared space
	Stack<char> * s = new (space) Stack<char>();

	//Everyone has to munmap 
	for (int i = 0; i < NBPROD; i++){
            if (fork() == 0) {  
		producteur(s);
		
		if(munmap(space,sizeof(Stack<char>)) == -1){
		    perror("Munmap failed");
		}
		cout<<"here return prod : "<<i<<endl;
		return 0;
	     }
	}


        	

	for (int i = 0; i < NBCONS; i++){
	    if (fork()==0) {
	        consomateur(s);
		if(munmap(space,sizeof(Stack<char>)) == -1){
		    perror("Munmap failed");
		}
		cout<<"here return cons : "<<i<<endl;
		return 0;
	    }
	}
	
	for (int i = 0; i < NBPROD+NBCONS; i++){
		wait(0);
	}
	

	if(munmap(space,sizeof(Stack<char>)) == -1){
		perror("Munmap failed");
	}
	//clang++ -std=c++20 -o prod_cons prod_cons.cpp Stack.h -lrt 
	s->~Stack();
	return 0;
}

