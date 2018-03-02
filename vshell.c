#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>

#define MAX_COMMAND_SIZE 60     // max command size..including 10 possible parameters
#define MAX_NUM_PARAMETERS 10       // this shell only supports 10 arguments

#define SHOWPIDS_PIDS_TO_DISPLAY 10 // To display only 10 pids in showpids()
#define HISTORY_COMMANDS_TO_DISPLAY 15  // To display only 15 commands in history()


//#################################
//  Author: Vaibhav Murkute
//  Project: VShell-Terminal Emulator
//  Version: 1.2
// Date: 02/26/2018
//#################################



char* StrDup (char *str);
void showpids();
void showHistory();

int pidList[11];
int currentPidPointer = 0;
pid_t pid = 0;

char* commHistory[60];
int currentCommandPointer = 0;

static void handle_signal (int sig )
{
    switch(sig){
        case SIGINT:
            if(pid != 0){
                kill(pid,SIGTERM);
//                printf("Process %d terminated!\n",pid);
            }
            break;

        case SIGTSTP:
            if(pid != 0){
                kill(pid,SIGSTOP);
//                printf("Process %d Suspended!\n",pid);
            }
            break;

        case SIGCHLD:
             // handling this signal to handle signals from child
             waitpid(pid, NULL, WNOHANG);   // makes sure no zombies
             break;

        default:
            printf("Unable to determine the signal\n");
            break;

    }

}

int main()
{
    struct sigaction act;

    memset (&act, '\0', sizeof(act));   // Zero out the sigaction struct

    act.sa_handler = &handle_signal;    // Set the handler to use the function handle_signal()

    // handle installed for each required signal type
    if (sigaction(SIGINT , &act, NULL) < 0) {
        perror ("sigaction: ");
        return 1;
    }

    if (sigaction(SIGTSTP , &act, NULL) < 0) {
        perror ("sigaction: ");
        return 1;
    }

    if (sigaction(SIGCHLD , &act, NULL) < 0) {
        perror ("sigaction: ");
        return 1;
    }


    char input[60];
    char origInput[60];
    char* command[60];
    char* paths[] = {"/usr/local/bin/","/usr/bin/","/bin/",'\0'};

    char cwd[1024];   // to store current working directory

    memset(pidList,'\0',sizeof(pidList));   // clearing the PID List
    memset(commHistory,'\0',sizeof(commHistory));   // clearing the command History List



    while(1){
        int count=0;
        memset(input,'\0',sizeof(input));   // clearing input for fresh start

        if(!(getcwd(cwd,sizeof(cwd)))){
            perror("Error: getcwd()");
        }

        printf("vsh: %s>\t",cwd);   // prints out the vsh prompt

        while( !fgets (input, MAX_COMMAND_SIZE, stdin) );     // scans all the input

        memcpy(origInput,input,MAX_COMMAND_SIZE);   // keeping a copy of original input string,
        origInput[MAX_COMMAND_SIZE]='\0';           //  cuz strtok() will distort input

        char* str = strtok(input," ");  // Tokenizing the first word

        // Limiting no. of accepted parameters to MAX_NUM_PARAMETERS

        while(count <= MAX_NUM_PARAMETERS && str != NULL){
            str = StrDup(str);
            command[count] = str;       // tokenizing rest of the input string
            str = strtok(NULL," ");
            count += 1;
        }

        command[count] = NULL;      // NULL terminating input string

        //---- Handling cd command ---- //
        if(strcmp(command[0],"cd")==0){
            if(command[1] != NULL){
                if(chdir(command[1]) == -1)
                    printf("<!> Invalid Path\n");
            }else{
                chdir("/");
            }

            commHistory[currentCommandPointer]= StrDup(origInput);    // keeping record of showpids command
            currentCommandPointer++;
            continue;

        }else if(strcmp(command[0],"quit")==0 || strcmp(command[0],"exit")==0){
            exit(0);
        }else if(strcmp(command[0],"\0")==0){
            continue;
        }else if(strcmp(command[0],"showpids")==0){
            showpids();
            commHistory[currentCommandPointer]= StrDup(origInput);    // keeping record of showpids command
            currentCommandPointer++;
            continue;
        }else if(strcmp(command[0],"history")==0){
            commHistory[currentCommandPointer]= StrDup(origInput);    // keeping record of history command
            currentCommandPointer++;
            showHistory();
            continue;
        }else if(strcmp(command[0],"bg")==0){
            commHistory[currentCommandPointer]= StrDup(origInput);    // keeping record of bg command
            currentCommandPointer++;
            kill(pid,SIGCONT);
            continue;

        }/*else if(strcmp(command[0],"fg")==0){
            commHistory[currentCommandPointer]= StrDup(origInput);    // keeping record of fg command
            currentCommandPointer++;
            kill(pid,SIGCONT);
            waitpid(pid, NULL, WNOHANG);
            printf("Process %d is now running in foreground!",pid);
        }   */

        /////////////////////////////////////////

        pid = fork();

        if(pid == -1){
            perror("Fork Failed!");
            exit(EXIT_FAILURE);
        }else if(pid == 0){
            if(*(origInput)=='!'){   // handling history command execution
                int index = origInput[1];  // converting ASCII to int with index-'0'

                if(isdigit(index)>0 && (index-'0') < currentCommandPointer){
                    index = index - '0';

                    if(origInput[2] != '\0' && isdigit(origInput[2])>0){
                        index = index*10 + (origInput[2]-'0');    // handled index for 2 digit input
                    }

                    if(index >= currentCommandPointer){
                        printf("<!> Command not in history\n");
                        continue;
                    }

                    int tempIndex = strcspn(commHistory[index],"\0");
                    char comm[tempIndex];
                    memcpy(comm,commHistory[index],tempIndex);   // copy whole command into comm
                    comm[tempIndex]='\0';  // using comm to avoid distorting orig input with strtok()

                    // again tokenizing the input command
                    int count1 = 0;
                    char* str1 = strtok(comm," ");

                    while(str1 != NULL){
                        str1 = StrDup(str1);
                        command[count1] = str1;       // tokenizing rest of the input string
                        str1 = strtok(NULL," ");
                        count1 += 1;
                    }

                    command[count1] = NULL;      // NULL terminating input string

                }else{
                    printf("<!> Command not in history\n");
                    continue;
                }

            }

            if(strcmp(command[0],"showpids")==0){
                showpids();
                continue;
            }else if(strcmp(command[0],"history")==0){
                showHistory();
                continue;
            }

            if(execvp(command[0],command) == -1){  // looking for command in the current working dir
                char commandPath[50];
                int j=0;
                for(j=0; paths[j];j++){
                    strcpy(commandPath,paths[j]);
                    strcat(commandPath,command[0]);

                    if(execvp(commandPath,command) == -1){ //look for command in the next directory
                        if(!paths[j+1]){  // input command is invalid
                            printf("%s : Command not found.\n\n",command[0]);
                            break;
                        }
                        continue;

                    }else
                        break;
                }
            }

            exit(EXIT_SUCCESS);

        }else{
            pidList[currentPidPointer] = (int) pid;     // keeping record of PIDs for showpids command
            currentPidPointer++;

            if(origInput[0] != '!'){    // to avoid history commands !4 from getting into history
                commHistory[currentCommandPointer] = StrDup(origInput);  // keeping record of all the input commands for history
                currentCommandPointer++;
            }

//            pause();
            wait(NULL);
            int i = 0;

            while(i < count){
                free(command[i]);
                i += 1;
            }

        }


    }
    return 0;
}


char* StrDup (char *str) {
    char *result = malloc (strlen (str) + 1);
    if (result == NULL)
        return NULL;
    str[strcspn(str,"\n")]='\0';   // to remove trailing new line character put by fgets()
    strcpy (result, str);
    return result;
}

void showpids(){
    // to display only certain number of PIDs
    int i = currentPidPointer-(SHOWPIDS_PIDS_TO_DISPLAY);

    // if i is negative start i from zero
    if(i<0)
        i=0;

    for(;i<currentPidPointer;i++){
       printf("%d : %d\n",i,pidList[i]);
    }
    return;
}

void showHistory(){
    // to display only certain number of History commands
    int j = currentCommandPointer-(HISTORY_COMMANDS_TO_DISPLAY);

    // if j is negative start i from zero
    if(j<0)
        j=0;

    for(;j<currentCommandPointer;j++){
        printf("%d : %s\n",j,commHistory[j]);
    }
    return;
}
