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
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include "ADTs/queue.h"
#include <fcntl.h>
#define UNUSED __attribute__((unused))
volatile bool USR1_seen = false;
volatile bool STOP_seen = false;
volatile bool CONT_seen = false;
volatile bool ALRM_seen = false;
bool firstARLM = true;
int quantum, ncores = 0; int nprocesses;
int timesProcPrinted = 0;
volatile int nprocesscopy; 
const Queue *q= NULL;
const Queue *runq = NULL;
int tempid;
typedef struct process
{
    pid_t id;
    bool isRunning;
    bool isCompleted;
}Process;
Process **runList;

int formatusecs(int startSeconds, int endSeconds, int startMsec, int endMsec){
    long long start = startSeconds*1000000;
    long long end = endSeconds*1000000;
    return (int) ((end-start + (endMsec-startMsec))%1000000)/1000; //Will return a nonnegative number of microsecs
}

void printProc(pid_t pid){
    char buffer[8096]; char toPrint[8096]; char pidStr[16]; UNUSED char fileBuf[8096]; int fd;
    char state[8096]; char ppid[8096]; char sysCR[8096]; char sysCW[8096]; char mem[8096];
    char utime[8096]; char statbuf[8096]; char iobuf[8096];
    char word[8096]; UNUSED char scratch[8096]; char processName[8096];
    char stime[8096];
    int temp;
    p1itoa(pid, pidStr);
    p1strcpy(buffer, "/proc/");
    p1strcat(buffer, pidStr); p1strcat(buffer, "/stat");
    //p1putstr(1, buffer);p1putchr(1, '\n');
    fd = open(buffer, O_RDONLY);
    //p1putstr(1, "fileDescriptor: "); p1putint(1, fd); p1putchr(1, '\n');
    if(fd == -1){
        p1putstr(2, "ERROR: /proc/pid/stat is not accessible\n");
        exit(EXIT_FAILURE);
    }
    p1getline(fd, statbuf, 8096);
    close(fd);
    p1strcpy(buffer, "/proc/");
    p1strcat(buffer, pidStr); p1strcat(buffer, "/io");
    if ((fd = open(buffer, O_RDONLY)) == -1){
        p1perror(2, "ERROR: /proc/pid/io is not accessible\n");
        exit(EXIT_FAILURE);
    }
    int x = 0;
    while((temp = p1getline(fd, iobuf, 8096))!= 0){
        int index = 0;
        index = p1getword(iobuf, index, word);
        if(x == 2){
            index = p1getword(iobuf, index, word);
            int size = p1strlen(word);
            word[size-1] = '\0'; //replace newline
            p1strcpy(sysCR, word);

        }
        if(x == 3){
            index = p1getword(iobuf, index, word);
            int size = p1strlen(word);
            word[size-1] = '\0'; //replace newline
            p1strcpy(sysCW, word);        }
        x++;
    }
    int index = 0;
    x = 1;
    while((index = p1getword(statbuf, index, word)) != -1){
        //p1putstr(1, "Index: "); p1putint(1, index); p1putchr(1, '\n');

        if(x == 2){
            p1strcpy(processName, word);
        }
        if(x == 3){
            p1strcpy(state, word);
        }
        if(x == 4){
            p1strcpy(ppid, word);
        }
        if(x == 14){
            p1strcpy(utime, word);
        }
        if(x == 15){
            p1strcpy(stime, word);
        }
        if(x == 24){
            p1strcpy(mem, word);
        }
        x++;
    }
    close(fd);
    char *printPtr = toPrint;
    int userTime = p1atoi(utime);
    int kernalTime = p1atoi(stime);
    int totalTime = userTime + kernalTime;
    p1itoa(totalTime, utime);
    //p1putstr(1, statbuf); p1putchr(1, '\n');
    //p1putstr(1, state);
    if(timesProcPrinted == 20)
        timesProcPrinted = 0;
    if(timesProcPrinted == 0)
    p1putstr(1, "PID     name            mem     state      read           write      time(ticks)\n");
    printPtr = p1strpack(pidStr, 7, ' ', printPtr);
    printPtr = p1strpack(processName, 18, ' ', printPtr);
    printPtr = p1strpack(mem, 11, ' ', printPtr);
    printPtr = p1strpack(state, 9, ' ',printPtr );
    printPtr = p1strpack(sysCR, 12, ' ', printPtr);
    printPtr = p1strpack(sysCW, 18, ' ', printPtr);    
    printPtr = p1strpack(utime, 6, ' ', printPtr);    
    timesProcPrinted++;

    p1putstr(1, toPrint);
    p1putchr(1, '\n');
}


void printTime(struct timeval start, struct timeval end, char** argList, int nprocesses, int ncores){
    int seconds = ((end.tv_sec)-start.tv_sec);
    int ms = formatusecs(start.tv_sec, end.tv_sec, start.tv_usec, end.tv_usec);
    char msecsFormatted[4];
    char temp[100];
    p1itoa(ms, temp);
    p1strpack(temp, -3, '0', msecsFormatted);
    p1putstr(1, "The elapsed time to execute ");
    p1putint(1, nprocesses);
    p1putstr(1, " copies of ");
    p1putstr(1, argList[0]);
    p1putstr(1, " on ");
    p1putint(1, ncores);
    p1putstr(1, " processer(s) is ");
    p1putint(1, seconds); p1putchr(1, '.');
    p1putstr(1, msecsFormatted);
    p1putstr(1, " seconds");
    p1putchr(1, '\n');
}

void onusr1(UNUSED int sig){
    USR1_seen = true;
}

void onstop(UNUSED int sig){
    STOP_seen = true;
}

void onchld(UNUSED int sig){
    pid_t pid;
    int status;
    //p1putstr(1, "onchild\n");
    while((pid= waitpid(-1, &status,WNOHANG))>0){ //-1 any child that sends a signal
		if(WIFEXITED(status)|| WIFSIGNALED(status)){
            for(int i = 0; i < ncores; i++){
                if(runList[i] != NULL){
                    Process* p = runList[i];
                    if(p != NULL){
                        if(p->id == pid){
                            p->isCompleted = true;
                            q->enqueue(q, p);
                            runList[i] = NULL;
                        }
                    }
                }
            }
			nprocesscopy--;
		}
	}
}

void onalrm(UNUSED int sig){
    //p1putstr(1, "alrm\n");
    if(firstARLM){
        firstARLM = false;
        for(int i = 0; i < ncores; i++){
            Process *p;
            if(!q->isEmpty(q)){
                bool result = q->dequeue(q, (void **)&p);
                if(result){
                    if(!p->isCompleted){
                        p->isRunning = true;
                        tempid = p->id;
                        kill(p->id, SIGCONT);
                        printProc(tempid);
                        runList[i] = p;
                    }else{
                        //free(p);
                        runList[i] = NULL;
                    }
                }
                
            }
        }
        
    }else{
        for(int i = 0; i<ncores; i++){
           Process *p;
           if(!q->isEmpty(q)){
                p = runList[i];
                if(p!=NULL){
                    p->isRunning = false;
                    if(!p->isCompleted){
                        tempid = p->id;
                        kill(p->id, SIGSTOP);
                        printProc(tempid);
                        q->enqueue(q, p);
                    }else{
                        //free(p);
                        runList[i] = NULL;
                    }
                }  
           }
            runList[i] = NULL;
        }
        for(int i = 0; i<ncores; i++){
            Process *p;
            bool result = q->dequeue(q, (void **)&p);
            if(result){
                if(!p->isCompleted){
                    p->isRunning=true;
                    tempid = p->id;
                    runList[i] = p;
                    printProc(tempid);
                    kill(p->id, SIGCONT);
                }else{
                    //free(p);
                    runList[i] = NULL;
                }
            }
         }
        }

}



int main(int argc, char *argv[]){
    UNUSED int val = -1; int status; int option, index; extern int opterr;
    bool ifL = false; bool ifQ = false; bool ifC = false; bool ifP = false;
    char *p; char *commandLine; char** argList;
    opterr = 0;
    UNUSED char *msecStr, nprocessesStr, ncoresStr;
    UNUSED pid_t id;
    struct timeval start, end;
    struct timespec ms20 = {0, 20000000};
    struct itimerval it_val;
    if ((p = getenv("TH_QUANTUM_MSEC"))!= NULL){
        quantum = p1atoi(p);
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
            case 'q': ifQ = true; quantum = p1atoi(optarg); msecStr = optarg; break;
            case 'p': ifP = true; nprocesses = p1atoi(optarg); nprocessesStr = *optarg; break;
            case 'c': ifC = true; ncores = p1atoi(optarg); ncoresStr = *optarg; break;
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
    if(quantum <= 0 || nprocesses <= 0 || ncores <= 0){
        p1perror(2, "ERROR: Negative input");
        return EXIT_FAILURE;
    }

    if (signal(SIGUSR1, onusr1) == SIG_ERR) {
		p1putstr(2, "Can't establish SIGUSR1 handler\n");
		return EXIT_FAILURE;
	}
    if (signal(SIGALRM, onalrm) == SIG_ERR) {
		p1putstr(2, "Can't establish SIGALRM handler\n");
		return EXIT_FAILURE;
	}
    if ((p = getenv("VARIABLE_NAME"))!= NULL){
        val = p1atoi(p);
    }
    signal(SIGCHLD, onchld);

    nprocesscopy = nprocesses;
    Process *processList[nprocesses];
    q = Queue_create(doNothing); //Readyqueue
    runList = (Process **)malloc(sizeof(Process *) * ncores);
     int argSize = 1;
    for(int i = 0; i < p1strlen(commandLine); i++){
        if(commandLine[i] == ' ' && i != p1strlen(commandLine)-1){
            argSize += 1;
        }
    }
    argList = (char **)malloc((argSize+1)*sizeof(char *));
    index = 0;
    int y = 0;
    while(index != -1){
        char word[64];
        index = p1getword(commandLine, index,word);
        argList[y] = p1strdup(word); y+=1;
        if(index == p1strlen(commandLine)){
            index = -1;
        }
    }
    argList[argSize] = NULL;
    pid_t* idList = (pid_t*)malloc(sizeof(pid_t)*nprocesses);
    gettimeofday(&start, NULL);
    for(int i = 0; i < nprocesses; i++){
        int pid = fork();
        idList[i] = pid;
        if(idList[i] < 0){
            p1putstr(2, "ERROR: Fork failed\n");
            return EXIT_FAILURE;
        }if(idList[i] == 0){
        while (!USR1_seen){
            nanosleep(&ms20, NULL);
        }
        if((status = (execvp(argList[0], argList)))== -1){
                p1putstr(2, "ERROR: execvp failed\n");
                _exit(status);
                p1putstr(1, "error\n");
                goto cleanup;
            }
        else if (status != 1){
            p1putstr(1, "EXECVP\n");}
        }
        if(pid > 0){
            Process *p = (Process *)malloc(sizeof(Process));
            p->id = pid;
            p->isCompleted = false; p->isRunning = false;
            processList[i] = p;
            q->enqueue(q, p);
        }
    }
    for(int i = 0; i < nprocesses; i++){
        kill(idList[i], SIGUSR1);}     
    for(int i = 0; i < nprocesses; i++){
        kill(idList[i], SIGSTOP);
    }
    it_val.it_value.tv_sec = quantum/1000;
    it_val.it_value.tv_usec = (quantum*1000) % 1000000;
    it_val.it_interval = it_val.it_value;
    if (setitimer(ITIMER_REAL, &it_val, NULL) == -1)
        p1perror(1, "error calling setitimer()\n");
    
    while(nprocesscopy){
        pause();
    }
    gettimeofday(&end, NULL);
    printTime(start, end, argList, nprocesses, ncores);

    //while(!q->isEmpty(q)){
    //    Process* x;
       // q->dequeue(q, (void **)&x);
        //p1putint(1, x->id); p1putstr(1, "- \n");
        //free(x);
        //x = NULL;
    //}

    goto cleanup;

    cleanup:
        if(processList != NULL){
            for(int i = 0; i < nprocesses; i++){
                if(processList[i]!= NULL)
                    free(processList[i]);
            }
        }
        if(argList!=NULL){
             for(int x = 0; x < argSize; x++){
                if(argList[x] != NULL){
                    free(argList[x]);
                }
            }
            free(argList);
        }
        if(idList != NULL){
            free(idList);
        }
        if(q != NULL){
            q->destroy(q);
        }

        if(runList != NULL){
            for(int i = 0; i < ncores; i++){
                if(runList[i]!= NULL){
                    free(runList[i]);
                }
            }
            free(runList);
        }

        return EXIT_SUCCESS;
}