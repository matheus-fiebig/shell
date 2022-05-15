#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>

//Conta o numero de pipes
int cont_pipes(char **); 
//Encotra a posicao dos pipes subsituindo-os por NULL. entao armazena o ponteiro para o primeiro argumento em um vetor.
void  le_comandos(char ***, char **); 


int main (int argc, char **argv){
    pid_t p;
    int num = cont_pipes(argv), i = 0, cont = 0;
    int fds[2*num];
    char **cmds[num+1]; //Vvetor de comandos. Deve possuir o numero de pipes + 1 para armazenar todos os comandos

    le_comandos(cmds, argv);

    //Abrindo todos os pipes
    for(i = 0; i < num; i++){
        if(pipe(fds + i*2) == -1){
            perror("Erro de pipe");
            exit(EXIT_FAILURE);
        } 
    }

    i = 0;

    //Enquanto i < que o numero de comandos.
    while(i < num+1){

        p = fork();
        if(p == 0){ //Filhos serao responsaveis por executar os comandos e liga-los.
           
            if(num != 0){
                //Se necessario abre duplica o descritor de entrada
                if(i != 0){
                    if(dup2(fds[2*i - 2], STDIN_FILENO) == -1){
                        perror("Erro de dup entrada");
                        exit(EXIT_FAILURE);
                    }
                }
                //Se necessario abre duplica o descritor de saida
                if(i*2 != 2*num)
                    if(dup2(fds[2*i + 1], STDOUT_FILENO) == -1){
                        perror("Erro de dup saida");
                        exit(EXIT_FAILURE);
                    }
            }

            int j;
            //Fecha os pipes
            for(j = 0; j < 2*num-1; j++){
                if(close(fds[j]) == -1){
                    perror("Erro de close");
                    exit(EXIT_FAILURE);
                }
            }

            //Executa o comando armazenado
            if(execvp(*cmds[i], cmds[i]) == -1){
                perror("Erro de exec");
                exit(EXIT_FAILURE);
            }    
        } 
        i++;
    }

    
    int j;

    //Fecha os todos os pipes
    for(j = 0; j < 2*num; j++){
        if(close(fds[j]) == -1){
            perror("Erro de closeB");
            exit(EXIT_FAILURE);
        }
    }

    //Espera que todos os filhos teminem a execucao
    for(j = 0; j < num; j++) waitpid(0, NULL, 0);

    return 0;

}

int cont_pipes(char **argv){

    int cont = 0, i = 0;

    //Compara o argumento em busca de um operador pipe para adicionar a contagem
    while(argv[i]){
        if(!strcmp(argv[i], "|")) cont++;
        i++;
    }

    //Retorna o numero de pipes encotrado
    return cont;
}

void le_comandos(char ***cmds, char **argv){

    int i = 0, j = 1;

    //Armazena o primeiro comando no vetor
    cmds[0] = &argv[1];

    while(argv[i]){
        //Quando encontrar um pipe o subistitui por NULL e armazena o proximo argumento no vetor.
        //A substituicao eh feita para que seja possivel utilizar o vetor na primitiva execvp
        if(!strcmp(argv[i], "|")){
            argv[i] = NULL;
            cmds[j] = &argv[i+1];
            j++;
        } 
        
        i++;
    }
}