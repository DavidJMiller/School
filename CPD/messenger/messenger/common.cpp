/*###############################
  #        David Miller         #
  #       COP 5570 - CPD        #
  #         Project 2           #
  #        Nov 29 2018          #
  ###############################*/

#include "common.h"
#include <iostream>
#include <string.h>                                /* strerror */

/* 
   err: enum type corresponding to what error occured
   msg: useful msg to be displayed about error
*/
void printError(enum Error err, const std::string msg, const char *who)
{
    std::cerr << who;
    switch(err)
    {
        /* program was not called correctly */
        case INVALID_CALL:
            std::cerr << "Error: invalid call: " << msg << "\n";
            break;
        /* file was not found */
        case INVALID_FILE:
            std::cerr << "Error: can not open file \"" << msg << "\": "
                      << strerror(errno) << "\n";
            break;
        /* empty handle for user */
        case EMPTY_HANDLE:
            std::cerr << "Error: line " << msg << ": handle must be nonempty\n";
            break;
        /* empty key for user */
        case EMPTY_KEY:
            std::cerr << "Error: line " << msg << ": key must be nonempty\n";
            break;
        /* config file */
        case PORT_SYNTAX:
            std::cerr << "Error: " << msg << ": perhaps you meant \"port\"\n";
            break;
        /* out of range port number in config file */
        case NEGATIVE_PORT:
            std::cerr << "Error: port " << msg << ": must be non-negative\n";
            break;
        /* socket() failed */
        case SOCKET_FAIL:
            std::cerr << "Socket failed: " << strerror(errno) << "\n";
            break;
        /* bind() fail */
        case BIND_FAIL:
            std::cerr << "Bind to fd " << msg << " failed: " 
                      << strerror(errno) << "\n";
            break;
        /* listen() fail */
        case LISTEN_FAIL:
            std::cerr << "Listening with queue size " << MAX_BACKLOG 
                      << " failed: " << strerror(errno) << "\n";
            break;
        /* a hostname funtion fail */
        case HOSTNAME_FAIL:
            std::cerr << "Host name retrival fail: " << strerror(errno) << "\n";
            break;
        /* getsockname() fail */
        case SOCKNAME_FAIL:
            std::cerr << "Get socket address fail: " << strerror(errno) << "\n";
            break;
        /* getaddrinfo() fail */
        case ADDRINFO_FAIL:
            std::cerr << "Resolving " << msg << " addrinfo fail: "
                      << strerror(errno) << "\n";
            break;
        /* accept() fail */
        case ACCEPT_FAIL:
            std::cerr << "Accept on port " << msg << " failed: "
                      << strerror(errno) << "\n";
            break;
        /* pthread_create() fail */
        case CREATE_THREAD_FAIL:
            std::cerr << msg << "thread creation failed: " 
                      << strerror(errno) << "\n";
            break;
        /* read() fail */
        case READ_FAIL:
            std::cerr << "Read from client fail: " << strerror(errno) << "\n";
            break;
        /* connect fail() */
        case CONNECT_FAIL:
            std::cerr << "Connect attempt(s) failed\n";
            break;
        /* pthread_mutex_init() fail */
        case MUTEX_CREATION_FAIL:
            std::cerr << "Failed to create " << msg << " mutex\n";
            break; 
        /* failed to set port as reusable */
        case REUSEPORT_FAIL:
            std::cerr << "Falied to reuse port " << msg << ": " 
                      << strerror(errno) << "\n";
            break;
        /* something happened */
        default:
            std::cerr << "Unknown error encountered\n";
            break;
    }

    return;
}