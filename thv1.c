/*
Krishna Patel
kpatel7
CIS 415 Project 1
This is my own work, but sites I referenced are listed below.
I also used code provided in some of the labs.

https://man7.org/linux/man-pages/man5/proc.5.html
*/
#include <stdlib.h>
#include "p1fxns.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>

#define UNUSED __attribute__((unused))

int formatusecs(int startSeconds, int endSeconds, int startMsec, int endMsec){
    long long start = startSeconds*1000000;
    long long end = endSeconds*1000000;
    //printf("%Ld\n", (end-start + (endMsec-startMsec)));
    return (int) ((end-start + (endMsec-startMsec))%1000000)/1000; //Will return a nonnegative number of microsecs
}

int main(int argc, char *argv[]){
    int msec = 0; int ncores = 0; int nprocesses =0; UNUSED int val = -1; int status; int option, index; extern int opterr;
    bool ifL = false; bool ifQ = false; bool ifC = false; bool ifP = false;
    char *p; char *commandLine; char** argList;
    opterr = 0;
    char * UNUSED msecStr, UNUSED nprocessesStr, UNUSED ncoresStr;
    pid_t id;
    struct timeval start, end;
    if ((p = getenv("TH_QUANTUM_MSEC"))!= NULL){
        msec = p1atoi(p);
        ifQ = true;
    }
    if ((p = getenv("TH_NPROCESSES"))!= NULL){
        nprocesses = p1atoi(p);
        ifP = true;
    }
    if ((p = getenv("TH_NCORES"))!= NULL){
        ncores = p1atoi(p);
        ifC = true;
    }
    while((option = getopt(argc, argv, "q:p:c:l:?"))!=-1){
        switch(option){
            case 'q': msec = p1atoi(optarg); ifQ = true; break;
            case 'p': nprocesses = p1atoi(optarg); ifP = true; break;
            case 'c': ncores = p1atoi(optarg);  ifC = true; break;
            case 'l': ifL = true;  commandLine = optarg; break;
            default:
            p1perror(2, "ERROR: Illegal flag used\n");
            return EXIT_FAILURE;
            }
    }
    if(!(ifL && ifQ && ifC && ifP)){
        p1perror(2, "ERROR: All flags not specified\n");
        return EXIT_FAILURE;
    }
    if(msec <= 0 || nprocesses <= 0 || ncores <= 0){
        p1perror(2, "ERROR: Negative input");
        return EXIT_FAILURE;
    }
    
    int argSize = 1;
    for(int i = 0; i < p1strlen(commandLine); i++){
        if(commandLine[i] == ' ' && i != p1strlen(commandLine)-1){
            argSize += 1;
        }
    }

    argList = (char **)malloc((argSize+1)*sizeof(char *));
    index = 0;
    int y = 0;
    //p1putstr(1, commandLine);
    while(index != -1){
        char word[64];
        index = p1getword(commandLine, index,word);
        argList[y] = p1strdup(word); y+=1;
        if(index == p1strlen(commandLine)){
            index = -1;
        }
    }
    argList[argSize] = NULL;

    pid_t idList[nprocesses];
    gettimeofday(&start, NULL);
    for(int i = 0; i < nprocesses; i++){
        id= fork();
        idList[i] = id;
        if(id == 0){
            if((status = (execvp(argList[0], argList)))== -1){
                p1putstr(2, "ERROR: execvp failed\n");
                _exit(status);
                p1putstr(1, "error\n");
                goto cleanup;
            }
        }else if(id < 0){
            p1putstr(2, "ERROR: Fork failed\n");
            return EXIT_FAILURE;
        }
    }
    for(int i = 0; i < nprocesses; i++){
        wait(&idList[i]);
        
    }
    gettimeofday(&end, NULL);
    //long time = (((start.tv_sec - start.tv_sec)*1000000L
     //      +end.tv_usec) - start.tv_usec);
    int seconds = ((end.tv_sec)-start.tv_sec);
    int ms = formatusecs(start.tv_sec, end.tv_sec, start.tv_usec, end.tv_usec);
    char msecsFormatted[4];
    char temp[100];
    p1itoa(ms, temp);
    p1strpack(temp, 3, '0', msecsFormatted);
    p1putstr(1, "The elapsed time to execute ");
    p1putint(1, nprocesses);
    p1putstr(1, " copies of ");
    p1putstr(1, argList[0]);
    p1putstr(1, " on ");
    p1putint(1, ncores);
    p1putstr(1," processer(s) is ");
    p1putint(1, seconds); p1putchr(1, '.');
    p1putstr(1, msecsFormatted);
    p1putstr(1, " seconds");
    p1putchr(1, '\n');
    

    cleanup:
        if(argList!=NULL){
            for(int x = 0; x < argSize; x++){
                if(argList[x] != NULL){
                    free(argList[x]);
                }
            }
            free(argList);
        }
        return EXIT_SUCCESS;
}