/*###############################
  #        David Miller         #
  #       COP 5570 - CPD        #
  #         Project 2           #
  #        Nov 29 2018          #
  ###############################*/

#include "common.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <netdb.h>                                 /* addrinfo */
#include <set>
#include <signal.h>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <typeinfo>
#include <unistd.h>
#include <unordered_map>
#include "user.h"
#include <vector>

std::string *parseConfigFile(char  *);             /* parse config file */
void *handleInput(void *);                         /* handle input from user */
void *handleServer(void *);                        /* handle server comms */
void *listenForFriends(void *);                    /* listen for friends */
void *messageFriend(void *);                       /* read friend messages */
void makeDiscoverable();                           /* listen for actions */
void destroyMutexes();                             /* destroy mutexes */
void displayFriends();
void handleInterrupt(int);                         /* SIGINT handler */

bool flag;                                         /* multi use flag */
bool loggedIn;                                     /* client's online status */                    
int rv;                                            /* return value */
int sockfd;                                        /* server socket */
int mysockfd;                                      /* local socket */
pthread_t serverThread;                            /* thread for server */
pthread_t inputThread;                             /* thread for user input */
pthread_t listenThread;                            /* thread for friend comms */
pthread_mutex_t friendMutex;                       /* online friends lock */
pthread_mutex_t inviteMutex;
pthread_mutex_t pendingMutex;
std::string myIp;                                  /* client ip/Hostname */
std::string myPort;                                /* client port */
std::string myHandle;                              /* client handle */
pthread_attr_t joinAttr;                           /* join thread attr */
pthread_attr_t detachAttr;                         /* detach thread attr */
struct addrinfo hints, *res, *ressave;             /* set up connections */
std::set<pthread_t> msgThreads;
Action action = INACTIVE;                           
std::unordered_map<int, User> friends;             /* friends' fd's and info */
std::set<std::string> sent_invites;                /* sent invites */
std::set<std::string> invites;                     /* pending invites */

/*
    argc: arg count
    argv: list of args
    return: when everything goes as planned
*/
int main(int argc, char *argv[])
{
    /* handle ctrl-c signal */
    signal(SIGINT, handleInterrupt);

    /* call to program must have form: messenger_client configuration_file */
    if(argc != 2)
    {
        printError(INVALID_CALL, "must have exactly 2 args", CLIENT);
        exit(EXIT_FAILURE);
    }

    /* ip/hostname = info[0] and port = info[1] */
    std::string *info = parseConfigFile(argv[1]);

    /* get addrinfo of server */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;
    if((rv = getaddrinfo(info[0].c_str(), info[1].c_str(), &hints, &res)) != 0)
    {
        printError(ADDRINFO_FAIL, info[0], CLIENT);
        exit(EXIT_FAILURE);
    }
    
    /* try to connect to server */
    flag = false;
    do {
        std::cout << "creating socket and attempting connect...\n";
        sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sockfd < 0)
            continue;
        if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0) 
        {
            flag = 1;
            break;
        }
        close(sockfd);
    } while ((res = res->ai_next) != NULL);
    
    /* were not able to connect */
    if (flag == false) 
    {
        printError(CONNECT_FAIL, "", CLIENT);
        exit(EXIT_FAILURE);
    }
    std::cout << "Connected to " << info[0] << " on port " << info[1] << "\n";
    loggedIn = false;

    /* setup thread so that we may wait on it and get its return value */
    pthread_attr_init(&joinAttr);
    pthread_attr_setdetachstate(&joinAttr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_init(&detachAttr);
    pthread_attr_setdetachstate(&detachAttr, PTHREAD_CREATE_DETACHED);

    /* initialize friend mutex */
    if (pthread_mutex_init(&friendMutex, nullptr) != 0) 
    {
        printError(MUTEX_CREATION_FAIL, "friendMutex", CLIENT);
        exit(EXIT_FAILURE);
    }

    /* initialize invites mutex */
    if (pthread_mutex_init(&inviteMutex, nullptr) != 0) 
    {
        printError(MUTEX_CREATION_FAIL, "inviteMutex", CLIENT);
        exit(EXIT_FAILURE);
    }

    /* initialize pending invites mutex */
    if (pthread_mutex_init(&pendingMutex, nullptr) != 0) 
    {
        printError(MUTEX_CREATION_FAIL, "pendingMutex", CLIENT);
        exit(EXIT_FAILURE);
    }

    /* create thread for user input which we will wait on with join */
    if(pthread_create(&inputThread, &joinAttr, handleInput, nullptr) != 0)
    {
        printError(CREATE_THREAD_FAIL, "inputThread", CLIENT);
        exit(EXIT_FAILURE);   
    }
    /* create thread read server responses which we will wait on with join */
    if(pthread_create(&serverThread, &joinAttr, handleServer, nullptr) != 0)
    {
        printError(CREATE_THREAD_FAIL, "serverThread", CLIENT);
        exit(EXIT_FAILURE);      
    }

    pthread_join(inputThread, NULL);
    pthread_join(serverThread, NULL);

    return EXIT_SUCCESS;
}

/*
   fileName: name of file storing server ip and port as "ip:port"
   return: two strings holding ip/hostname and port
*/
std::string *parseConfigFile(char *fileName)
{
    std::string *result = new std::string[2];

    /* file does not exist in current dir */
    std::ifstream file(fileName);
    if(file.is_open() == false)
    {
        printError(INVALID_FILE, fileName, CLIENT);
        exit(EXIT_FAILURE);
    }

    /* tokenize file of the form: ip/hostname:port */
    std::string line, token;
    getline(file, line);
    std::istringstream ss(line);
    /* get ip/hostname */
    getline(ss, token, ':');
    result[0] = token;
    /* eat leading whitespaes and get port */
    ss >> std::ws;
    getline(ss, token);
    result[1] = token;
    /* clean up trailing garbage */
    while(!isdigit(result[1].back()))
        result[1].erase(result[1].size() - 1);

    return result;
}

/*
    return: when fail to create socket to listen for incoming connections or 
    on success
*/
void makeDiscoverable()
{
    struct sockaddr_in localAddr;
    int len;
    char hostName[MAX_BUFLEN], buffer[MAX_BUFLEN];
    std::string data = "DISCOVER ";

    /* create listening socket */
    if((mysockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printError(SOCKET_FAIL, "", CLIENT);
        exit(EXIT_FAILURE);
    }

    /* allow the listening port to be reused if no longer active */
    int tmp = 1;
    if (setsockopt(mysockfd, SOL_SOCKET, SO_REUSEPORT, &tmp, sizeof(int)) < 0)
    {
        printError(REUSEPORT_FAIL, std::to_string(mysockfd), CLIENT);
    }

    /* build local address to bind to socekt */
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(CLIENT_PORT);
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(mysockfd, (struct sockaddr *)&localAddr, sizeof(localAddr)) < 0) 
    {
        printError(BIND_FAIL, std::to_string(mysockfd), CLIENT);
        exit(EXIT_FAILURE);
    }

    /* retrieve sockname of mysockfd */;
    len = sizeof(localAddr);
    if(getsockname(mysockfd, (struct sockaddr *)&localAddr,(socklen_t *)&len) < 0) 
    {
        printError(SOCKNAME_FAIL, "", CLIENT);
        exit(EXIT_FAILURE);
    }

    /* listen on mysockfd with max pending connections of MAX_BACKLOG */
    if(listen(mysockfd, MAX_BACKLOG) < 0) 
    {
        printError(LISTEN_FAIL, std::to_string(MAX_BACKLOG), CLIENT);
        exit(EXIT_FAILURE);
    }

    /*  get host name*/
    if(gethostname(hostName, HOSTNAME_MAX) < 0) 
    {
        printError(HOSTNAME_FAIL, "", CLIENT);
        exit(EXIT_FAILURE);
    }

    /* get our information */
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;
    if(getaddrinfo(hostName, nullptr, &hints, &res) != 0) 
    {
        if(getaddrinfo(hostName, "https", &hints, &res) != 0) 
        {    
            printError(ADDRINFO_FAIL, hostName, CLIENT);
            exit(EXIT_FAILURE);
        }
    }

    /* Display client location and tell server we are discoverable */
    myIp = res->ai_canonname;
    myPort = std::to_string(ntohs(localAddr.sin_port));
    std::cout << "Your Hostname: " << myIp << "\n";
    std::cout << "Your Port:     " << myPort << "\n";
    data = data + myHandle + " " + myIp + " " + myPort;
    std::copy(data.begin(), data.end(), buffer);
    write(sockfd, buffer, data.length());
    freeaddrinfo(res);

    loggedIn = true;

    /* create thread that listens for connections and accepts them */
    if (pthread_create(&listenThread, &detachAttr, listenForFriends, nullptr) != 0) 
    {
        printError(CREATE_THREAD_FAIL, "listenThread", CLIENT);
        exit(EXIT_FAILURE);
    }

    return;
}

/*
    arg: fd of friend we read from
    return: when client or friend logout or client or server terminate
*/
void *messageFriend(void *arg)
{
    int fd = *(int *)arg;
    char buffer[MAX_BUFLEN];

    /* keep reading from friend until they logout */
    while(true)
    {
        if(read(fd, buffer, sizeof(buffer)) > 0)
        {
            std::istringstream ss(buffer);
            std::string first, handle, msg;
            ss >> first;
            /* TCP connection was just established for this user */
            if(first.compare("CONNECTED") == 0)
            {
                ss >> handle;
                getline(ss, msg);
                /* update the fd for this user */
                pthread_mutex_lock(&friendMutex);
                std::unordered_map<int, User>::iterator itr;
                for(itr = friends.begin(); itr != friends.end(); ++itr)
                {
                    if(itr->second.getHandle().compare(handle) == 0)
                    {
                        friends.insert(std::pair<int, User>(fd, itr->second));
                        friends.erase(itr);
                        break;
                    }
                }
                pthread_mutex_unlock(&friendMutex);
                std::cout << handle << " >>" << msg << "\n";
            }
            /* friend logged off, close fd and kill of thread by returning */
            else if(first.compare("END_CONNECTION") == 0)
            {
                close(fd);
                return nullptr;
            }
            /* in a messaging instance with this user already */
            else
            {
                getline(ss, msg);
                std::cout << first << " >>" << msg << "\n";
            }
            memset(buffer, 0, sizeof(buffer));
        }
    }

    return nullptr;
}

/*
    arg: nothing userful, but I'm think we need this form for thread function
    return: client or server terminate
*/
void *handleInput(void *arg)
{
    std::string input, handle, key, data, amigo, msg, fip, fport;
    char buffer[MAX_BUFLEN];

    /* Give the user a prompt for commands */
    std::cout << "\n";
    std::cout << "Cmd   Info\n";
    std::cout << "----  ----\n";
    std::cout << "r     register\n";
    std::cout << "l     log in\n";
    std::cout << "exit  terminate connection\n";
    std::cout << "\n";

    /* keep getting user input from STDIN */
    while (true)
    {
        getline(std::cin, input);
        std::istringstream ss(input);
        std::string cmd;
        ss >> cmd; 
        data.clear();
        action = INACTIVE;
        memset(buffer, 0, sizeof(buffer));

        /* if client is not logged in, limit their options */
        if(!loggedIn)
        {
            /* register: r [username] [password] or or asks for handle and key */
            if(cmd.compare("r") == 0)
            {
                handle.clear();
                key.clear();
                ss >> handle >> key;
                action = REGISTER;
                data = "REGISTER ";
                while(handle.empty() && !EMPTY_HANDLE_ALLOWED)
                {
                    std::cout << "Username: ";
                    getline(std::cin, handle);
                }
                while(key.empty() && !EMPTY_KEY_ALLOWED)
                {
                    std::cout << "Password: ";
                    getline(std::cin, key);
                }
                data = data + handle + " " + key;
                std::copy(data.begin(), data.end(), buffer);
                write(sockfd, buffer, data.length());
            }
            /* log in: l [username] [password] or asks for handle and key */
            else if(cmd.compare("l") == 0)
            {
                myHandle.clear();
                key.clear();
                ss >> myHandle >> key;
                action = LOGIN;
                data = "LOGIN ";
                while(myHandle.empty() && !EMPTY_HANDLE_ALLOWED)
                {
                    std::cout << "Username: ";
                    getline(std::cin, myHandle);
                }
                while(key.empty() && !EMPTY_KEY_ALLOWED)
                {
                    std::cout << "Password: ";
                    getline(std::cin, key);
                }
                data = data + myHandle + " " + key;
                std::copy(data.begin(), data.end(), buffer);
                write(sockfd, buffer, data.length());
            }
            /* exit the program: exit */
            else if(cmd.compare("exit") == 0)
            {
                action = INACTIVE;
                data = "EXIT";
                std::copy(data.begin(), data.end(), buffer);
                write(sockfd, buffer, data.length());
                close(sockfd);
                exit(EXIT_SUCCESS);
            }
            /* display help menu: help */
            else if(cmd.compare("help") == 0)
            {
                action = INACTIVE;
                std::cout << "\n";
                std::cout << "Cmd   Info\n";
                std::cout << "----  ----\n";
                std::cout << "r     register\n";
                std::cout << "l     log in\n";
                std::cout << "exit  terminate connection\n";
                std::cout << "\n";
            }
            /* input is not recognized as valid */
            else
            {
                action = INACTIVE;
                std::cout << "Invalid command, type \"help\" for help\n";
            }
        }
        /* client is logged in so give them these options */
        else
        {
            bool loggedOff = false;
            amigo.clear();
            msg.clear();
            /* message friend: m [friendname] [message] */
            if(cmd.compare("m") == 0)
            {
                action = MESSAGE;
                ss >> amigo;
                if(amigo.empty())
                {
                    std::cout << "Please enter a nonempty username\n";
                    continue;
                }
                ss >> std::ws;
                getline(ss, msg);
                if(amigo.compare(myHandle) == 0)
                {
                    std::cout << "me >> " << msg << "\n";
                    continue;
                }
                /* dont waste resources sending empty messages */
                if(msg.empty())
                {
                    std::cout << "Please enter a nonempty message\n";
                    continue;
                }
                /* get friend location and fd */
                pthread_mutex_lock(&friendMutex);                
                int *friendfd = (int *)malloc(sizeof(int));
                std::unordered_map<int, User>::iterator fditr;
                bool cantFind = true;
                for(fditr = friends.begin(); fditr != friends.end(); ++fditr)
                {
                    if(fditr->second.getHandle().compare(amigo) == 0)
                    {
                        *friendfd = fditr->first;
                        fip = fditr->second.getIp();
                        fport = fditr->second.getPort();
                        if((fip.compare("-1") == 0) || (fport.compare("-1") == 0))
                        {
                            loggedOff = true;
                        }
                        cantFind = false;
                        break;
                    }
                }
                pthread_mutex_unlock(&friendMutex);

                /* user entered is not a friend */
                if((*friendfd == 0) || cantFind)
                {
                    std::cout << amigo << " is not your friend\n";
                    continue;
                }
                /* user entered has a TCP connection established */
                if(*friendfd > 0)
                {
                    msg = myHandle + " " + msg;
                    std::copy(msg.begin(), msg.end(), buffer);
                    write(*friendfd, buffer, msg.length());
                    continue;
                }
                if(loggedOff)
                {
                    std::cout << amigo << " is no longer online\n";
                    continue;
                }

                /* first time messaging, set up TCP connection */
                struct addrinfo fHints;
                struct addrinfo *fInfo;
                int fsocket;

                /* get friend addrinfo (these are settings for client) */
                memset(&hints, 0, sizeof(hints));
                fHints.ai_family = AF_INET;
                fHints.ai_socktype = SOCK_STREAM;
                fHints.ai_flags = AI_CANONNAME;
                if(getaddrinfo(fip.c_str(), fport.c_str(), &fHints, &fInfo) != 0) 
                {
                    std::cout << "Could not retrieve friend's addr info (probably offline)\n";
                    continue;
                }

                /* create socket for friend */
                if ((fsocket = socket(fInfo->ai_family, fInfo->ai_socktype, fInfo->ai_protocol)) < 0) 
                {
                    std::cout << "Failed to create socket for " << amigo
                              << "at fd: " << fsocket << "\n";
                    continue;
                }

                /* connect to friend */
                if(connect(fsocket, fInfo->ai_addr, fInfo->ai_addrlen) < 0)
                {
                    std::cout << "Could not connect to " << amigo << "\n";
                    continue;
                }

                /* update fd to which we read friend's messages */
                pthread_mutex_lock(&friendMutex);
                for(fditr = friends.begin(); fditr != friends.end(); ++fditr)
                {
                    if(fditr->second.getHandle().compare(amigo) == 0)
                    {
                        friends.insert(std::pair<int, User>(fsocket, fditr->second));
                        friends.erase(fditr);
                        fip = friends.at(fsocket).getIp();
                        fport = friends.at(fsocket).getPort();
                        break;
                    }
                }
                pthread_mutex_unlock(&friendMutex);

                /* tell user its our first time connecting */
                data = "CONNECTED " + myHandle + " " + msg;
                std::copy(data.begin(), data.end(), buffer);
                write(fsocket, buffer, data.length());
                int *fdptr = &fsocket;

                /* create thread that will read from friend's fd */
                pthread_t firstContactThread;
                if (pthread_create(&firstContactThread, &detachAttr, messageFriend, (void *)fdptr) != 0) 
                {
                    printError(CREATE_THREAD_FAIL, "firstContactThread", CLIENT);
                    exit(EXIT_FAILURE);
                }
                msgThreads.insert(firstContactThread);
            }
            /* invite user to be friend: i [username] [optional message] */
            else if(cmd.compare("i") == 0)
            {
                action = INVITE;
                bool repeat = false;
                ss >> amigo;
                if(amigo.empty())
                {
                    std::cout << "Please enter a nonempty username\n";
                    continue;
                }
                /* can not invite yourself */
                if(amigo.compare(myHandle) == 0)
                {
                    std::cout << "You can not invite yourself\n";
                    continue;
                }
                /* check if user is already our friend */
                pthread_mutex_lock(&friendMutex);
                std::unordered_map<int, User>::iterator uitr;
                for(uitr = friends.begin(); uitr != friends.end(); ++uitr)
                {
                    if(uitr->second.getHandle().compare(amigo) == 0)
                    {
                        std::cout << amigo << " is already your friend\n";
                        repeat = true;
                        break;
                    }
                }
                pthread_mutex_unlock(&friendMutex);

                pthread_mutex_lock(&pendingMutex);
                /* check if an invite to this user has already been sent */
                std::set<std::string>::iterator sitr;
                for(sitr = sent_invites.begin(); sitr != sent_invites.end(); ++sitr)
                {
                    if(sitr->compare(amigo) == 0)
                    {
                        std::cout << "You have already sent an invite to this user\n";
                        repeat = true;
                        break;
                    }
                }
                /* check if user has already invited us */
                if(!repeat)
                {
                    for(sitr = invites.begin(); sitr != invites.end(); ++sitr)
                    {
                        if(sitr->compare(amigo) == 0)
                        {
                            std::cout << "You already have a pending invite from this user\n";
                            repeat = true;
                            break;
                        }
                    }
                }
                pthread_mutex_unlock(&pendingMutex);
                if(repeat)
                    continue;

                /* inform server to send an invite to user */
                ss >> std::ws;
                getline(ss, msg);
                data = "INVITE " + amigo + " " + msg;
                std::copy(data.begin(), data.end(), buffer);
                write(sockfd, buffer, data.length());
                pthread_mutex_lock(&pendingMutex);
                sent_invites.insert(amigo);
                pthread_mutex_unlock(&pendingMutex);
            }
            /* accept invite: ia [username] [optional message] */
            else if(cmd.compare("ia") == 0)
            {
                action = ACCEPT_INVITE;
                ss >> amigo;
                if(amigo.empty())
                {
                    std::cout << "Please enter a nonempty username\n";
                    continue;
                }
                pthread_mutex_lock(&inviteMutex);
                /* check if we actually have an invite from this user */
                bool pending = false;
                std::set<std::string>::iterator sitr;
                for(sitr = invites.begin(); sitr != invites.end(); ++sitr)
                {
                    if(sitr->compare(amigo) == 0)
                    {
                        pending = true;
                        break;
                    }
                }
                pthread_mutex_unlock(&inviteMutex);
                if(!pending)
                {
                    std::cout << "You do not have an invite from " << amigo << "\n";
                    continue;
                }

                /* inform server we have accepted the invite from user */
                ss >> std::ws;
                getline(ss, msg);
                data = "ACCEPT " + amigo + " " + msg;
                std::copy(data.begin(), data.end(), buffer);
                write(sockfd, buffer, data.length());
            }
            /* show friends: f */
            else if(cmd.compare("f") == 0)
            {
                displayFriends();
            }
            /* logout: logout */
            else if(cmd.compare("logout") == 0)
            {
                action = LOGOUT;
                loggedIn = false;
                data = "END_CONNECTION";
                std::copy(data.begin(), data.end(), buffer);
                pthread_cancel(listenThread);
                pthread_mutex_lock(&friendMutex);
                std::unordered_map<int, User>::iterator itr;
                for(itr = friends.begin(); itr != friends.end(); ++itr)
                {
                    write(itr->first, buffer, data.length());
                    close(itr->first);
                }
                friends.clear();
                std::set<pthread_t>::iterator titr;
                for(titr = msgThreads.begin(); titr != msgThreads.end(); ++titr)
                {
                    pthread_cancel(*titr);
                }
                msgThreads.clear();
                pthread_mutex_unlock(&friendMutex);
                data.clear();
                data = "LOGOUT " + myHandle;
                std::copy(data.begin(), data.end(), buffer);
                write(sockfd, buffer, data.length());
                close(mysockfd);
                myHandle.clear();
            }
            /* display help menu: help */
            else if(cmd.compare("help") == 0)
            {
                action = INACTIVE;
                std::cout << "\n";
                std::cout << "Cmd                       Info\n";
                std::cout << "---------------------     --------------------\n";
                std::cout << "m  [handle] [message]     message friend\n";
                std::cout << "i  [handle] [message]     invite\n";
                std::cout << "ia [handle] [message]     accept invite\n";
                std::cout << "f                         show friends\n";
                std::cout << "logout                    log out\n";
                std::cout << "\n";
            }
            /* unrecognized command */
            else
            {
                action = INACTIVE;
                std::cout << "Invalid command, type \"help\" for help\n";
            }
        }
    }

    return nullptr;
}
    
/*
    arg: nothing userful, but I'm think we need this form for thread function
    return: client or server terminates
*/
void *handleServer(void *arg)
{
    char response[MAX_BUFLEN] = "";
    std::string cmd, amigo, msg, ip, port, handle;

    /* read responses from the server */
    while (true)
    {
        memset(response, 0, sizeof(response));
        if(read(sockfd, response, sizeof(response)) <= 0)
        {
            std::cout << "Connection to server lost\n";
            if(loggedIn)
            {
                std::unordered_map<int, User>::iterator itr;
                for(itr = friends.begin(); itr != friends.end(); ++itr)
                {
                    close(itr->first);
                }
            }
            close(mysockfd);
            close(sockfd);
            pthread_cancel(listenThread);
            destroyMutexes();
            exit(EXIT_SUCCESS);
        }
        else
        {
            /* std::cout << "RESPONSE: " << response << "\n"; */
            std::istringstream ss(response);
            ss >> cmd;
            /* server successfully completed one of our requests */
            if(cmd.compare("200") == 0)
            {
                ss >> cmd;
                /* server successfully logged user in */
                if(cmd.compare("LOGIN") == 0)
                {
                    /* read friends list */
                    pthread_mutex_lock(&friendMutex);
                    int i = -1;
                    while(ss >> amigo)
                    {
                        User onlineFriend(amigo, "");
                        friends.insert(std::pair<int, User>(i, onlineFriend));
                        i--;
                    }
                    pthread_mutex_unlock(&friendMutex);
                    makeDiscoverable();
                    /* present the user with prompt */
                    std::cout << "Login successful\n";
                    std::cout << "\n";
                    std::cout << "Cmd                       Info\n";
                    std::cout << "---------------------     --------------------\n";
                    std::cout << "m  [handle] [message]     message friend\n";
                    std::cout << "i  [handle] [message]     invite\n";
                    std::cout << "ia [handle] [message]     accept invite\n";
                    std::cout << "f                         show friends\n";
                    std::cout << "logout                    log out\n";
                    std::cout << "\n";
                }
                /* server successfully registered new user */
                else if(cmd.compare("REGISTER") == 0)
                {
                    std::cout << "Registration successful, please login\n";
                }
                /* server successfully logged user out */
                else
                {
                    std::cout << "Logout successful\n";
                }
            }
            /* server did not complete one of our requests */
            else if(cmd.compare("500") == 0)
            {
                ss >> cmd;
                /* server could not log user in */
                if(cmd.compare("LOGIN") == 0)
                {
                    std::cout << "Login failed\n";
                }
                /* server could not register new user */
                else
                {
                    std::cout << "Registration failed\n";
                }
            }
            /* server informs client of a friends location */
            else if(cmd.compare("LOCATE") == 0)
            {
                pthread_mutex_lock(&friendMutex);
                while(ss >> amigo >> ip >> port)
                {
                    /* lock for user and add location information */
                    std::unordered_map<int, User>::iterator fitr;
                    for(fitr = friends.begin(); fitr != friends.end(); fitr++)
                    {
                        if(fitr->second.getHandle().compare(amigo) == 0)
                        {
                            fitr->second.setLocation(ip, port);
                            break;
                        }
                    }
                }
                pthread_mutex_unlock(&friendMutex);
                displayFriends(); 
            }
            else if(cmd.compare("OFFLINE") == 0)
            {
                ss >> amigo;
                pthread_mutex_lock(&friendMutex);
                std::unordered_map<int, User>::iterator fitr;
                for(fitr = friends.begin(); fitr != friends.end(); fitr++)
                {
                    if(fitr->second.getHandle().compare(amigo) == 0)
                    {
                        int k = -1;
                        while(friends.find(k) != friends.end()){k--;}
                        User newFriend(amigo, "");
                        friends.insert(std::pair<int, User>(k, newFriend));
                        fitr->second.setLocation("-1", "-1");
                        close(fitr->first);
                        friends.erase(fitr);
                        std::cout << amigo << " logged out\n"; 
                        break;
                    }
                }
                pthread_mutex_unlock(&friendMutex);
            }
            /* server informs client of new friend */
            else if(cmd.compare("ACCEPT_FRIEND") == 0)
            {
                ss >> amigo >> ip >> port;
                /* server informs client new friend is not currently online */
                if(ip.compare("OFFLINE") == 0)
                {
                    std::cout << amigo << " is now your friend, but is offline\n";
                    /* create a new entry for new friend */
                    pthread_mutex_lock(&friendMutex); 
                    int k = -1;
                    while(friends.find(k) != friends.end()){k--;}
                    User newFriend(amigo, "");
                    friends.insert(std::pair<int, User>(k, newFriend));
                    pthread_mutex_unlock(&friendMutex);
                }
                /* new friend is online */
                else
                {
                    ss >> std::ws;
                    msg.clear();
                    getline(ss, msg);
                    if(!msg.empty())
                    {
                        std::cout << amigo << " is now your friend: " << msg << "\n";    
                    }
                    else
                    {
                        std::cout << amigo << " is now your friend, and is online\n";
                    }
                    /* create a new entry for new friend, including location */
                    pthread_mutex_lock(&friendMutex); 
                    int k = -1;
                    while(friends.find(k) != friends.end()){k--;}
                    User newFriend(amigo, "");
                    newFriend.setLocation(ip, port);
                    friends.insert(std::pair<int, User>(k, newFriend));
                    pthread_mutex_unlock(&friendMutex); 
                }
                displayFriends();
            }
            /* server informs client of an invite from a user */
            else if(cmd.compare("INVITE") == 0)
            {
                ss >> amigo;
                ss >> std::ws;
                getline(ss, msg);
                std::cout << "You got an invite from " << amigo << ": " << msg << "\n";
                pthread_mutex_lock(&inviteMutex);
                invites.insert(amigo);
                pthread_mutex_unlock(&inviteMutex);
            }
            /* server informs client the user's invite was not successful */
            else if(cmd.compare("INVITE_FAIL") == 0)
            {
                ss >> amigo;
                std::cout << amigo << " is not online, invite not sent\n";
                pthread_mutex_lock(&pendingMutex);
                sent_invites.erase(amigo);
                pthread_mutex_unlock(&pendingMutex);
            }
            /* server is trying to say something that is not a command */
            else
            {
                std::cout << response << "\n";
            }
        }
    }

    return nullptr;
}

/*
    arg: nothing userful, but I'm think we need this form for thread function
    return: client logout, termination, or server termination
*/
void *listenForFriends(void *arg)
{
    int *friendfd = (int *)malloc(sizeof(int));
    struct sockaddr_in friendAddr;
    socklen_t len = sizeof(friendAddr);
    pthread_t msgThread;

    /* keep accepting new connections until logout or program termination */
    while(true)
    {
        *friendfd = accept(mysockfd, (struct sockaddr *)&friendAddr, &len);
        if(*friendfd == -1)
        {
            printError(ACCEPT_FAIL, std::to_string(*friendfd), CLIENT);
            continue;
        }
        /* create thread to read from conected user's fd */
        if(pthread_create(&msgThread, &detachAttr, messageFriend, (void *)friendfd) != 0)
        {
            printError(CREATE_THREAD_FAIL, "msgThread", CLIENT);
            msgThreads.erase(pthread_self());
            continue;
        }
        msgThreads.insert(msgThread);
    }

    return nullptr;
}

void displayFriends()
{
    pthread_mutex_lock(&friendMutex);
    std::unordered_map<int, User>::iterator itr;
    std::cout << "\n";
    for(itr = friends.begin(); itr != friends.end(); ++itr)
    {
        if((itr->second.getIp().empty()) || 
           (itr->second.getIp().compare("-1") == 0))
        {
            std::cout << "[offline] ";
        }
        else
        {
            std::cout << "[online]  ";
        }
        std::cout << itr->second.getHandle() << "\n";
    }
    std::cout << "\n";
    pthread_mutex_unlock(&friendMutex);

    return;
}

void destroyMutexes()
{
    pthread_mutex_destroy(&friendMutex);
    pthread_mutex_destroy(&inviteMutex);
    pthread_mutex_destroy(&pendingMutex);        

    return;
}

/*
    sig: signal 
    return:  finished "gentle" clean up
*/
void handleInterrupt(int sig)
{
    std::string data;
    char buffer[MAX_BUFLEN] = "";
    /* inform server we are termination (CHANGE THIS TO DIF CMD: TERMINATE) */
    data.clear();
    if(loggedIn)
    {
        data = "KILL " + myHandle;
        std::copy(data.begin(), data.end(), buffer);
        write(sockfd, buffer, data.length());
        loggedIn = false;
        pthread_mutex_lock(&friendMutex);
        std::unordered_map<int, User>::iterator itr;
        for(itr = friends.begin(); itr != friends.end(); ++itr)
        {
            close(itr->first);
        }
        pthread_mutex_unlock(&friendMutex);

        /* kill off all detached threads that are still alive */
        std::set<pthread_t>::iterator mitr;
        for(mitr = msgThreads.begin(); mitr != msgThreads.end(); ++mitr)
        {
            pthread_cancel(*mitr);
        }
        pthread_cancel(listenThread);
        close(mysockfd);
    }
    else
    {
        data = "EXIT";
        std::copy(data.begin(), data.end(), buffer);
        write(sockfd, buffer, data.length());
    }

    close(sockfd);
    destroyMutexes();
    std::cout << " Terminated all connections\n";

    exit(EXIT_SUCCESS);
}
