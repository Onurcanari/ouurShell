#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>


#define TOK_DELIM " \t\r\n"
#define RED "\033[0;31m"
#define RESET "\e[0m"
#define TK_BUFF_SIZE 64

char *home_directory = "/home/onurcanari";
char current_directory[PATH_MAX];


// main functions
void loop();
char* read_line();
char** split_line();
int shell_exit();
int shell_execute(char** args);
int shell_launch(char** args);
int shell_cd(char** args);
int shell_help(char** args);
int shell_exit(char** args);
int shell_mkdir(char** args);
int shell_listdir(char** args);
int shell_pwd(char** args);

// utility functions
DIR* open_dir(char* directory);
void set_cwd();


char* builtin_str[] = {"cd", "help", "exit", "dir", "mkdir", "cwd"};
int (*builtin_func[])(char**) = {   
    &shell_cd, 
    &shell_help, 
    &shell_exit,
    &shell_listdir,
    &shell_mkdir,
    &shell_pwd
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
    char *line;
    size_t len=0;
    ssize_t readed_chars = getline(&line, &len, stdin);
    return line;
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


int shell_pwd(char** args){
    set_cwd();
    printf("%s\n",current_directory);
    return 1;
}

int shell_execute(char** args){ 
    int i;
    if(args[0] == NULL) return 1;
    for(i=0; i<shell_num_builtins(); i++){
        if(strcmp(args[0],builtin_str[i]) == 0){
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

int shell_listdir(char** args){
    DIR *dp;
    struct dirent *entry;
    dp = open_dir(".");
    while(entry = readdir(dp)){
        if(!strcmp(entry->d_name,".") || !strcmp(entry->d_name, ".."))
            continue;
        printf("%s\t",entry->d_name);
    }
    printf("\n");
    closedir(dp);
    return 1;
}
int shell_mkdir(char** args){
    if(args[1] == NULL || !strcmp(args[1],"")){
        perror("missing operand.\n");
        return 1;
    }
    set_cwd();
    char dir_path[PATH_MAX];
    memset(dir_path,0,sizeof(dir_path));
    strcpy(dir_path,current_directory);
    strncpy(dir_path+strlen(dir_path),"/",1);
    strncpy(dir_path+strlen(dir_path),args[1],strlen(args[1]));
    mkdir(dir_path,S_IRWXU|S_IRGRP|S_IXGRP);
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
    chdir(args[1]);
    shell_pwd(NULL);
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

DIR* open_dir(char* directory){
    DIR *dp;
    dp = opendir(directory);
    if(dp == NULL){
        perror("opened dir is null");
    }
    return dp;
}

void set_cwd(){
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
    strcpy(current_directory, cwd);
}