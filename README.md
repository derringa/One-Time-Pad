# One-Time-Pad #
Files include a ciphertext generator and the client and server daemons for performing both text encryption and decryption.

## Ciphertext Generator ##
The keygen program autogenerates and prints to standard output a list of english capital letters and spaces according to the length specified by the user up to the maximum capacity of the int data type. The output should be redirected to a file for use with the encryption and decryption clients. 
### Requirements ###
* A cipher text must be at least as long as the text it is encrypting or decrypting in order for either application to work.

## Encryption and Decryption Server Daemons ##
Each daemon is run in the background and establishes a listening socket on an unused port of the user's specification. The listening socket receive is looped until a specified message end tag is received at which time the text and cypher text are seperated and either encrypted or decrypted using modulo 27 operations. Each is capable of multi-processing up to 5 connections at a time.
### Requirements ###
* Server daemon must me established on an unused port.
* Each program must be running as a background process using the & command operator.
* Each will only connect to the client created for its purpose, either encrytion or decryption.

## Encryption and Decryption Clients ##
Each client receives a text, encrypted or decrypted depending on the client, along with the cypher text and port number for which their respective server daemon is actively listening. Upon connection, it strings together the text, cyphertext, and personalized ending tag or signature to be sent to the server.
### Requirements ###
* The program will exit if the cypher text is not at least as long as the text received.
* Each client produces its own unique signature that it's respective server expects to read in order to process the information.
* Port number provided must match that of the server daemon.

## Set-Up ##
```
$ compileall
$ cat testtext
HELLO MY NAME IS ANDREW DERRINGER
$ keygen 50 > cyphertext
$ cat cyphertext
SACTQHHENWXMSL NAREWREYGLCIXKCJSSLKXIGBVBYGTYWVYMZ
$ otp_enc_d 57171 &
$ otp_dec_d 58181 &
$ otp_enc testtext cyphertext 57171 > encryption
$ cat encryption
FA VNC XPKRBMRDXRZM CQKATPSOFKB K
$ otp_dec encryption cyphertext 58181
HELLO MY NAME IS ANDREW DERRINGER
```
