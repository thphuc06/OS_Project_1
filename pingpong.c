#include "kernel/types.h"
#include "user/user.h"

int main() {
    int parentToChild[2], childToParent[2];
    pipe(parentToChild); // Cha -> Con
    pipe(childToParent); // Con -> Cha

    if (fork() == 0) {
        //tiến trình con
        close(parentToChild[1]); 
        close(childToParent[0]); 
        
        char buf;
        read(parentToChild[0], &buf, 1); 
        printf("%d: received ping\n", getpid());
        write(childToParent[1], &buf, 1); 
        
        close(parentToChild[0]);
        close(childToParent[1]);
        exit(0);
    } else {
        //tiến trình cha
        close(parentToChild[0]); 
        close(childToParent[1]); 
        
        write(parentToChild[1], "x", 1); 
        char buf;
        read(childToParent[0], &buf, 1); 
        printf("%d: received pong\n", getpid());
        
        close(parentToChild[1]);
        close(childToParent[0]);
        wait(0); 
    }
    
    exit(0);
}
