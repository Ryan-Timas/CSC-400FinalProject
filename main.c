//This Program was written by Ryan and Ahmed

#include <stdio.h>
#include <sys/sysinfo.h>
#include <stdlib.h>
#include<sys/socket.h>
#include<string.h>
#include<arpa/inet.h>
#include <pthread.h>
#include<unistd.h>
#include <math.h>
#include <signal.h>

#define CLIENT_PORT 1027
#define FILE_SERVER_PORT 1028
#define MEMORY_SERVER_PORT 1029
#define BUF_SIZE 256

typedef struct {
    int requestType; //1 = save, 2 = read, 3 = delete, 0 = invalid request
    int requestSize; //requestType filename *n*:[contents of file]
    char *requestFileName; //requestType *filename* n:[contents of file]
    char *requestContent; //requestType n:[*contents of file*]
} Request;

char receiveLine[BUF_SIZE];

int clientSocket;
int serverSocket;

int allowedRequests;
int currentRequests;
//*********************
//Miscellaneous Methods
//*********************
//returns how many characters an int has
int digitsInInt(int value) {
    int digits = 1;
    while (value/10 > 0) {
        value = value/10;
        digits++;
    }
    return digits;
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
//communicate with server
void sendTCPRequest(char* request,int lenRqst, int forFileServer) {
    int  bytesRead;
    char sendLine[BUF_SIZE];

    //clear whatever was previously recieved from the server
    bzero(&receiveLine, sizeof(receiveLine));

    // Setup server connection
    struct sockaddr_in serverAddress;
    bzero(&serverAddress, sizeof(serverAddress));

    // Create socket to server
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Unable to create socket\n");
    }

    // Setup the type of connection and where the server is to connect to
    serverAddress.sin_family = AF_INET; // AF_INET - talk over a network, could be a local socket
    serverAddress.sin_port   = htons((forFileServer ==1 )? FILE_SERVER_PORT: MEMORY_SERVER_PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr) <= 0) {
        printf("Unable to convert IP for server address\n");
    }

    // Connect to server, if we cannot connect, then exit out
    if (connect(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        printf("Unable to connect to server\n");
    }

    snprintf(sendLine, sizeof(sendLine), "%s", request);

    printf("Sending Request %s with size %d to %s\n", sendLine, lenRqst, (forFileServer ==1 )? "FILE_SERVER_PORT": "MEMORY_SERVER_PORT");
    // Write will actually write to a file (in this case a socket) which will transmit it to the server
    write(serverSocket, sendLine, lenRqst);

    // Now start reading from the server
    // Read will read from socket into receiveLine up to BUF_SIZE
    bytesRead = read(serverSocket, receiveLine, BUF_SIZE);
    receiveLine[bytesRead] = 0;

    printf("Received %d bytes from server with message: %s\n", bytesRead, receiveLine);

    // Close the server socket
    close(serverSocket);
//    strncat(receiveLine, "\0", 1);
}

// We need to make sure we close the connection on signal received, otherwise we have to wait for server to timeout.
void closeConnection() {
    close(clientSocket);
}

//Initialize Socket for server communication
void initClientSocket() {
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in clientAddress;

    bzero(&clientAddress, sizeof(clientAddress));
    clientAddress.sin_family = AF_INET;

    // INADDR_ANY means we will listen to any address
    // htonl and htons converts address/ports to network formats
    clientAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    clientAddress.sin_port = htons(CLIENT_PORT);

    // Bind to port
    if (bind(clientSocket, (struct sockaddr *) &clientAddress, sizeof(clientAddress)) == -1) {
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

void returnToClient(int connectionToClient, char* request,int lenRqst) {
    char sendLine[BUF_SIZE];

    snprintf(sendLine, sizeof(sendLine), "%s", request);

    // Write will actually write to a file (in this case a socket) which will transmit it to the server
    printf("Returning %s to Client\n", request);
    write(connectionToClient, sendLine, lenRqst);

    close(connectionToClient);
}

//*******************
//User Input Commands
//*******************
Request interpretUserInput(char *userInput) {
    Request request;
    int startingCharacter = 5;
    for (int i = 0; i < sizeof(request); ++i) {

    }

    //Determine which request the user is trying to make
    char firstChar = userInput[0];
    if (firstChar == 's') {
        request.requestType=1;
    }

    else if (firstChar == 'r') {
        request.requestType=2;
    }

    else if (firstChar == 'd') {
        request.requestType=3;
        //since delete is a longer word than the other two...
        startingCharacter += 2;
    }

    else {
        request.requestType=0;
    }

    //if valid request, find fileName, and fileSize/fileContent if applicable
    if (request.requestType != 0) {
        for (int i = startingCharacter; i < BUF_SIZE; ++i) {
            //first breakpoint, check for filename
            if(userInput[i] == ' '|| userInput[i] == '\0') {
                i++;

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
                    startingCharacter = i +1;
                    while (userInput[i] != '\0') i++;

                    //third breakpoint
                    request.requestContent = getSubstring(startingCharacter, i+1, userInput, 0);
                }
                break;
            }
        }
    }


    return request;
}

void saveFile(Request request) {
    //set aside bytes for the 9 static request characters, the digits in the request size, and the size of the request
    char str[9 + digitsInInt(request.requestSize) + request.requestSize];

    //Create a string for the request you plan to send to the server
    sprintf(str,"write %s %d:%s",request.requestFileName, request.requestSize, request.requestContent);

    //send the request to the FileServer
    sendTCPRequest(str,request.requestSize,1);
}

void readFile(Request request, int *loadInMemory) {
    //set aside 5 bytes for static characters, then additional characters for the filename
    char str[5 + sizeof(request.requestFileName)];

    //format your server request
    sprintf(str,"load %s",request.requestFileName);

    //send this request to the memory server first, if it exist
    sendTCPRequest(str,sizeof(str),0);

    //if the file does not exist in memory, check the file server
    if (receiveLine[0] == '0') {
        //set the top level retrieveStatement
        sprintf(str,"read %s",request.requestFileName);
        sendTCPRequest(str,sizeof(str),1);

        //if the file exists on the file server, then load it into memory
        if (receiveLine[0] != 0) *loadInMemory = 1;
    }


//    sprintf(str,"serverRes %s\n",request.requestFileName);
}

void storeInMemory(Request request) {
    //set aside bytes for the 7 static request characters, the digits in the request size, and the size of the request
    char str[7 + sizeof(request.requestFileName) + sizeof(receiveLine)];

    //Create a string for the request you plan to send to the server
    sprintf(str,"store %s %s",request.requestFileName, receiveLine);

    //send the request to the FileServer and store the response
    sendTCPRequest(str,sizeof(str),0);
}

void deleteFile(Request request) {
    char str[7+ sizeof(request.requestFileName)];
    //Format your MemoryServer string and send it to the server

    sprintf(str,"rm %s",request.requestFileName);
    sendTCPRequest(str,sizeof(str),0);

    //Format your FileServer string and send it to the server
    sprintf(str,"delete %s",request.requestFileName);
    sendTCPRequest(str,sizeof(str),1);
}

void* beginRequestThread(void* clientConnection) {
    int connectionToClient = *(int *)clientConnection;

    if (currentRequests+1 <= allowedRequests) {
        currentRequests ++;
        printf("Requests In use %d \n", currentRequests);

        bzero(&receiveLine, sizeof(receiveLine));
        char userInput[BUF_SIZE];
        int bytesReadFromClient = 0;

        // Read the request that the client has
        bytesReadFromClient = read(connectionToClient, userInput, BUF_SIZE) > 0;
        userInput[bytesReadFromClient] = 0;

        Request request = interpretUserInput(userInput);

        //save Request
        if (request.requestType == 1) {
            saveFile(request);
            returnToClient(connectionToClient, "Successfully Saved", 18);
        }

            //Read Request
        else if (request.requestType ==2) {
            int loadInMemory = 0;

            readFile(request, &loadInMemory);

            //find the : character divider
            for (int i = 0; i< sizeof(receiveLine) ; ++i) {
                if (receiveLine[i] == ':') {
                    char* requestSize = getSubstring(0, i, receiveLine, 1);
                    int fileSize = atoi(requestSize);
                    free(requestSize);

                    int start = i;
                    while (receiveLine[i] != '\0') i++;

                    //get the content from the file
                    char* fileContent = getSubstring(start, i, receiveLine+1, 0);

                    //return the file to the client then clean when no longer needed
                    returnToClient(connectionToClient, fileContent, fileSize);
                    free(fileContent);
                    break;
                }
            }

            if (loadInMemory) storeInMemory(request);
        }
            //Delete Request
        else if (request.requestType == 3) {
            deleteFile(request);
            returnToClient(connectionToClient, "Successfully deleted", 20);
        }

        free(request.requestFileName);
        free(request.requestContent);
        currentRequests = currentRequests-1;
    }
    else returnToClient(connectionToClient, "Please wait for a request to open up.", 37);
}

//***********
//Main Method
//***********
int main(int argc, char *args[]) {
    //set how many threads can occur at a given time, only need to keep an int since there's no requirement
    // to print request details outside of its own thread
    allowedRequests = get_nprocs_conf() *2;

    int connectionToClient;
    initClientSocket();

    while(1) {
        connectionToClient = accept(clientSocket, (struct sockaddr *) NULL, NULL);

        if (connectionToClient == -1) break;

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, beginRequestThread, (void*)&connectionToClient);
    }
}