#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>

int returnValue = 0;
char** commands = NULL;
char** copyArgv = NULL;

int isOperand(char* argv, char* token) {
    if(token != NULL){
        return strcmp(argv, token) == 0;
    }

	return strcmp(argv, "&&") == 0 || strcmp(argv, "||") == 0 || strcmp(argv, "|") == 0;
}

char** createCopy(int argc, char** argv) {
    char** newArray = (char**)malloc(sizeof(char*) * argc);
    for (int i = 0; i < argc; i++) {
        newArray[i] = (char*)malloc(sizeof(char) * strlen(argv[i]));
    }

    for (int i = 0; i < argc; i++) {
        strcpy(newArray[i], argv[i]);
    }

    return newArray;
}

int getNextCommand(int actualCommandIndex){
    if(actualCommandIndex == -1)
        return 0;
    int numberOfParameters = 1;
    int i = actualCommandIndex;
    while (commands[i] != NULL){
        numberOfParameters++;
        i++;
    }
    return numberOfParameters + actualCommandIndex;
}

int isBackground(int actualCommandIndex, int argc){
    int i = actualCommandIndex;
    while (copyArgv[i] != NULL && i < argc - 1){
        if(strcmp(copyArgv[i], "&") == 0)
            return 1;
        i++;
    }
    return 0;
}

int countNumberOfCommands(int argc){
    int numeroDeComandos = 1;
    for(int i = 0; i < argc-1; i++){
        if(isOperand(commands[i],NULL) == 1 || isOperand(commands[i],"&") == 1){
            if(!isOperand(commands[i],"&")) numeroDeComandos++;
            commands[i] = NULL;
        }
    }
    commands[argc - 1] = NULL;
    return numeroDeComandos;
}

void execute(int commandIndex, char* operator){   
    pid_t p = fork();
    if(p == 0){
        execvp(commands[commandIndex], &commands[commandIndex]);
    }else{
        waitpid(p,&returnValue,0);
    }
}

void executeOperand(char* op, int actualCommandIndex, int nextCommandIndex){
    if(
        (isOperand(op,"&&") && returnValue == 0) || 
        isOperand(op,"&")
      )
    {
        execute(actualCommandIndex, op);
        return;
    }

    execute(actualCommandIndex, op);
}

int main(int argc, char** argv) {
    copyArgv = createCopy(argc,argv);
	commands = &argv[1];
    
    int numeroDeComandos = countNumberOfCommands(argc);
    
    int commandIndex = -1, nextCommandIndex = -1; 
    while(numeroDeComandos-- > 0){
        commandIndex = getNextCommand(commandIndex);       
        nextCommandIndex = getNextCommand(commandIndex);
        int bg = isBackground(commandIndex, argc);
        executeOperand(bg ? "&" : copyArgv[commandIndex], commandIndex, nextCommandIndex);
    }

	return 0;
}

