//Name: Kushal Patel		ID: V00733023
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#define MAX 300

typedef struct{
	pid_t pid;
	char *cmd;
	int isStop; //process status flag
	int proc_id; //trace which pid to kill
}proc;
int flag = 0;//flag for bglist
int procNum = 0;//keep track of processess

void exec(int arg_count,char **args);//execute fork
void list(int bgCount,proc *bList);//print all processes
void killBg(int bgCount,int id,proc *bList);//kill pid
void remBg(pid_t pid,int *bgCount,proc *bList);//remover process
void stop(int bgCount,proc *bList,int x);//stop input process
void cont(int bgCount,proc *bList,int x);//continue input process
proc bgExec(int arg_count,char **args, char *dir);//execute bg process
pid_t bgProc();//finds background processess
void in(int *arg_count,char *cmdLine,char **args);//read and tokenize user input

int main(){
	proc *bList = (proc*) malloc(sizeof(proc));
	int bgCount = 0;
	pid_t bgPid;
	
	while(1){
		char dir[1600];
		getcwd(dir,sizeof(dir));
		strcat(dir,">");
		char *cmdLine;
		cmdLine = readline("PMan>");
		
		char*args[MAX] = {NULL};
		int arg_count = 0;
		in(&arg_count,cmdLine,args);
		bgPid = bgProc();		
		if(bgPid>0){
			remBg(bgPid,&bgCount,bList);
		}
		
		if(arg_count>0){
			if(!strcmp(args[0],"bg")){
				args[0] = NULL;
				int i = 0;
				while(i<arg_count - 1){
					args[i] = args[i+1];
					i++;
				}
				args[arg_count - 1] = NULL;
				arg_count--;
				char direc[500];
				getcwd(direc,sizeof(direc));
				proc newProc = bgExec(arg_count,args,direc);
				bList = (proc*) realloc(bList,(bgCount+1)*sizeof(proc));
				bList[bgCount++] = newProc;
			}else if(!strcmp(args[0],"bglist")){
				list(bgCount,bList);
			}else if(!strcmp(args[0],"bgkill")){
				if(arg_count==2){
					int k = atoi(args[1]);
					killBg(bgCount,k,bList);
				}else{
					printf("Invalid PID\n");
					exit(1);
				}
			}else if(!strcmp(args[0],"bgstop")){
				if(arg_count==2){
					flag = 1;
					int l = atoi(args[1]);
					stop(bgCount,bList,l);
				}else{
					printf("Invalid PID\n");
				}
			}else if(!strcmp(args[0],"bgstart")){
				if(arg_count==2){
					flag = 1;
					int k = atoi(args[1]);
					cont(bgCount,bList,k);
				}else{
					printf("Invalid PID\n");
				}
			}else{
				exec(arg_count,args);
			}
		}
	}
	free(bList);
	return(0);						
}

void exec(int arg_count,char **args){
	pid_t cPid;		
	cPid = fork();
	if(cPid>=0){//fork works
		if(cPid==0){//
			args[arg_count] = NULL;
			if(execvp(args[0],args)<0){
				perror("Cannot Execute");
				exit(1);
			}
		}else{ //wait for child
			waitpid(cPid,NULL,0);
		}
	}else{//fork does not work
		perror("Cannot fork");
		exit(1);
	}
	return;
}

void list(int bgCount,proc *bList){
	int i = 0;
	if(flag==1){
		while(i<bgCount){
			if(bList[i].isStop==0){
				printf("%d[Running]: %s\n",bList[i].pid,bList[i].cmd);
			}else{
				printf("%d[Stopped]: %s\n",bList[i].pid,bList[i].cmd);
			}
		i++;
		}
	}else{
		while(i<bgCount){
			printf("%d: %s\n",bList[i].pid,bList[i].cmd);
			i++;
		}
	}
	printf("There are %d jobs running in the background\n",bgCount);
	return;
}

void killBg(int bgCount,int id,proc *bList){
	int i = 0;
	while(i<bgCount){
		if(bList[i].pid==id){
			kill(bList[i].pid,SIGKILL);
		}
		i++;
	}
	return;
}

void remBg(pid_t pid,int *bgCount,proc *bList){
	int i = 0;
	int remId = 0;
	while(i<(*bgCount)){
		if(bList[i].pid==pid){
			printf("Terminated Process %d PID: %d %s\n",bList[i].proc_id,bList[i].pid,bList[i].cmd);
			(*bgCount)--;
			remId = i;
			break;
		}
		i++;
	}
	
	int j = remId;
	while(j<(*bgCount)){
		bList[j] = bList[j+1];
		j++;
	}
	return;
}

void stop(int bgCount,proc *bList,int x){
	int i = 0;
	while(i<bgCount){
		if(bList[i].proc_id==x){
			if(bList[i].isStop==1){
				printf("Process already inactive");
			}else{
				bList[i].isStop=1;
                printf("Stopping Process: %d PID: %d %s\n",bList[i].proc_id,bList[i].pid,bList[i].cmd);
                kill(bList[i].pid,SIGSTOP);
            }
        }
        i++;
    }
    return;
}

void cont(int bgCount,proc *bList,int x){
	int i = 0;
	while(i<bgCount){
		if(bList[i].proc_id==x){
			if(bList[i].isStop==0){
				printf("Process already running\n");
			}else{
				bList[i].isStop = 0;
				printf("Starting process %d PID: %d %s\n",bList[i].proc_id,bList[i].pid,bList[i].cmd);
				kill(bList[i].pid,SIGCONT);
			}
		}
		i++;
	}
	return;
}

proc bgExec(int arg_count,char **args, char *dir){
    pid_t cPid;
    int i = 0;
    char arg_str[MAX];
    proc newProc;
    
    cPid = fork();
    if(cPid>=0){
        if(cPid==0){//
            args[arg_count] = NULL;
            if(execvp(args[0],args)<0){
                perror("Cannot Execute");
                exit(1);
            }
        }else{
            int arg_s = 0;
            newProc.pid = cPid;
            arg_s = arg_s + strlen(dir+1);//format length of directory
            strcpy(arg_str,dir);//add input to args[]
            
            while(i<arg_count){
                if(i==0){
                    strcat(arg_str," ");
                    strcat(arg_str,args[0]);
                }else if(arg_s<MAX){
                    strcat(arg_str," ");
                    strcat(arg_str,args[i]);
                }else{
                	perror("Out of Memory");
                	exit(1);
                }
                arg_s = arg_s+strlen(args[i])+1;
                i++;
            }
            newProc.isStop = 0;
            newProc.cmd = (char*) malloc(arg_s*sizeof(char)+1);
            strcpy(newProc.cmd,arg_str);
            newProc.proc_id = procNum;
           	procNum++;
        }
    }else{ //unable to fork
    	perror("Cannot fork");
    	exit(1);
    }
    return newProc;
}

pid_t bgProc(){//look for processes running in the bg
	int bgPid;
	int stat;//status
	bgPid = waitpid(-1,&stat,WNOHANG);
	for(;;){
		if(bgPid==0){
			return 0;
		}else{
			return bgPid;
		}
	}
}		
	
void in(int *arg_count,char *cmdLine,char **args){//breaks input for usage elsewhere
	char *token;
	token = (char*) malloc(sizeof(char));
	token = strtok(cmdLine," \t\n");
	while(token!=NULL){
		args[*arg_count]=token;
		token=strtok(NULL," \t\n");
		(*arg_count)++;
	}
	return;
}