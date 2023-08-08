# Simple-File-Transfer
This project includes a client and a server capable of performing a simple file transfer from the client to the server.

##Client
The client will loop, asking the user for the name of a file to transfer to the server.
With each filename, the client will
• Send the size of the name of the file to be saved
• Send the name of the file to be saved
• Send the filesize to the server (files will be less than 1GB in size)
• Send all the bytes (less than 1000 bytes at a time) to the server
• Wait for an ACK from the server that tells the client how many total bytes the server
received.
When the file name “DONE” is entered, the client will exit the loop and close the socket

##Server
The server will wait for an incoming connection from a client. Once the connection is
established, it will loop doing the following:
• receive the size of the filename to be received
• receive the name of the file to be received
• received the filesize (in bytes)
• loop receiving all the bytes for that file and writing them to disk
• once all the bytes have been received it will send the total bytes received back to the
server
• It will continue to wait for that client to send data
• If the connection is closed by client (meaning read() returned 0) the server will close the
connected socket and go back to wait for another client to connect


