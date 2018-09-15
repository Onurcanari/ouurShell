#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define TOK_DELIM " \t\r\n"
#define RED "\033[0;31m"
#define RESET "\e[0m"
#define TK_BUFF_SIZE 64
#define RL_BUFF_SIZE 1024

void loop();
char* read_line();
char** split_line();
int shell_exit();
int shell_execute(char** args);
int shell_launch(char** args);
int shell_cd(char** args);
int shell_help(char** args);
int shell_exit(char** args);

char* builtin_str[] = {"cd", "help", "exit"};
int (*builtin_func[])(char**) = {
    &shell_cd, 
    &shell_help, 
    &shell_exit
    };
int shell_num_builtins(){
    return sizeof(builtin_str) / sizeof(char*);
}
void loop(){
    char* line;
    char** args;
    int status = 1;

    do{
        printf("> ");
        line = read_line();
        args = split_line(line);
        status = shell_execute(args);
        free(line);
        free(args);
    }while(status);
}
char* read_line(){
    int buffsize = RL_BUFF_SIZE;
    int position = 0;
    char* buffer = malloc(sizeof(char) * buffsize);
    int c;
    //is buffer null or not
    if(!buffer){
        fprintf(stderr,"%souur: Allocation error%sn",RED,RESET);
        exit(EXIT_FAILURE);
    }
    while(1){
        // getchar() func uses keyboard buffer. we can use getline for this duty
        c = getchar();
        if(c == EOF || c == '\n'){
            buffer[position] = '\0';
            return buffer;
        }
        else{
            buffer[position] = c;
        }
        position++;

        if(position>=buffsize){
            buffsize += RL_BUFF_SIZE;
            buffer = realloc(buffer, buffsize);
            if(!buffer){
            fprintf(stderr,"ouur: Allocation error");
            exit(EXIT_FAILURE);
            }
        }
    }
}

char** split_line(char* line){
    int buffsize = 1024;
    int position = 0;
    char** tokens = malloc(buffsize * sizeof(char*));
    char* token;

    if(!tokens){
        fprintf(stderr,"%souur: Allocation error%sn",RED,RESET);
        exit(EXIT_FAILURE);
    }

    token = strtok(line, TOK_DELIM);
    while(token != NULL){
        tokens[position] = token;
        position++;

        if(position>=buffsize){
            buffsize += TK_BUFF_SIZE;
            tokens = realloc(tokens, buffsize * sizeof(char*));
            if(!token){
                fprintf(stderr,"%souur: Allocation error%sn",RED,RESET);
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}
int shell_execute(char** args){
    int i;
    if(args[0] == NULL) return 1;
    for(i=0;i<shell_num_builtins();i++){
        if(strcmp(args[0],builtin_str[i]) ==0){
            return (*builtin_func[i])(args);
        }
    }
    return shell_launch(args);
}
int shell_launch(char** args){
    pid_t pid, wpid;
    int status;

    pid = fork();

    if(pid == 0){
        // child process
        if(execvp(args[0], args) == -1){
            perror("ouur error");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0){
        // error forking
        perror("ouur");
    } else {
        // parent process
        do{
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status) );
    }
    return 1;
}

int shell_cd(char** args){
    if(args[1] == NULL)
        fprintf(stderr, "ouur: expected argument to \"cd\"\n");
    else{
        if(chdir(args[1]) != 0){
            perror("ouur");
        }
    }
    return 1;
}
int shell_help(char** args){
    int i;
    printf("Onurcan Ari's SHELL\n");
    printf("Type program names and argument, and hit enter.\n");
    for(i=0;i<shell_num_builtins();i++){
        printf("%s\t",builtin_str[i]);
    }
    printf("\n");
    return 1;
}
int shell_exit(char** args){
    return 0;
}
int main(){
    loop();
}