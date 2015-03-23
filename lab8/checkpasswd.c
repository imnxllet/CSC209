#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/* Read a user id and password from standard input, 
   - create a new process to run the validate program
   - use exec (probably execlp) to load the validate program.
   - send 'validate' the user id and password on a pipe, 
   - print a message 
        "Password verified" if the user id and password matched, 
        "Invalid password", or 
        "No such user"
     depending on the return value of 'validate'.

Setting the character arrays to have a capacity of 256 when we are only
expecting to get 10 bytes in each is a cheesy way of preventing most
overflow problems.
*/

#define MAXLINE 256
#define MAXPASSWD 10

void strip(char *str, int capacity) {
    char *ptr;
    if((ptr = strchr(str, '\n')) == NULL) {
        str[capacity - 1] = '\0';
    } else {
        *ptr = '\0';
    }
}


int main(void) {
    char userid[MAXLINE];
    char password[MAXLINE];

    /* Read a user id and password from stdin */
    printf("User id:\n");
    if((fgets(userid, MAXLINE, stdin)) == NULL) {
        fprintf(stderr, "Could not read from stdin\n"); 
        exit(1);
    }
    strip(userid, MAXPASSWD);

    printf("Password:\n");
    if((fgets(password, MAXLINE, stdin)) == NULL) {
        fprintf(stderr, "Could not read from stdin\n"); 
        exit(1);
    }
    strip(password, MAXPASSWD);

    /*Your code here*/
    int fd[2],status,r,result;
    if ((result = pipe(fd)) == -1){
        perror("pipe");
        exit(1);
    }
    if ((r = fork()) == -1){
        perror("fork");
        exit(1);
    }
    else if (r>1){  //parent process
        close(fd[0]);  // 0:read 1:write
        write(fd[1],userid,MAXPASSWD);   
        write(fd[1],password,MAXPASSWD);
        close(fd[1]);
        if (wait(&status) != -1){
            if (WIFEXITED(status)){
                if(WEXITSTATUS(status) == 0)
                    printf("Password verified\n");
                else if(WEXITSTATUS(status) == 2) 
                    printf("Invalid password\n");
                else if (WEXITSTATUS(status) == 3)
                    printf("No such user\n");
            }
        }
    }
    else{ //child process
        dup2(fd[0],fileno(stdin));
        close(fd[1]);
        close(fd[0]);
        execlp("./validate","validate", (char *) 0);
    }
    
    return 0;
}
