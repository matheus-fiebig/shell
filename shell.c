#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>

int returnValue = 0;

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

int getNextCommand(char** commands, int actualCommandIndex){
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

int isBackground(char** commands, int actualCommandIndex, int argc){
    int i = actualCommandIndex;
    while (commands[i] != NULL && i < argc - 1){
        if(strcmp(commands[i], "&") == 0)
            return 1;
        i++;
    }
    return 0;
}

int countNumberOfCommands(char** commands, int argc){
    int numeroDeComandos = 1;
    for(int i = 0; i < argc-1; i++){
        if(isOperand(commands[i],NULL) == 1 || isOperand(commands[i],"&") == 1){
            if(isOperand(commands[i],"&") == 0) numeroDeComandos++;
            commands[i] = NULL;
        }
    }
    commands[argc - 1] = NULL;
    return numeroDeComandos;
}

void execute(char** commands, int startIndex, char* operation){
    pid_t p = fork();
    if(p == 0){
        execvp(commands[startIndex], &commands[startIndex]);
    }else if(p > 0 && isOperand(operation, "&") == 0){
        wait(&returnValue);
    }
}

void executeOperand(char* op, char** commands, int nextCommandIndex){
    if(strcmp(op, "&&") == 0 && returnValue == 0){
        execute(commands, nextCommandIndex, op);
    }

    if(strcmp(op, "||") == 0 && returnValue != 0){
        execute(commands, nextCommandIndex, op);
    }

    if(strcmp(op, "&") == 0){
        execute(commands, nextCommandIndex, op);
    }

    if(strcmp(op, "|") == 0){
        execute(commands, nextCommandIndex, op);
    }
}

int main(int argc, char** argv) {
    char** copyArgv = createCopy(argc,argv);
	char** commands = &argv[1];
    
    int numeroDeComandos = countNumberOfCommands(commands, argc);
    int commandIndex = -1; 
    
    while(numeroDeComandos-- > 0){
        commandIndex = getNextCommand(commands,commandIndex);
        if(isOperand(copyArgv[commandIndex],NULL)){
            executeOperand(copyArgv[commandIndex], commands, commandIndex);
            continue;
        }
        
        if(isBackground(copyArgv, commandIndex, argc)){            
            executeOperand("&", commands, commandIndex);
            continue;
        }
        
        execute(commands,commandIndex, "");
    }

	return 0;
}

