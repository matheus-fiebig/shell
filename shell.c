#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>

int returnValue = 0;
int commandsNumber=0;
char** commands = NULL;
char** copyArgv = NULL;
int pipes = 0;
int nargs= 0;

int is(char* argv, char* token) {
    if(token == "NULL")
        return 1;

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

int countNumberOfCommands(int argc){
    int numeroDeComandos = 1;
    for(int i = 0; i < argc-1; i++){
        if(is(commands[i],"&")) numeroDeComandos--;
        if(is(commands[i],NULL) == 1 || is(commands[i],"&") == 1){
            numeroDeComandos++;
            commands[i] = NULL;
        }
    }
    commands[argc - 1] = NULL;
    return numeroDeComandos;
}

char*** chainPipes(int actual, int next, char* op){
    int numberOfPipes = 0;
    while(is(op,"|")){
        numberOfPipes++;
        next = getNextCommand(next);
        op = next >= nargs ? "NULL": copyArgv[next];
    }

    char ***cmd = (char***) malloc(sizeof(char**)*numberOfPipes+2);
    for(int i = 0; i < numberOfPipes+2; i++){
        cmd[i] = (char**)malloc(sizeof(char*) * 20);
        for(int j = 0; j < 20; j++){
            cmd[i][j] = (char*) malloc(sizeof(char)*20);
        }
    }

    int j = 0, p = 0;
    for(p = 0; p < numberOfPipes+1; p++){
        int i = actual;
        while(commands[i] != NULL){
            cmd[p][i] = commands[j];
            i++;j++;
        }
        cmd[p][i] = NULL;
        j++;
    }

    cmd[p] = NULL;
    
    return cmd;
}

void execute(int commandIndex, char* op){   
    pid_t p = fork();
    if(p == 0){
        if(is(op,"&")) setpgid(0,0);
        execvp(commands[commandIndex], &commands[commandIndex]);
    }else if(p > 0 && !is(op,"&")){
        wait(&returnValue);
    }
}

void executePipe(char ***cmd)
{
	pid_t pid;
	int fd[2];
	int in = 0;
	while (*cmd != NULL) {
		pipe(fd);
        pid = fork();
		if (pid == -1) {
			exit(1);
		} else if (pid == 0) {
			dup2(in, 0);
			if (*(cmd + 1) != NULL) {
				dup2(fd[1], 1);
			}
			close(fd[0]);
			execvp((*cmd)[0], *cmd);
			exit(1);
		} else {
			wait(&returnValue);
			close(fd[1]);
			in = fd[0];
			cmd++;
		}
	}
}

void executeOperand(int actualCommandIndex, char* opBefore, char* opAfter, int nextCommandIndex){
    if(is(opAfter,"|")){
        char*** cmd = chainPipes(actualCommandIndex, nextCommandIndex, opAfter);
        executePipe(cmd);
        return;
    }

    if(is(opAfter,"&")){
        execute(actualCommandIndex,"&");
    }

    if( (is(opBefore,"||") && returnValue != 0) 
        || (is(opBefore,"&&") && returnValue == 0)
        || !is(opBefore, NULL)
    ){
        execute(actualCommandIndex,"");   
    }
}

int main(int argc, char** argv) {
    nargs = argc;
    copyArgv = createCopy(argc,argv);
	commands = &argv[1];
    commandsNumber = countNumberOfCommands(argc);
    
    int commandIndex = -1, nextCommandIndex = -1; 
    while(commandsNumber-- > 0){
        commandIndex = getNextCommand(commandIndex);       
        nextCommandIndex = getNextCommand(commandIndex);       
        char* opBefore = copyArgv[commandIndex];
        char* opAfter = nextCommandIndex >= argc ? "NULL": copyArgv[nextCommandIndex];
        executeOperand(commandIndex, opBefore, opAfter, nextCommandIndex);
    }

	return 0;
}

