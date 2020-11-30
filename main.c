#include <stdio.h>
#include <sys/sysinfo.h>
#include <stdlib.h>
#include<sys/socket.h>
#include<string.h>
#include<arpa/inet.h>
#include<unistd.h>

//This Program was written by Ahmed and Ryan

//todo:
//todo: save,read,delete file function, send to file server

//todo: to communicate with memory cache, must format load, rm, store commands correctly

//todo: create a tcp connection using a socket, not sure where it should necessarily link to at this point, probably server ip and port number of a different team


int length;
int * requestArray;

//retrieves all available processors on system, then doubles
int retrieveTotalAllowedThreads() {
    return get_nprocs_conf() *2;
}

//**************************
//Request Management Methods
//**************************
void initRequestList() {
    for (int i = 0; i < retrieveTotalAllowedThreads(); i++)
        requestArray[i] = -1;
    length = sizeof(requestArray);
}
void addRequest(int pid) {
    for (int i = 0; i < length; ++i) {
        if (requestArray[i] == -1) {
            requestArray[i] = pid;
            break;
        }
    }
}

//*******************
//User Input Commands
//*******************
void saveFile() {}

void readFile() {}

void deleteFile() {}

//******************************
//Format Server Requests
//******************************
void formatStoreRequest() {}

void formatLoadRequest() {}

void formatDeleteRequest(int forFileServer) {}

void formatSaveRequest() {}

void formatReadRequest() {}


//*******************
//TCP Request Methods
//*******************
//Initialize Socket for server communication
int initSocket(void) {
    int serverSocket;
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    return serverSocket;
}

//Establish your TCP connection to another server. If your forFileServer value is 0, connect to the memory cache server. If not, connect to the file server.
int establishTCPConnection(int forFileServer, int serverSocket) {
    //if you want to communicate
    int port = (forFileServer == 1)? 108: 107;

    struct sockaddr_in remote= {0};
    //todo: set the ip of the local host here, don't know if we need a command to get this or can just use risingstar's ip
    remote.sin_addr.s_addr = inet_addr("todo");
    remote.sin_family = AF_INET;
    remote.sin_port = htons(port);

    return connect(serverSocket,(struct sockaddr *)&remote,sizeof(struct sockaddr_in));
}

//send data to the server
int sendToServer(int serverSocket ,char* requestType,int lenRqst) {
    struct timeval tv;
    tv.tv_sec = 20;  /* 20 Secs Timeout */
    tv.tv_usec = 0;

    //if the server request times out, return a -1 as a send request cannot have a size of -1
    if(setsockopt(serverSocket,SOL_SOCKET,SO_SNDTIMEO,(char *)&tv,sizeof(tv)) < 0)
        return -1;

    return send(serverSocket, requestType, lenRqst, 0);
}

//receive data from the server
int receiveFromServer(int serverSocket, char* Rsp, int RvcSize) {
    struct timeval tv;
    tv.tv_sec = 20;
    tv.tv_usec = 0;

    //if the server request times out, return a -1 as a recv request cannot have a size of -1
    if(setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO,(char *)&tv,sizeof(tv)) < 0)
        return -1;

    return recv(serverSocket, Rsp, RvcSize, 0);
}

//I don't think we'll need this
int setUpBackgroundListener() {}

//***********
//Main Method
//***********
int main(int argc, char *args[]) {
    //standard user input, as always
    char buffer[32];
    //allocate memory to the array, since this is established at the beginning, we DO NOT free this memory until we finish the loop,
    //as we only allocate once, so no memory leak will occur
    requestArray = (int*)malloc(retrieveTotalAllowedThreads());

    initRequestList();

    while(1) {
        printf("Dispatcher> ");
        fgets(buffer, 32, stdin);

        if (addRequest()) {
            pthread_t thread_id;
            pthread_create(&thread_id, NULL, runProcess, (void*)&newProcess);
        }
        else {

        }

        switch (buffer[0]) {
            case 'r': {
                printf("read");
                printf("te");
            }
            case 's': {
                printf('write');
            }
            case 'd': {
                printf('delete');
            }
            default: {
                printf('not recognized');
            }
        }
    }
}




