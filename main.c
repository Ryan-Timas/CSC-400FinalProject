#include <stdio.h>
#include <sys/sysinfo.h>
//This Program was written by Ahmed and Ryan

//retrieves all available processors on system, then doubles
int retrieveTotalAllowedThreads() {
    return get_nprocs_conf() *2;
}



int main() {
    char buffer[32];

    printf("%d", retrieveTotalAllowedThreads());
    return 0;
}




int main(int argc, char *args[]) {
    int status = 0;
    Process currentProcess;s

    initTable();

    //Within this while loop is where the shell exists,
    while(1) {
        printf("Priority Shell> ");
        fgets(buffer, 32, stdin);

        int processPriority;
        //check for the space character, once you find it you're done with the process name
        for (int i = 0; i < 32; i++) {
            if (buffer[i] == ' ') {
                processPriority = buffer[i+1] - '0';

                buffer[i] = '\0';
                break;
            }
            else if (buffer[i] == '\n') {
                buffer[i] = '\0';
                break;
            }
        }


        Process newProcess = {.priority = processPriority};
        strcpy(newProcess.program, buffer);

        //if the user typed status, then print table...

        if (strcmp(buffer, "status") == 0)
            printTable();
            //...otherwise, we'll build the new process. Then, check if its priority is higher than the currentProcess or not
        else {
            pthread_t thread_id;
            processArray[getNextProcessToRun()].status = 1;
            pthread_create(&thread_id, NULL, runProcess, (void*)&newProcess);
        }
    }
}