/*Client*/
/*Written by Peitong Zhu, Aug 2023*/

#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFSIZE 1000


char filename[100];
FILE *inputFile;

int main (int argc, char *argv[]) {
    /*Decalre variables needed for file transfer*/
    int sd; //socket descriptor: an integer to identify the client socket
    struct sockaddr_in server_address; //store info about IPV4 socket address
    int portNumber;
    char serverIP[29];
    int rc = 0; // return code, stores bytes read/send each system call; rc is also used for error checking
    int DONE = 0;

    /*Check terminal input*/
    if (argc < 3) {
        printf("Input error. The format should be: client <ip address> <port>\n");
        exit(1);
    }

    /*--------------------Create TCP Socket--------------------*/
    /*socket() function creates a new TCP socket(because SOCK_STREAM is used), and returns an integer descriptor to sd*/
    /*AF_INET specifies that the socket should use the IPV4 address family*/
    sd = socket(AF_INET, SOCK_STREAM, 0);
    /*strtol() converts a string to a long integer*/
    portNumber = strtol(argv[2], NULL, 10);
    strcpy(serverIP, argv[1]);
    /*Update server_address info in order to use connect() later*/
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(portNumber); //htons converts little-endian to big-endian, for consistancy
    server_address.sin_addr.s_addr = inet_addr(serverIP);

    /*---------------------Connect socket and check error---------------------*/
    if (connect(sd, (struct sockaddr *)&server_address, sizeof(struct sockaddr_in)) < 0) {
        close(sd);
        perror("Error connecting socket\n");
        exit(1);
    }


    /*Loop until the user enters DONE*/
    while (strcmp(filename, "DONE") != 0) {
        printf("Enter file name: ");
        scanf("%s", filename);

        if (strcmp(filename, "DONE") != 0) {
            inputFile = fopen(filename, "rb"); //open the file
            /*check whether file entered is valid*/
            if (inputFile == NULL) {
                perror("Error, please try again");
                continue;
            }
        } else {
            /*If DONE is entered, end the client*/
            printf("DONE is entered, close socket\n");
            rc = write(sd, &DONE, sizeof(int));
            close(sd);
            exit(0);
        }
        

        /*if valid, prompt related info and start sending the file*/
        printf("********Client: start sending file********\n");

        /*------------------Task 1: Send the size of filename to server------------------*/
        int sizeofFileName = strlen(filename);
        int convertedSizeofFileName = ntohs(sizeofFileName);
        rc = write(sd, &convertedSizeofFileName, sizeof(convertedSizeofFileName)); //write() requires the second parameter to be a pointer
        if (rc < 0) {
            perror("Client Error: write size of filename incorrectly\n");
            exit(1);
        }
        printf("Client Task #1: Send size of filename: %d bytes.\n", sizeofFileName);


        /*------------------Task 2: Send the file name to server------------------*/
        rc = write(sd, filename, sizeofFileName);
        if (rc < 0) {
            perror("Client Error: write filename incorrectly\n");
            exit(1);
        }
        printf("Client Task #2: Send filename: %s.\n", filename);


        /*------------------Task #3: Send the file size to server------------------*/
        /* seek to the end of the file and then ask for the position, to get filesize */
        fseek(inputFile, 0L, SEEK_END);
        int fileSize = ftell(inputFile);
        /* Check whether the file size is larger than 1 GB (2^30 bytes) */
        if (fileSize >= 1073741824) {
            perror("For reliability reason, file size should be less than 1 GB.\nPlease try a smaller file:");
            continue;
        }
        /* seek back to the beginning, for future reading */
        rewind(inputFile);

        /* Send filesize and error checking. */
        rc = write(sd, &fileSize, sizeof(fileSize));
        if (rc < 0) {
            perror("Client Error: write file size incorrectly\n");
            exit(1);
        }
        printf("Client task #3: Send file_size: %d bytes\n", fileSize);


        /*------------------Task #4: Send the file content to server------------------*/
        char dataBuffer[BUFFSIZE];
        int totalBytesSent = 0;
        int currentLoopReadBytes = 0; // bytes read each loop

        while (totalBytesSent < fileSize) {
            currentLoopReadBytes = fread(dataBuffer, 1, BUFFSIZE, inputFile);
            if (currentLoopReadBytes < 0) {
                perror("Client Error: fread() file incorrectly\n");
                exit(1);
            }

            rc = write(sd, dataBuffer, currentLoopReadBytes);
            if (rc < 0) {
                perror("Client Error: write file data incorrectly\n");
                exit(1);
            }
            printf("\tIn client loop, send a data packet of %d bytes...\n", rc);

            /*Update totalBytesSent*/
            totalBytesSent += currentLoopReadBytes;
        }
        printf("Client task #4: Successfully send the whole file(%d bytes) to the server\n", totalBytesSent);


        /*------------------Task #5: Wait ACK from the server------------------*/
        int totalBytesReceived_ACK = 0;
        rc = read(sd, &totalBytesReceived_ACK, sizeof(int));
        if (rc < 0) {
            perror("Client Error: read ACK incorrectly\n");
            exit(1);
        }
        /*If the total bytes sent from client is different than total bytes received by server, error*/
        if (totalBytesReceived_ACK == totalBytesSent) {
            printf("ACK received: total bytes sent equals to total bytes sent. Great!\n");
        } else {
            perror("Error: ACK received, but total bytes sent is different than total bytes received. Bad!\n");
            exit(1);
        }
    }

    return 0;
}