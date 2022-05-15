#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>

int returnValue = 0;
int pipes = 0;
int pipeIterator = 0;
int* fds = NULL;
char** commands = NULL;
char** copyArgv = NULL;
pid_t* lateAwait = NULL;

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
            if(isOperand(commands[i], "|")) pipes++;
            commands[i] = NULL;
        }
    }
    commands[argc - 1] = NULL;
    return numeroDeComandos;
}

void openPipes(int numberOfPipes){
    for(int i = 0; i < numberOfPipes; i++){
        if(pipe(fds + i*2) == -1){
            perror("Erro de pipe");
            exit(EXIT_FAILURE);
        } 
    }
}

void closePipes(int numberOfPipes, int factor){
    for(int j = 0; j < 2*numberOfPipes-factor; j++){
        if(close(fds[j]) == -1){
            perror("Erro de close");
            exit(EXIT_FAILURE);
        }
    }
}

void configurePipes(char* op){
    /* if(!isOperand(op,"|")){
        return;
    } */
    
    if(pipes != 0){
        if(pipeIterator != 0){
            if(dup2(fds[2*pipeIterator - 2], STDIN_FILENO) == -1){
                perror("Erro de dup entrada");
                exit(EXIT_FAILURE);
            }
        }

        if(pipeIterator*2 != 2*pipes){
            if(dup2(fds[2*pipeIterator + 1], STDOUT_FILENO) == -1){
                perror("Erro de dup saida");
                exit(EXIT_FAILURE);
            }
        }
    }

    closePipes(pipes,1);
}

void execute(int startIndex, char* operator){
    pid_t p = fork();
    if(p == 0){
        configurePipes(operator);
        execvp(commands[startIndex], &commands[startIndex]);
    }
    
    if(!isOperand(operator,"&")) waitpid(0,&returnValue,0);
}

void executeOperand(char* op, int nextCommandIndex){
    if(isOperand(op,"&&") && returnValue == 0){
        execute(nextCommandIndex, op);
    }

    if(isOperand(op,"||") && returnValue != 0){
        execute(nextCommandIndex, op);
    }

    if(isOperand(op,"&")){
        execute(nextCommandIndex, op);
    }

    if(isOperand(op,"|")){
        pipeIterator++;
        execute(nextCommandIndex, op);
    }
}

int main(int argc, char** argv) {
    copyArgv = createCopy(argc,argv);
	commands = &argv[1];
    
    int numeroDeComandos = countNumberOfCommands(argc);
    fds = (int*)malloc(sizeof(int)*pipes*2);
    openPipes(pipes);
    
    int commandIndex = -1; 
    while(numeroDeComandos-- > 0){
        commandIndex = getNextCommand(commandIndex);
        if(isOperand(copyArgv[commandIndex],NULL)){
            executeOperand(copyArgv[commandIndex], commandIndex);
            continue;
        }
        
        if(isBackground(commandIndex, argc)){            
            executeOperand("&", commandIndex);
            continue;
        }
        
        execute(commandIndex, "NULL");
    }

    closePipes(pipes,0);
    for(int j = 0; j < pipes; j++) waitpid(0,NULL,0);
	return 0;
}

