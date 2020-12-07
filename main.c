#include <stdio.h>
#include <sys/sysinfo.h>
#include <stdlib.h>
#include<sys/socket.h>
#include<string.h>
#include<arpa/inet.h>
#include <pthread.h>
#include<unistd.h>

#define RESOURCE_SERVER_PORT 1027 // Change this!
#define BUF_SIZE 256

//This Program was written by Ahmed and Ryan

//todo: add a fork somewhere
//todo: save,read,delete file function, send to file server
//todo: nc -l 1029
//sprintf 
// snprintf 
// char str[80];
//   
// sprintf(str, "Value of Pi = %f", M_PI);
//  sprintf(str, "save %s %d:%s", request.requestFileName, request.requestSize, request.reqeustContent)

//todo: to communicate with memory cache, must format load, rm, store commands correctly

//todo: create a tcp connection using a socket, not sure where it should necessarily link to at this point, probably server ip and port number of a different team

typedef struct {
    int requestType; //1 = save, 2 = read, 3 = delete, 0 = invalid request
    int requestSize; //requestType filename *n*:[contents of file]
    char *requestFileName; //requestType *filename* n:[contents of file]
    char *requestContent; //requestType n:[*contents of file*]
    } Request;

int clientSocket;
int serverSocket;

int allowedRequests;
int currentRequests;

//*********************
//Miscellaneous Methods
//*********************
//retrieves all available processors on system, then doubles
int retrieveTotalAllowedThreads() {
    return get_nprocs_conf() *2;
}

//returns a portion of a string. Remember to free memory when done
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

//*******************
//TCP Request Methods
//*******************
void * processClientRequest(void * request) {
    int connectionToClient = *(int *)request;
    char receiveLine[BUF_SIZE];
    char sendLine[BUF_SIZE];

    int bytesReadFromClient = 0;
    // Read the request that the client has
    while ( (bytesReadFromClient = read(connectionToClient, receiveLine, BUF_SIZE)) > 0) {
        // Need to put a NULL string terminator at end
        receiveLine[bytesReadFromClient] = 0;

        // Show what client sent
        printf("Received: %s\n", receiveLine);

        // Print text out to buffer, and then write it to client (connfd)
        snprintf(sendLine, sizeof(sendLine), "true");
        printf("%Sending s\n", sendLine);
        write(connectionToClient, sendLine, strlen(sendLine));

        // Zero out the receive line so we do not get artifacts from before
        bzero(&receiveLine, sizeof(receiveLine));
        close(connectionToClient);
    }
}

// We need to make sure we close the connection on signal received, otherwise we have to wait
// for server to timeout.
void closeConnection() {
    printf("\nClosing Connection with file descriptor: %d \n", clientSocket);
    close(clientSocket);
    exit(1);
}

//Initialize Socket for server communication
void initClientSocket() {
    struct sockaddr_in serverAddress;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;

    // INADDR_ANY means we will listen to any address
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    // htonl and htons converts address/ports to network formats
    serverAddress.sin_port = htons(RESOURCE_SERVER_PORT);

    // Bind to port
    if (bind(clientSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) == -1) {
        printf("Unable to bind to port just yet, perhaps the connection has to be timed out\n");
        exit(-1);
    }

    // Before we listen, register for Ctrl+C being sent so we can close our connection
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = closeConnection;
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    // Listen and queue up to 10 connections
    listen(clientSocket, 10);
}

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

    //if valid request, find fileName, and fileSize/fileContent if applicable
    if (request.requestType != 0) {
        for (int i = startingCharacter; i < BUF_SIZE; ++i) {
            //first breakpoint, check for filename
            if(userInput[i] == ' '|| userInput[i] == '\0') {
                request.requestFileName = getSubstring(startingCharacter, i, userInput, 0);

                //if read, go to second and third breakpoint
                if (request.requestType == 1) {
                    startingCharacter = i;
                    while (userInput[i] != ':') i++;

                    //second breakpoint
                    char* requestSize = getSubstring(startingCharacter, i, userInput, 1);
                    request.requestSize = atoi(requestSize);
                    free(requestSize);

                    //bypass character : straight to the first character of the file
                    startingCharacter = i ++;
                    while (userInput[i] != '\0') i++;

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

void* beginRequestThread(void* clientConnection) {
    int connectionToClient = *(int *)clientConnection;
    char receiveLine[BUF_SIZE];
    char userInput[BUF_SIZE];

    int bytesReadFromClient = 0;

    // Read the request that the client has
    while ((bytesReadFromClient = read(connectionToClient, receiveLine, BUF_SIZE)) > 0) {
        // Need to put a NULL string terminator at end
        receiveLine[bytesReadFromClient] = 0;

        // Show what client sent
        printf("Received: %s\n", receiveLine);

        // Print text out to buffer, and then write it to client (connfd)
        snprintf(userInput, sizeof(userInput), "true");
//        printf("%Sending s\n", userInput);
//        write(connectionToClient, userInput, strlen(userInput));

        // Zero out the receive line so we do not get artifacts from before
        bzero(&receiveLine, sizeof(receiveLine));
        close(connectionToClient);
    }
    strncat(userInput, "\0", 1);

    Request request = interpretUserInput(userInput);

    switch (request.requestType) {
        case 1: saveFile(request);
        case 2: readFile(request);
        case 3: deleteFile(request);
    }

    free(request.requestContent);
    free(request.requestFileName);

    currentRequests = currentRequests -1;
}

//communicate with server
char* sendTCPRequest(int forFileServer, char* request,int lenRqst) {
    int  serverSocket, bytesRead;

    char sendLine[BUF_SIZE];
    char receiveLine[BUF_SIZE];

    // Setup server connection
    struct sockaddr_in serverAddress;
    bzero(&serverAddress, sizeof(serverAddress));

    // Create socket to server
    if ( (serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Unable to create socket\n");
        return "error";
    }

    // Setup the type of connection and where the server is to connect to
    serverAddress.sin_family = AF_INET; // AF_INET - talk over a network, could be a local socket
    serverAddress.sin_port   = htons((forFileServer ==1 )? 1028: 1029);

    if (inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr) <= 0) {
        printf("Unable to convert IP for server address\n");
        return "error";
    }

    // Connect to server, if we cannot connect, then exit out
    if (connect(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        printf("Unable to connect to server");
    }

    snprintf(sendLine, sizeof(sendLine), request);

    // Write will actually write to a file (in this case a socket) which will transmit it to the server
    write(serverSocket, sendLine, lenRqst);

    // Now start reading from the server
    // Read will read from socket into receiveLine up to BUF_SIZE
    while ( (bytesRead = read(serverSocket, receiveLine, BUF_SIZE)) > 0) {
        receiveLine[bytesRead] = 0; // Make sure we put the null terminator at the end of the buffer
        printf("Received %d bytes from server with message: %s\n", bytesRead, receiveLine);

        // Got response, get out of here
        break;
    }

    // Close the server socket
    close(serverSocket);

    strncat(receiveLine, "\0", 1);

    return receiveLine;
}

////send data to the server
//int sendToServer(int serverSocket ,char* requestType,int lenRqst) {
//    struct timeval tv;
//    tv.tv_sec = 20;  /* 20 Secs Timeout */
//    tv.tv_usec = 0;
//
//    //if the server request times out, return a -1 as a send request cannot have a size of -1
//    if(setsockopt(serverSocket,SOL_SOCKET,SO_SNDTIMEO,(char *)&tv,sizeof(tv)) < 0)
//        return -1;
//
//    return send(serverSocket, requestType, lenRqst, 0);
//}
//
////receive data from the server
//int receiveFromServer(int serverSocket, char* Rsp, int RvcSize) {
//    struct timeval tv;
//    tv.tv_sec = 20;
//    tv.tv_usec = 0;
//
//    //if the server request times out, return a -1 as a recv request cannot have a size of -1
//    if(setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO,(char *)&tv,sizeof(tv)) < 0)
//        return -1;
//
//    return recv(serverSocket, Rsp, RvcSize, 0);
//}

//***********
//Main Method
//***********
int main(int argc, char *args[]) {
    //set how many threads can occur at a given time, only need to keep an int since there's no requirement
    // to print request details outside of its own thread
    allowedRequests = retrieveTotalAllowedThreads();

    int connectionToClient;
    initClientSocket();

    while(1) {
        connectionToClient = accept(clientSocket, (struct sockaddr *) NULL, NULL);

        //if a new request won't go over the thread limit
        if (currentRequests+1 <= allowedRequests) {
            currentRequests ++;

            pthread_t thread_id;
            pthread_create(&thread_id, NULL, beginRequestThread, (void*)&connectionToClient);
        }
        else printf("Please wait for a request to open up.");
    }
}
