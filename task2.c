#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#define MY_FILE "./my_file.txt"
#define STAT "./stat.txt"


int lockNum = 0;
int unlockNum = 0;


void lockingFile(char * file) {
    char buf[1024];
    sprintf(buf, "%s.lck", file);

    int fd_lock = -1;
    while (fd_lock == -1) {
        fd_lock = open(buf, O_CREAT | O_EXCL | O_RDWR, 0640);
    }

    pid_t curPid = getpid();
    char out_buf[1024] = {0};
    sprintf(out_buf, "%d\n", curPid);
    size_t len = strlen(out_buf);

    write(fd_lock, out_buf, strlen(out_buf));

    if (close(fd_lock) == -1) {
        perror("File close error\n");
        exit(1);
    }

    lockNum++;
}


void unlockingFile(char * file) {
    char buf[1024];
    sprintf(buf, "%s.lck", file);

    int fd_lock = open(buf, 
                       O_WRONLY | O_CREAT | O_TRUNC, 
                       S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    char in_buf[1024];
    size_t len = read(fd_lock, in_buf, 1023);

    in_buf[len] = '\0';
    pid_t curPid = getpid();
    pid_t lockPid;

    scanf(in_buf, "%d", & lockPid);
    if (close(fd_lock) == -1) {
        perror("File close error\n");
        exit(1);
    }

    remove(buf);
    unlockNum++;
}


void sigFunc(int sig) {
    int logLocked = 0;

    lockingFile(STAT);

    char out_buf[1024] = {0};
    sprintf(out_buf, 
            "Process PID: %d\nlocks: %d\nunlocks: %d\n", 
            getpid(),
            lockNum,
            unlockNum);
    
    printf("\naaaaaaaaaaaaaaaaaaa\n");

    size_t len = strlen(out_buf);
    int fd_lock = open(STAT, 
                       O_WRONLY | O_APPEND | O_CREAT,
                       S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if (write(fd_lock, out_buf, strlen(out_buf)) != len) {
        fprintf(stderr, "%s", "io error\n");
        exit(1);
    }
    if (close(fd_lock) == -1) {
        perror("File close error\n");
        exit(1);
    }
    unlockingFile(STAT);
    
    exit(0);
}


int myFileLocked = 0;


int main(){
    printf("\nppppppppppppppppppppppp\n");
    signal(SIGINT, & sigFunc);

    while (1) {
        lockingFile(MY_FILE);
        sleep(1);
        unlockingFile(MY_FILE);
        printf("%d", unlockNum);
      
    }

    return 0;
}