#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>

static int isEqualTo(char* argv, char* token) {
    if(token == "NULL" || argv == NULL || token == NULL)
        return 1;

    if(token != NULL){
        return strcmp(argv, token) == 0;
    }

	return strcmp(argv, "&&") == 0 || strcmp(argv, "||") == 0 || strcmp(argv, "|") == 0;
}

static void pipeline(char ***cmd)
{
	int fd[2];
	pid_t pid;
	int fdd = 0;				/* Backup */

	while (*cmd != NULL) {
		pipe(fd);				/* Sharing bidiflow */
		if ((pid = fork()) == -1) {
			perror("fork");
			exit(1);
		}
		else if (pid == 0) {
			dup2(fdd, 0);
			if (*(cmd + 1) != NULL) {
				dup2(fd[1], 1);
			}
			close(fd[0]);
			execvp((*cmd)[0], *cmd);
			exit(1);
		}
		else {
			wait(NULL); 		/* Collect childs */
			close(fd[1]);
			fdd = fd[0];
			cmd++;
		}
	}
}

static void logic_operation(char *** cmd){
    int returnValue = 0;
    while (*cmd != NULL) {
        pid_t pid = fork();
		if (pid == 0) {
                    printf("%s %d\n",(*cmd)[0], returnValue);
        if(
            (isEqualTo((*cmd)[0], "&&") && returnValue == 0)
            ||(isEqualTo((*cmd)[0], "||") && returnValue != 0)
        ){
            exit(1);
            return;
        }
			execvp((*cmd)[0], *cmd);
			exit(1);
		} else {
			waitpid(pid, &returnValue, 0);
			cmd++;
		}
	}
}

int main(int argc, char *argv[])
{
	//char *c1[] = {"ls", "-la", NULL};
	//char *c2[] = {"grep", "demo.c", NULL};
	//char *c3[] = {"grep", "demo", NULL};
	//char **cmd[] = {c1, c2, c3, NULL};
	//pipeline(cmd);

	char *c1[] = {"echo", "teste", NULL};
    char *o1[] = {"&&"};
	char *c2[] = {"echo", "teste2", NULL};
    char *o2[] = {"||"};
	char *c3[] = {"echo", "teste3", NULL};
	char **cmd[] = {c1, o1 , c2, o2, c3, NULL};
    logic_operation(cmd);

	return (0);
}