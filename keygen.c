/********************************************************************
 * Program:		Basic Encrytion
 * Author:		Andrew Derringer
 * Last Modified:	11/30/2019
 * Param:		Int for size of output string
 * Summary:		Upon execution keygen takes one command line
 * 			arg int to create a specified buffer size
 * 			that is filled randomly with capital alpha
 * 			characters or spaces and returned via stdout.
********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**************************************
 * Summary:	Generate random int in range
 * 		and convert to char.
 * Output:	Random Char in range.
**************************************/
char getRandChar() {

   // get rand in between 0 - 26
   //int randChar = (rand() / (RAND_MAX / 27 + 1));
   int randChar = rand() % (27 + 1 - 1) + 1;
   char retVal;

   // 26 reserved for space char
   if (randChar == 27) {
      retVal = 32;
   }
   // 0 - 25 for 26 cap letters in alpha
   else {
      retVal = randChar + 64;
   }

   return retVal;
}


/**************************************
 * Summary:	Chaeck for valid args from
 * 		cmd line. Create string buffer
 * 		of specified length and fill
 * 		with rand letters.
 * Param:	int for string buffer length
 * Output:	String randomly filled with cap
 * 		letters and spaces.
**************************************/
int main(int argc, char* argv[]) {

   // Only accept correct number of arguments
   if (argc <= 1) {
      fprintf(stderr, "Error: not enough arguments passed to keygen.\n");
      exit(1);
   } else if (argc > 2) {
      fprintf(stderr, "Error: too many arguments passed to keygen.\n");
      exit(1);
   }

   // prepare rand num generation and empty buffer
   srand(time(0));
   int keyLength = atoi(argv[1]);
   char buffer[keyLength + 1];
   memset(buffer, '\0', sizeof(buffer));

   // loop through buffer and call function to fill with rand char
   int i, temp;
   for (i = 0; i < keyLength; i++) {
      buffer[i] = getRandChar();
   }
   //buffer[keyLength] = '\n';

   // Send buffer content to stdout and end
   printf("%s\n", buffer);

   return 0;
}
