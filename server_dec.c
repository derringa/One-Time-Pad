/********************************************************************
 * Program: 		One-Time Pads
 * Author: Andrew 	Derringer
 * Last Modified: 	12/6/2019
 * Summary:		Params:	[1] Desired port
 * 			This client establishes a socket connection
 * 			on the passed port number and listens as a
 * 			background process. It establishes up to 5
 * 			connections where text and key are received
 * 			and used to encrypt message along with client
 * 			program signature.
********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#define BUF_S 100000


// struct allowing us to send multiple pieces
// of information as arguments to a thread.
struct con_package {
   int conFD;
   int* active_threads;
};


// Error function used for reporting issues
void error(const char *msg) { perror(msg); exit(1); }


// thread functino
void *enc_service(void *args) {

   // Unpack the arguments passed
   struct con_package *pack;
   pack = (struct con_package*) args;
   int establishedConnectionFD = pack->conFD;
   int* active_threads = pack->active_threads;
   char buffer[BUF_S * 2];
   char chunk[BUF_S * 2];
   char sendBuffer[BUF_S];
   int charsRead;

   // active thrad count shared by reference and incremented
   (*active_threads)++;

   memset(buffer, '\0', sizeof(buffer));     
   while(1) {
      //printf("in the loop!");
      memset(chunk, '\0', sizeof(chunk));
      charsRead = recv(establishedConnectionFD, chunk, sizeof(chunk), 0); // Read the client's message from the socket
      if (charsRead < 0) { // if error reading message manage memory and exit thread
         (*active_threads)--;
         pack->active_threads = NULL;
         free(pack);
         close(establishedConnectionFD);
         fprintf(stderr, "ERROR reading from socket\n");
         pthread_exit(NULL);
      } else if (strchr(chunk, '!') != NULL) { // we've reading the end, add chunk and break
         strcat(buffer, chunk);
         break;
      } else { // running smoothly but haven't reached the end
         strcat(buffer, chunk);
      }
   }

   // strtok strips our delimiters for us
   char* text = strtok(buffer, "*"); // place encrypted text in text
   char* key = strtok(NULL, "*"); // place key in key
   char* delim = strtok(NULL, "!"); // place signature to check for valid connection
   char* client;

    memset(sendBuffer, '\0', sizeof(sendBuffer)); 
   if ((client = strchr(delim, 'd')) == NULL) {
      strcpy(sendBuffer, "ERROR attempted connection with otp_dec_d$");
      charsRead = send(establishedConnectionFD, sendBuffer, sizeof(sendBuffer), 0); // Send success back
   } else {
      int textLength = strlen(text);
      //printf("Loop/enc length = %d\n", textLength);
      int i, temp, keyval;
      for (i = 0; i < textLength; i++) {
         temp = (text[i] - 64);
         keyval = (key[i] - 64);
         if (temp < 1) {
            temp = 27;
         }
         if (keyval < 1) {
            keyval = 27;
         }

         temp -= keyval;
         if (temp < 1) {
            temp += 27;
         }

         if (temp == 27) {
            temp = 32;
         } else {
            temp += 64;
         }
         sendBuffer[i] = (char) temp;
      }
      sendBuffer[strlen(sendBuffer)] = '!';

      //printf("SERVER: Sending this out: %s\n", sendBuffer);
      // Send a Success message back to the client
      charsRead = send(establishedConnectionFD, sendBuffer, sizeof(sendBuffer), 0); // Send success back
      if (charsRead < 0) {
         (*active_threads)--;
         pack->active_threads = NULL;
         free(pack);
         close(establishedConnectionFD);
         fprintf(stderr, "ERROR writing to socket\n");
         pthread_exit(NULL);
      }
   }
      close(establishedConnectionFD); // Close the existing socket which is connected to the client

      //pthread_detach(pthread_self());
      (*active_threads)--;
      pack->active_threads = NULL;
      pthread_detach(pthread_self());

}


int main(int argc, char *argv[]) {
   int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
   socklen_t sizeOfClientInfo;
   struct sockaddr_in serverAddress, clientAddress;
   struct con_package *pack;
   pthread_t thread;
   int t = 0, *active_threads = &t;

   if (argc < 2) {
      fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1);
   } // Check usage & args

   // Set up the address struct for this process (the server)
   memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
   portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
   serverAddress.sin_family = AF_INET; // Create a network-capable socket
   serverAddress.sin_port = htons(portNumber); // Store the port number
   serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

   // Set up the socket
   listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
   if (listenSocketFD < 0) {
   error("ERROR opening socket");
   }

   // Enable the socket to begin listening
   if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {// Connect socket to port
      error("ERROR on binding");
   }

   listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

   while (1) {
      // Accept a connection, blocking if one is not available until one connects
      sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
      establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
      if (establishedConnectionFD < 0) {
         error("ERROR on accept");
      }

      // if FD connection established build a package to send to thread
      pack = (struct con_package*) malloc(1 * sizeof(struct con_package));
      pack->conFD = establishedConnectionFD;
      pack->active_threads = active_threads;

      if( *active_threads < 5) { // only create new thread if less than 5 active threads
         if(pthread_create(&thread, NULL, enc_service, pack) != 0) {
            error("Error on thread create");
         }
      } else {
         fprintf(stderr, "ERROR too many concurrent socket connections\n");
      }

   }

   close(listenSocketFD); // Close the listening socket
   return 0;

}
