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

int countPipes(char** argv, int argc){
    int cont = 0, i = 0;
    while(argv[i] != NULL && i < argc - 1){
        if(isOperand(argv[i], "|")) cont++;
        i++;
    }
    return cont;
}

void openPipes(int numberOfPipes){
    for(int i = 0; i < numberOfPipes; i++){
        if(pipe(fds + i*2) == -1){
            perror("Erro de pipe");
            exit(EXIT_FAILURE);
        } 
    }
}

void closePipes(int numberOfPipes, int fds[], int factor){
    for(int j = 0; j < 2*numberOfPipes-factor; j++){
        if(close(fds[j]) == -1){
            perror("Erro de close");
            exit(EXIT_FAILURE);
        }
    }
}

void configurePipe(char* op, pid_t p){
    if(pipes != 0){
        if(pipeIterator != 0){
            printf("Error: %d", pipeIterator);
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

    closePipes(pipes,fds,1);
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
        configurePipe(operation,p);
        execvp(commands[startIndex], &commands[startIndex]);
    }
    
    pipeIterator++;
}

void executeOperand(char* op, char** commands, int nextCommandIndex){
    if(strcmp(op, "&") == 0 || strcmp(op, "|") == 0){
        execute(commands, nextCommandIndex, op);
    }
}

int main(int argc, char** argv) {
    char** copyArgv = createCopy(argc,argv);
	char** commands = &argv[1];
    
    int numeroDeComandos = countNumberOfCommands(commands, argc);
    int commandIndex = -1; 

    pipes = countPipes(copyArgv, argc);
    fds = (int*)malloc(sizeof(int)*pipes*2);
    openPipes(pipes);

    while(numeroDeComandos-- > 0){
        commandIndex = getNextCommand(commands,commandIndex);
        if(isOperand(copyArgv[commandIndex],NULL)){
            executeOperand(copyArgv[commandIndex], commands, commandIndex);
            continue;
        }
        execute(commands,commandIndex, "");
    }

    closePipes(pipes,fds,0);
    for(int j = 0; j < pipes; j++) waitpid(0,NULL,0);
	
    return 0;
}

