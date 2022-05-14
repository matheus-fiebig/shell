#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>

int returnValue = 0;
int isBackground = 0;
int isOperand(char* argv) {
	if(strcmp(argv, "&&") == 0 || strcmp(argv, "||") == 0 || strcmp(argv, "|") == 0){
		return 1;
	
	return 0;
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

int runInBackground(char** commands, int actualCommandIndex, int argc){
    int i = actualCommandIndex;
    while (commands[i] != NULL && i < argc - 1){
        if(strcmp(commands[i], "&") == 0)
            return 1;
        i++;
    }
    return 0;
}

void execute(char** commands, int startIndex){
    pid_t p = fork();
    if(p == 0){
        execvp(commands[startIndex], &commands[startIndex]);
    }else if(p > 0 && isBackground == 0){
        wait(&returnValue);
    }
}


/*Implementar se possivel seu operador aqui*/
void executeOperand(char* op, char** commands, int nextCommandIndex){
    if(strcmp(op, "&&") == 0 && returnValue == 0){
        execute(commands, nextCommandIndex);
    }

    if(strcmp(op, "||") == 0 && returnValue != 0){
        execute(commands, nextCommandIndex);
    }

    if(strcmp(op, "&") == 0){
        execute(commands, nextCommandIndex);
    }
}

int main(int argc, char** argv) {
    char** copyArgv = createCopy(argc,argv);
	char** commands = &argv[1];

    int numeroDeComandos = 1;
    for(int i = 0; i < argc-1; i++){
        if(isOperand(commands[i]) == 1){
            commands[i] = NULL;
            numeroDeComandos++;
        }
        if(strcmp("&",commands[i]) == 0){
            commands[i] = NULL;
        }
    }
    commands[argc - 1] = NULL;

    int commandIndex = -1; 
    while(numeroDeComandos > 0){
        isBackground = 0;
        numeroDeComandos--;
        commandIndex = getNextCommand(commands,commandIndex);
        if(isOperand(copyArgv[commandIndex]) == 1){
            executeOperand(copyArgv[commandIndex], commands, commandIndex);
            continue;
        }
        
        if(runInBackground(copyArgv, commandIndex, argc) == 1){
            isBackground = 1;
            executeOperand("&", commands, commandIndex);
            continue;
        }
        
        execute(commands,commandIndex);
    }

	return 0;
}
