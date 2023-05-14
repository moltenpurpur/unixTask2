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


int flagErr = 0;


void errHandling(char * errStr, int immediateExit) {
    flagErr = 1;
    fprintf(stderr, "%s", errStr);

    if (immediateExit) {
        exit(1);
    }
    else {
        kill(getpid(), SIGINT);
    }
}


int fileCheck(char * file) {
    struct stat st;

    if (stat(file, & st) == -1) {
        perror("error getting file information (stat function)\n");
        return 0;
    }

    if (S_ISREG(st.st_mode)) {
        return 1;
    }
    else {
        fprintf(stderr, "%s", "File is not a regular");
        exit(1);
    }
}


void lockingFile(char * file, int * lockExist, int immediateExit) {
    char buf[1024];
    sprintf(buf, "%s.lck", file);
 
    int fd_lock = open(buf, 
                        O_WRONLY | O_CREAT | O_TRUNC, 
                        S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd_lock == -1) {
        perror("Failed create lock file\n");
        exit(1);
    }

    * lockExist = 1;
    pid_t curPid = getpid();
    char out_buf[1024] = {0};
    sprintf(out_buf, "%d\n", curPid);
    size_t len = strlen(out_buf);

    if (write(fd_lock, out_buf, strlen(out_buf)) != len) {
        errHandling("File read error\n", immediateExit);
    }

    if (close(fd_lock) == -1) {
        perror("File close error\n");
        exit(1);
    }
}


void unlockingFile(char * file, int * lockExist, int immediateExit) {
    char buf[1024];
    sprintf(buf, "%s.lck", file);
    // if (!fileCheck(buf)) {
    //     errHandling("Lock file existence error", immediateExit);
    // }

    int fd_lock = open(buf, 
                       O_WRONLY | O_CREAT | O_TRUNC, 
                       S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd_lock == -1) {
        errHandling("File open error\n", immediateExit);
    }

    char in_buf[1024];
    size_t len = read(fd_lock, in_buf, 1023);
    if (fd_lock == -1) {
        errHandling("File read error\n", immediateExit);
    }

    in_buf[len] = '\0';
    pid_t curPid = getpid();
    pid_t lockPid;

    if (scanf(in_buf, "%d", & lockPid) != 1) {
        errHandling("File file format error\n", immediateExit);
    }
    if (curPid != lockPid) {
        errHandling("Invalid unlock file PID\n", immediateExit);
    }
    if (close(fd_lock) == -1) {
        perror("File close error\n");
        exit(1);
    }

    remove(buf);
    * lockExist = 0;
}


int lockNum = 0;
int unlockNum = 0;


void sigFunc(int sig) {
    int logLocked = 0;

    lockingFile(STAT, & logLocked, 1);

    char out_buf[1024] = {0};
    sprintf(out_buf, 
            "Process PID: %d\nlocks: %d\nunlocks: %d\n%s\n \n", 
            getpid(),
            lockNum,
            unlockNum,
            flagErr ? "End with error" : "End without error");
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
    unlockingFile(STAT, & logLocked, 1);
    exit(0);
}


int myFileLocked = 0;


int main(){
    if (signal(SIGINT, & sigFunc) == SIG_ERR){
        perror("Signal processing function error\n");
        exit(1);
    }

    while (1) {
        lockingFile(MY_FILE, & myFileLocked, 0);
        lockNum++;
        sleep(1);
        unlockingFile(MY_FILE, & myFileLocked, 0);
        printf("%d", unlockNum);
        unlockNum++;
    }

    return 0;
}