#include "kernel/types.h"
#include "user/user.h"

void primes(int fd) __attribute__((noreturn));

void primes(int fd) {
    int prime;
    if (read(fd, &prime, sizeof(prime)) != sizeof(prime)) {
        close(fd);
        exit(0);  
    }
    
    printf("prime %d\n", prime);
    
    int p[2];
    if (pipe(p) < 0) {
        fprintf(2, "pipe failed\n");
        exit(1);
    }
    
    int pid = fork();
    if (pid < 0) {
        fprintf(2, "fork failed\n");
        exit(1);
    }
    
    if (pid == 0) {
        // Tiến trình con
        close(p[1]);   
        close(fd);      
        primes(p[0]); 
    } else {
        // Tiến trình cha
        close(p[0]);   
        int num;
        while (read(fd, &num, sizeof(num)) == sizeof(num)) {
            if (num % prime != 0) {
                //gửi số không chia hết cho prime sang pipe mới
                write(p[1], &num, sizeof(num));
            }
        }
        
        close(fd);
        close(p[1]);
        
        wait(0);
        exit(0);
    }
}

int main(int argc, char *argv[]) {
    // Tạo pipe ban đầu
    int p[2];
    if (pipe(p) < 0) {
        fprintf(2, "pipe failed\n");
        exit(1);
    }
    
    // Fork tiến trình đầu tiên để bắt đầu pipeline
    int pid = fork();
    if (pid < 0) {
        fprintf(2, "fork failed\n");
        exit(1);
    }
    
    if (pid == 0) {
        //tiến trình con - bắt đầu pipeline
        close(p[1]);    
        primes(p[0]);   //lọc số nguyên tố
    } else {
        //tiến trình cha - sinh các số từ 2 đến 280
        close(p[0]);   
        
        for (int i = 2; i <= 280; i++) {
            write(p[1], &i, sizeof(i));
        }
        close(p[1]);
        wait(0);
        exit(0);
    }
}



