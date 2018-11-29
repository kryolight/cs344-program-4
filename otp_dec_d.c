#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char const *argv[])
{
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	char buffer[102400];
	struct sockaddr_in serverAddress, clientAddress;

	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

	while(1)
	{
		// Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) error("ERROR on accept");

		int pid = fork();
		int exitStatus;

		char message[102400];
		char plainText[50000];
		char key[50000];
		char signature[2400];
		char cipherText[50000];
		memset(message, '\0', sizeof(message));
		memset(plainText, '\0', sizeof(plainText));
		memset(key, '\0', sizeof(key));
		memset(signature, '\0', sizeof(signature));
		memset(cipherText, '\0', sizeof(cipherText));

		int cipherTextLength = 0;
		//int keyLength = 0;
		int plainNum;
		int keyNum;
		int cipherNum;
		int i;

		switch (pid)
		{
			case -1:
				fprintf(stderr, "Forking error\n");
				exit(1);
			case 0:
				// Get the message from the client and display it
				memset(message, '\0', sizeof(message));
				charsRead = recv(establishedConnectionFD, message, sizeof(message), 0); // Read the client's message from the socket
				if (charsRead < 0) error("ERROR reading from socket");
				//printf("SERVER: I received this from the client: \"%s\"\n", message);

				// encoding code
				sscanf(message, "%[^'$']$%[^'$']$%s", cipherText, key, signature);

				//printf("%s\n", cipherText);
				//printf("%s\n", key);
				//printf("%s\n", signature);

				cipherTextLength = strlen(cipherText);
				
				for (i = 0; i < cipherTextLength; ++i)
				{
					cipherNum = cipherText[i];
					if (cipherNum == 32)
					{
						cipherNum = 0;
					}
					else
					{
						cipherNum -= 64;
					}

					keyNum = key[i];
					if (keyNum == 32)
					{
						keyNum = 0;
					}
					else
					{
						keyNum -= 64;
					}

					plainNum = (27 + cipherNum - keyNum) % 27;

					if (plainNum == 0)
					{
						plainText[i] = ' ';
					}
					else
					{
						plainText[i] = plainNum + 64;
					}
				}

				//printf("%s\n", plainText);

				// Send a Success message back to the client
				//charsRead = send(establishedConnectionFD, "I am the server, and I got your message", 39, 0); // Send success back
				charsRead = send(establishedConnectionFD, &plainText, strlen(plainText), 0); // Send success back
				if (charsRead < 0) error("ERROR writing to socket");
			default:
				close(establishedConnectionFD); // Close the existing socket which is connected to the client			
				do
				{
					waitpid(pid, &exitStatus, 0);
				} while (!WIFEXITED(exitStatus) && !WIFSIGNALED(exitStatus));
		}
	}

	

	// Get the message from the client and display it
	// memset(buffer, '\0', 256);
	// charsRead = recv(establishedConnectionFD, buffer, 255, 0); // Read the client's message from the socket
	// if (charsRead < 0) error("ERROR reading from socket");
	// printf("SERVER: I received this from the client: \"%s\"\n", buffer);

	// Send a Success message back to the client
	// charsRead = send(establishedConnectionFD, "I am the server, and I got your message", 39, 0); // Send success back
	// if (charsRead < 0) error("ERROR writing to socket");
	//close(establishedConnectionFD); // Close the existing socket which is connected to the client
	close(listenSocketFD); // Close the listening socket
	return 0;
}