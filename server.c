/*Server*/
/*Written by Peitong Zhu, Aug 2023*/

#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFSIZE 1000 /*Randomly defined buffer size for reading file data.*/

int main (int argc, char *argv[]) {
    int portNumber;
    int listening_sd; //listen for incoming connection requests from clients,doesn't handle data transfer
    int connected_sd; //connected socket is generated once the listening socket hears a connection
    int rc = 0; //return code, stores how many bytes are read; also used for error checking
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    socklen_t clientLength;
    char buffer[100];

    /*Check terminal input*/
    if (argc < 2) {
        perror("Input Error. The format should be: server <portnumber>\n");
        exit(1);
    }

    portNumber = atoi(argv[1]); //atoi(): ASCII to Integer
    /*Initialize the listening socket*/
    listening_sd = socket(AF_INET, SOCK_STREAM, 0); //AF_INET means IPv4 is used; SOCK_STREAM means this is a TCP socket; 0 allows the system to choose the proper protocol
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(portNumber); //htons: host to network short, to make byte order consistent
    server_address.sin_addr.s_addr = INADDR_ANY; //this line specifies the IP address on which the server will listen for incoming connections; INADDR_ANY requires the server to listen all available IP address

    /*---------------------Connect socket and check error---------------------*/
    if (bind(listening_sd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Error connecting socket\n");
        exit(1);
    }

    
LOOP1:
    /*Starts listening connections*/
    printf("********Server: Start listening connections********\n");
    listen(listening_sd, 5); //5 is the max pending connections that the server can queue up
     
    /*accept() is a system call that accepts a incoming client connection;
    It blocks the server's execution until a client connects;
    When a client is connected, it returns a new connected socket for data transfer;
    The original listening socket remains in listening state
    */
    connected_sd = accept(listening_sd, (struct sockaddr *)&client_address, &clientLength);
    printf("Server: Successfully connect to a Client\n");

LOOP2:
    /*-----------------Task #1: Read the size of filename-----------------*/
    printf("\nWaiting for a new flename......\n");
    int sizeOfFilename;
    rc = read(connected_sd, &sizeOfFilename, sizeof(int));
    /*Check whether read successfully*/
    if (rc < 0) {
        perror("Server Error: read size of filename incorrectly\n");
        exit(1);
    } else if (rc == 0) {
        printf("Connection is closed by client(ctrl + c). Current socket is closed and wait for new connections\n\n");
        close(connected_sd);
        goto LOOP1;
    }

    sizeOfFilename = ntohs(sizeOfFilename);
    if (sizeOfFilename == 0) {
        printf("Connection is closed by client(DONE entered). Current socket is closed and wait for new connections\n\n");
        close(connected_sd);
        goto LOOP1;
    }

    printf("Server: Receive size of filename: %d bytes\n", sizeOfFilename);



    /*-----------------Task #2: Read the filename-----------------*/
    int receivedBytesOfFilename = 0;
    //Usually, filename is small and can be sent from the client in one single data packet, while loop is not needed
    //But while loop is used here to handle cases where the file name data might arrive in multiple packets due to network considerations or the way data is sent
    while (receivedBytesOfFilename < sizeOfFilename) {
        // rc stores the number of bytes read from the socket during each read system call
        //the 2nd parameter in read() is where the data being read will be stored
        //the 3rd parameter is the number of bytes to read
        rc = read(connected_sd, buffer + receivedBytesOfFilename, sizeOfFilename - receivedBytesOfFilename);
        if (rc < 0) {
            perror("Server Error: read filename incorrectly\n");
            exit(1);
        }

        /*Update by how many bytes are read*/
        receivedBytesOfFilename += rc;
    }
    printf("Server: Successfully receive the file '%s'\n", buffer);


    /*------------------Task #3: Read the file size------------------*/
    int fileSize = 0;
    rc = read(connected_sd, &fileSize, sizeof(int));
    if (rc < 0) {
        perror("Server Error: read file size incorrectly\n");
        exit(1);
    }
    printf("Server: Successfully receive the file size of %d bytes\n", fileSize);


    /*------------------Task #4: Loop receiving all bytes for the file and write them to disk------------------*/
    int totalBytesReceived = 0;

    /*Set the output file name and open it, with error checking*/
    FILE *outputFile;
    char outputFileName[112] = "outputFile_"; // buffer size 100 + "outputFile_" = 111
    strcat(outputFileName, buffer); //buffer currently stores filename
    outputFile = fopen(outputFileName, "wb");
    if (outputFile == NULL) {
        printf("Error: Failed to open outputFile.\n");
        exit(1); 
    }

    /*If the filename is correctly used, reset the buffer*/
    bzero(buffer, 100);

    /*Read the data by 1000 Bytes each time*/
    while (totalBytesReceived < fileSize) {
        char dataBuffer[BUFFSIZE];
        bzero(dataBuffer, BUFFSIZE);
        
        /*Read data by 1000 bytes a time*/
        rc = read(connected_sd, dataBuffer, BUFFSIZE);
        if (rc <= 0){
            perror("Read error while receving file in loop. (May because termination of Client.)\n Exit program");
            exit(1);
        }
        printf("\tIn loop, read file package: %d bytes.\n", rc);

        /* sum the total bytes received according to ACK response*/
        totalBytesReceived += rc;

        /* write to disk */
        rc = fwrite(dataBuffer, 1, rc, outputFile);
        if (rc <= 0){
            perror("I/O error while writing file to outputFile.\n");
            exit(1);
        }
    }
    printf("Server: Total bytes received: %d bytes.\n", totalBytesReceived);
    fclose(outputFile);


    /*------------------Task #5: Send ACK to the client------------------*/
    rc = write(connected_sd, &totalBytesReceived, sizeof(int));
    if (rc < 0) {
        perror("Server Error: write ACK\n");
        exit(1);
    }
    printf("Server: ACK sent\n");


    /*Keep listening for the next filename*/
    goto LOOP2;
    return 0;
}