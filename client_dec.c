/********************************************************************
 * Program: 		One-Time Pads
 * Author: Andrew 	Derringer
 * Last Modified: 	12/6/2019
 * Summary:		Params:	[1] encrypted text file
 * 				[2] encrytpion key text file
 * 				[3] Desired Port
 * 			Cond:	[1] key must be >= text length
 * 				[2] text must contain only upper
 * 				case alpha, space, and trailing \n
 * 			This client establishes a socket connection
 * 			on the passed port number, passes it's text
 * 			and key, then listens to receive an unecrypted
 * 			version of it's message that is sent to
 * 			stdout.
********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <fcntl.h>
#define BUF_S 100000

 // Error function used for reporting issues
void error(const char *msg) { perror(msg); exit(0); }

int main(int argc, char *argv[]) {

   int socketFD, portNumber, charsWritten, charsRead;
   struct sockaddr_in serverAddress;
   struct hostent* serverHostInfo;

   // Confirms corrent number of args passed    
   if (argc < 3) {
      fprintf(stderr,"USAGE: %s hostname port\n", argv[0]);
      exit(1);
   }

   /***************************************
   * Open text and key files
   ***************************************/

   // Create plaintext FD an get size before allocating read buffer
   int textFD = open(argv[1], O_RDONLY); //read only
   if (textFD == -1) {
      fprintf(stderr, "CLIENT: Error, no such plaintext file\n");
      exit(1);
   }
   // Use of stat struct to get file size
   struct stat textST;
   fstat(textFD, &textST);
   int textSize = textST.st_size;

   // Repeat for key FD
   int keyFD = open(argv[2], O_RDONLY);
   if (keyFD == -1) {
      fprintf(stderr, "CLIENT: Error, no such key file\n");
      exit(1);
   }
   struct stat keyST;
   fstat(keyFD, &keyST);
   int keySize = keyST.st_size;

   //if plaintext > key length then error exit
   if (textSize > keySize) {
      fprintf(stderr, "CLIENT: Error, plaintext longer than key\n");
      exit(2);
   }

   /***************************************
   * Set up the server address struct
   ***************************************/

   memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
   portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
   serverAddress.sin_family = AF_INET; // Create a network-capable socket
   serverAddress.sin_port = htons(portNumber); // Store the port number
   serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
   if (serverHostInfo == NULL) {
      fprintf(stderr, "CLIENT: ERROR, no such host\n");
      exit(0);
   }
   memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

   /***************************************
   * Set up the socket
   ***************************************/

   // sockets maintained just as file descriptors
   socketFD = socket(AF_INET, SOCK_STREAM, 0);
   if (socketFD < 0) {
      error("CLIENT: ERROR opening socket");
   }
	
   // Connect to server using address struct
   if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
      error("CLIENT: ERROR connecting");
   }

   /***************************************
   * Read from text and key FDs
   * Summary: Confirm valid characters in
   * text, replace newlines with custom
   * delimiters (*) and combine with trailing
   * client signature (e) and end note (!)
   ***************************************/

   char *nl;
   int i;

   // Read FD into buffer
   char textBuffer[textSize]; // correct size from earlier stat struct
   memset(textBuffer, '\0', sizeof(textBuffer)); // clear buffer and read in file
   read(textFD, textBuffer, sizeof(textBuffer));
   for(i = 0; textBuffer[i] != '\n'; i++) {
      if(textBuffer[i] > 90 || textBuffer[i] < 65) { // only accept cap alpha and spaces
         if (textBuffer[i] != 32) {
            error("ERROR bad character");
         }
      }
   }
   nl = strchr(textBuffer, '\n'); // if valid replace trailing newline with custom delimiter
   *nl = '*';
   
   // Read key FD into buffer
   char keyBuffer[keySize];
   memset(keyBuffer, '\0', sizeof(keyBuffer));
   read(keyFD, keyBuffer, sizeof(keyBuffer));
   nl = strchr(keyBuffer, '\n');
   *nl = '*';

   // allocate buffer to fit max of text and key
   char buffer[BUF_S * 2];
   memset(buffer, '\0', sizeof(buffer));
   strcat(buffer, textBuffer); // push text into buffer
   strcat(buffer, keyBuffer); // push key into buffer
   strcat(buffer, "d!"); // push signature and end note into buffer

   /***************************************
   * Send message to server
   * Summary: Format of file output look like
   * "text"*"key"*e!
   ***************************************/

   // write to the server
   charsWritten = send(socketFD, buffer, strlen(buffer), 0);
   if (charsWritten < 0) {
      error("CLIENT: ERROR writing to socket");
   }
   if (charsWritten < strlen(buffer)) {
      fprintf(stderr, "CLIENT: WARNING: Not all data written to socket!\n");
   }

   /***************************************
   * Wait for return message from server
   ***************************************/

   memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
   char chunk[BUF_S]; //allocate buffer for grabbing chuncks if sent in multiple packates
   char *sp;

   while(1) {
      memset(chunk, '\0', sizeof(chunk)); // Clear out the buffer again for reuse
      charsRead = recv(socketFD, chunk, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
      if (charsRead < 0) { // complete error reading from socket
         error("CLIENT: ERROR reading from socket");
      } else if ((sp = strchr(chunk, '!')) != NULL) { // we've reached the end if ! is present
         *sp = '\0'; // remove ! for final output
         strcat(buffer, chunk);
         break;
      } else if ((sp = strchr(chunk, '$')) != NULL) { // $ indicates an error message returned
         strcat(buffer, chunk);
         *sp = '\n'; // replace trailing $ with newline for output to stderr
         fprintf(stderr, buffer);
         exit(2);
      } else { // rec is good but not at the end yet
         strcat(buffer, chunk);
      }
   }

   printf("%s\n", buffer);

   close(socketFD); // Close the socket
   return 0;

}
