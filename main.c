#include <stdio.h>
#include <sys/sysinfo.h>
#include <stdlib.h>
#include<sys/socket.h>
#include<string.h>
#include<arpa/inet.h>
#include <pthread.h>
#include<unistd.h>

//This Program was written by Ahmed and Ryan

//todo: add a fork somewhere
//todo: save,read,delete file function, send to file server

//todo: to communicate with memory cache, must format load, rm, store commands correctly

//todo: create a tcp connection using a socket, not sure where it should necessarily link to at this point, probably server ip and port number of a different team

typedef struct {
    int PID;//may be necessary, may not be we'll see
    int requestType; //1 = save, 2 = read, 3 = delete, 0 = invalid request
    int requestSize; //requestType filename *n*:[contents of file]
    char *requestFileName; //requestType *filename* n:[contents of file]
    char *requestContent; //requestType n:[*contents of file*]
    } Request;


int length;
int allowedRequests;
int currentRequests;

//*********************
//Miscellaneous Methods
//*********************
//retrieves all available processors on system, then doubles
int retrieveTotalAllowedThreads() {
    return get_nprocs_conf() *2;
}

char* getSubstring(int startingCharacter, int finalCharacter, char* buffer, int isInt) {
    //how many characters would you like returned +1 for \0
    int substringLength = finalCharacter - startingCharacter;

    //If you would like an int to be returned, don't reserve a byte for the \0
    char *subbuff = malloc((isInt==1)? substringLength-1: substringLength);
    memcpy(subbuff, &buffer[startingCharacter], substringLength);

    //Only add the null terminator if you don't plan to cast to int
    if (isInt!=1) subbuff[substringLength-1] = '\0';

    return subbuff;
}


//**************************
//Request Management Methods
//**************************

//todo: remove if we don't find a use for these later on
//void initRequestList() {
//    for (int i = 0; i < retrieveTotalAllowedThreads(); i++)
//        requestArray[i] = -1;
//
//    length = sizeof(requestArray);
//}
//
//void addRequest(int pid) {
//    for (int i = 0; i < length; ++i) {
//        if (requestArray[i] == -1) {
//            requestArray[i] = pid;
//            break;
//        }
//    }
//}
//
//void removeRequest(int pid) {
//    for (int i = 0; i < length; ++i) {
//        if (requestArray[i] == pid) {
//            requestArray[i] = -1;
//            break;
//        }
//    }
//}

//*******************
//User Input Commands
//*******************
Request interpretUserInput(char *userInput) {
    Request request;
    int startingCharacter = 5;

    //Determine which request the user is trying to make
    switch (userInput[0]) {
        case 's':
            request.requestType=1;
        case 'r':
            request.requestType=2;
        case 'd': {
            request.requestType=3;
            //since delete is a longer word than the other two...
            startingCharacter += 2;
        }
        default: {
            request.requestType=0;
        }
    }

    //find fileName, and fileSize/fileContent if applicable
    if (request.requestType != 0) {
        for (int i = startingCharacter; i < 32; ++i) {
            //first breakpoint, check for filename
            if(userInput[i] == ' '|| userInput[i] == '\n') {
                request.requestFileName = getSubstring(startingCharacter, i, userInput, 0);

                //if read, go to second and third breakpoint
                if (request.requestType == 1) {
                    startingCharacter = i;
                    while (userInput[i] != ':') i++;

                    //second breakpoint
                    request.requestSize = atoi(getSubstring(startingCharacter, i, userInput, 1));

                    //bypass characters :[ straight to the first character of the file
                    startingCharacter = i + 2;
                    while (userInput[i] != ']') i++;

                    //third breakpoint
                    request.requestContent = getSubstring(startingCharacter, i, userInput, 0);
                }
                break;
            }
        }
    }

    return request;
}

void saveFile(Request request) {
    //user inputted save test.c 4:[this is what I want written in the file]
    //take the buffer, format it into a write test.c 4:[this is what I want written in the file]
    //once the request is formatted correctly, try to establish an establishTCPConnection()
    //if true, call a sendToServer() request to the file server, and if successful, clear this request from the request array
}

void readFile(Request request) {
    //user inputs read filename
    //format to load filename from buffer
    //once the request is formatted correctly, try to establish an establishTCPConnection()
    //if true, call a receiveFromServer() request to the memory server, and if successful, clear this request from the request array
    //if you get a return of 0, however, this means the file is not on the memory cache and needs to be loaded from file server
    //so... try to establish a new establishTCPConnection()
    //if successful, call a receiveFromServer() request to the file server, and if successful, clear this request from the request array
    //for file server send read filename NOT load filename

    //once the file is found, print out what the server returns
}

void deleteFile(Request request) {
    //user inputs delete filename
    //format to rm filename from buffer
    //once the request is formatted correctly, try to establish an establishTCPConnection()
    //if true, call a sendToServer() request to the memory server, and if successful, cool but one more step...
    //next, try to establish another establishTCPConnection()
    //this time, call a sendToServer() request to the file server, and if successful, this time remove from request array
    //Just as in read file, send the delete filename request NOT rm filename
}


void* beginRequestThread(void* requestPointer) {
    Request request = *(Request *)requestPointer;

    switch (request.requestType) {
        case 1: saveFile(request);
        case 2: readFile(request);
        case 3: deleteFile(request);
    }

    currentRequests = currentRequests -1;
}

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
    remote.sin_addr.s_addr = inet_addr("10.0.95.31");
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
    //set how many threads can occur at a given time, only need to keep an int since there's no requirement
    // to print request details outside of its own thread
    allowedRequests = retrieveTotalAllowedThreads();

    while(1) {
        //standard user input, as always
        char buffer[32];

        printf("Dispatcher> ");
        fgets(buffer, 32, stdin);

        //if a new request won't go over the thread limit
        if (currentRequests+1 <= allowedRequests) {
            currentRequests ++;

            Request request = interpretUserInput(buffer);

            if (request.requestType != 0) {
                pthread_t thread_id;
                pthread_create(&thread_id, NULL, beginRequestThread, (void*)&request);
            }
        }

        else printf("Please wait for a request to open up.");
    }
}




