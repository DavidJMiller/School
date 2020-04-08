/*###############################
  #        David Miller         #
  #       COP 5570 - CPD        #
  #         Project 2           #
  #        Nov 29 2018          #
  ###############################*/

#ifndef COMMON_HEADER
#define COMMON_HEADER

#define HOSTNAME_MAX 128                           /* max len of hostname */ 
#define MAX_BUFLEN 512                             /* mac len of read/writes */
#define MAX_BACKLOG 5                              /* max backlog for listen */
#define CLIENT_PORT 6737                           /* client port number */
#define EMPTY_HANDLE_ALLOWED false                 /* allow empty handle */
#define EMPTY_KEY_ALLOWED false                    /* allow empty key */
#define SERVER "Server: "                          /* error came from server */
#define CLIENT "Client: "                          /* error came from client */

#include <string>

enum Action{INACTIVE,                              /* user not doing anything */
            REGISTER,                              /* user registering */
            LOGIN,                                 /* user logging in */
            LOGOUT,                                /* user logging out */
            MESSAGE,                               /* user messaging */
            INVITE,                                /* user inviting */
            ACCEPT_INVITE};                        /* user accepting invite */

enum Error{INVALID_CALL,                           /* invalid call to program */
           INVALID_FILE,                           /* file not found */
           EMPTY_HANDLE,                           /* handle is empty */
           EMPTY_KEY,                              /* key is empty */
           PORT_SYNTAX,                            /* check for "port: value" */
           NEGATIVE_PORT,                          /* negative port number */ 
           SOCKET_FAIL,                            /* socket creation fail */
           BIND_FAIL,                              /* ip <--> fd bind fail */
           LISTEN_FAIL,                            /* fail listening setup */
           HOSTNAME_FAIL,                          /* fail to get host name */
           SOCKNAME_FAIL,                          /* fail to get socket addr */
           ADDRINFO_FAIL,                          /* fail to get addrinfo */
           ACCEPT_FAIL,                            /* fail to accept client */
           CREATE_THREAD_FAIL,                     /* fail to create thread */
           READ_FAIL,                              /* fail ro read client */
           CONNECT_FAIL,                           /* fail to connect */
           MUTEX_CREATION_FAIL,                    /* fail to create mutex */ 
           REUSEPORT_FAIL};                        /* fail to reuse port */

void printError(enum Error, const std::string,     /* print errors */
                const char *);    

#endif
