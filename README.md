# Simple-File-Transfer
This project includes a client and a server capable of performing a simple file transfer from the client to the server.

# Tasks
## Client
The client will loop, asking the user for the name of a file to transfer to the server.
With each filename, the client will:
- Send the size of the name of the file to be saved
- Send the name of the file to be saved
- Send the filesize to the server (files will be less than 1GB in size)
- Send all the bytes (less than 1000 bytes at a time) to the server
- Wait for an ACK from the server that tells the client how many total bytes the server received
- When the file name “DONE” is entered, or "ctrl + c", the client will exit the loop and close the socket

## Server
The server will wait for an incoming connection from a client. Once the connection is established, it will loop and do the following:
- receive the size of the filename to be received
- receive the name of the file to be received
- received the filesize (in bytes)
- loop receiving all the bytes for that file and writing them to disk
- once all the bytes have been received it will send the total bytes received back to the server
- It will continue to wait for that client to send data
- If the connection is closed by the client (meaning read() returned 0) the server will close the connected socket and go back to wait for another client to connect

# Test
1. C Make
2. Open a server in a new terminal: `./server 1818` (for example, using port 1818)
   - Then the server will start listening to a connection
3. Open a client in a new terminal: `./client 127.0.0.1 1818` (using the localhost IP)
   - Then the connection will be established and prompted
4. Enter the file name on the client side: `testPicture.jpg` for instance
5. Check whether the file is correctly transferred:
   - By checking the prompt, including packet size, ACK, and other useful info
   ![image](https://github.com/MacbookWithoutBook/Simple-File-Transfer/assets/74396454/2d05cbdd-fb04-44b7-8f52-a8f500fc0fcb)
   ![image](https://github.com/MacbookWithoutBook/Simple-File-Transfer/assets/74396454/1bb79dba-fe92-4912-98d7-79ad6ccf0acf)
   - By checking the "outputFile_originalFilename" file, which was just written to the current folder
   ![image](https://github.com/MacbookWithoutBook/Simple-File-Transfer/assets/74396454/bd66b5a5-8cc1-4c59-834c-6cb78424732f)

6. Close the client by entering "DONE" or using "ctrl + c"
7. Close the server by "ctrl + c" if you want to end the program

