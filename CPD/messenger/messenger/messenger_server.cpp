/*###############################
  #        David Miller         #
  #       COP 5570 - CPD        #
  #         Project 2           #
  #        Nov 29 2018          #
  ###############################*/

#include <arpa/inet.h>                             /* server IP address */
#include "common.h"                                /* userful stuff */
#include <errno.h>                                 /* errno */
#include <fstream>                                 /* is_open(), ifstream */
#include <iostream>                                /* cerr */
#include <list>                                    /* list container */
#include <map>                                     /* mao container */
#include <netdb.h>                                 /* addrinfo */
#include <netinet/in.h>                            /* sockaddr_in */
#include <pthread.h>                               /* mutex */
#include <sys/select.h>                            /* select() multiplexor */
#include <set>                                     /* set */
#include <signal.h>                                /* signals and handlers */
#include <sys/socket.h>                            /* sockets */
#include <sstream>                                 /* istringstream */
#include <stdlib.h>                                /* exit() */
#include <string>                                  /* strings */
#include <string.h>                                /* strerror */
#include <unistd.h>                                /* get hostname */
#include "user.h"                                  /* User class */
#include <vector>                                  /* vectors */  

void parseUserFile(char *);                        /* parse user file */
void parseConfigFile(char  *);                     /* parse config file */
char *registerUser(std::string, std::string);      /* register new user */
char *loginUser(std::string, std::string, int);    /* log user in */
bool handleClient(char *, int);                    /* handle client input */
void handleInterrupt(int);                         /* SIGINT handler */
void sendFriends(std::string, int);                /* send friends on login */
void addFriends(std::string, std::string);         /* add each as friends */

std::vector<User> userBase;                        /* stores server users */
std::map<int, User> onlineUsers;                   /* stores online users */
struct sockaddr_in localAddr;                      /* store local addr */
struct addrinfo hints, *res;                       /* used to get addrinfo */
int port;                                          /* port number */
int sockfd;                                        /* socket to listen on */
char hostName[HOSTNAME_MAX];                       /* hold hostname */
char userFilename[MAX_BUFLEN];                     /* name of user file */
int len;                                           /* hold length of items */
pthread_mutex_t onlineMutex;                       /* online users lock */
pthread_mutex_t baseMutex;                         /* all users lock */
pthread_mutex_t fileMutex;                         /* mutex for "database" */
fd_set readfds;                                    /* fds to read from */
fd_set masterfds;                                  /* master fds */
fd_set writefds;                                   /* fds to write to */
fd_set exceptfds;                                  /* exception fds */

/*
   argc: number of args
   argv: list of args
*/
int main(int argc, char *argv[])
{
    /* catch ctrl-C and handle it */
    signal(SIGINT, handleInterrupt);

    /* call to program must have form: messenger_server user_file config_file */
    if(argc != 3)
    {
        printError(INVALID_CALL, "must have exactly 3 args", SERVER);
        exit(EXIT_FAILURE);
    }

    /* parse and store user info and server configs (exit if port is bad) */
    parseUserFile(argv[1]);
    strcpy(userFilename, argv[1]);
    parseConfigFile(argv[2]);

    /* create server socket */
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printError(SOCKET_FAIL, "", SERVER);
        exit(EXIT_FAILURE);
    }

    /* allow the server port to be reused if no longer active */
    int tmp = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &tmp, sizeof(int)) < 0)
    {
        printError(REUSEPORT_FAIL, std::to_string(sockfd), SERVER);
    }

    /* build local address to bind to socekt */
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(port);
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(sockfd, (struct sockaddr *)&localAddr, sizeof(localAddr)) < 0) 
    {
        printError(BIND_FAIL, std::to_string(sockfd), SERVER);
        exit(EXIT_FAILURE);
    }

    /* retrieve local address of sockfd */;
    len = sizeof(localAddr);
    if(getsockname(sockfd, (struct sockaddr *)&localAddr,(socklen_t *)&len) < 0) 
    {
        printError(SOCKNAME_FAIL, "", SERVER);
        exit(EXIT_FAILURE);
    }

    /* listen on socket with max pending connections of MAX_BACKLOG */
    if(listen(sockfd, MAX_BACKLOG) < 0) 
    {
        printError(LISTEN_FAIL, std::to_string(MAX_BACKLOG), SERVER);
        exit(EXIT_FAILURE);
    }

    /*  get host name*/
    if(gethostname(hostName, HOSTNAME_MAX) < 0) 
    {
        printError(HOSTNAME_FAIL, "", SERVER);
        exit(EXIT_FAILURE);
    }

    /* get server information */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;
    if(getaddrinfo(hostName, nullptr, &hints, &res) != 0) 
    {
        printError(ADDRINFO_FAIL, hostName, SERVER);
        exit(EXIT_FAILURE);
    }
    std::cout << "Hostname: " << hostName << "\n";
    std::cout << "Port:     " << ntohs(localAddr.sin_port) << "\n";
    freeaddrinfo(res);

    /* intiialize mutexes for userBase, onlineUsers, and database file */
    if (pthread_mutex_init(&baseMutex, nullptr) != 0) 
    {
        printError(MUTEX_CREATION_FAIL, "baseMutex", CLIENT);
        exit(EXIT_FAILURE);
    }
    if (pthread_mutex_init(&onlineMutex, nullptr) != 0) 
    {
        printError(MUTEX_CREATION_FAIL, "onlineMutex", CLIENT);
        exit(EXIT_FAILURE);
    }
    if (pthread_mutex_init(&fileMutex, nullptr) != 0) 
    {
        printError(MUTEX_CREATION_FAIL, "fileMutex", CLIENT);
        exit(EXIT_FAILURE);
    }

    /* setup infrastructure to accept new connections */
    struct sockaddr_in client_addr;
    socklen_t sock_len = sizeof(client_addr);
    int clientfd, maxfd;
    ssize_t n;
    std::list<int> sockfds;
    std::cout << "Server now listening...\n";
    
    FD_ZERO(&masterfds);
    FD_SET(sockfd, &masterfds);
    maxfd = sockfd;
    sockfds.push_back(sockfd);
    char buffer[MAX_BUFLEN];

    /* keep trying to read from fds */
    while(true)
    {
        readfds = masterfds;
        select(maxfd+1, &readfds, &writefds, &exceptfds, NULL);
        std::list<int> tempfds(sockfds);
        std::list<int>::iterator itr;
        for(itr = tempfds.begin(); itr != tempfds.end(); ++itr)
        {
            /* check if we can perform a read */
            int temp = *itr;
            if(FD_ISSET(temp, &readfds))
            {
                /* new connection */
                if(temp == sockfd)
                {
                    clientfd = accept(sockfd, (struct sockaddr *)&client_addr, 
                                      &sock_len);
                    FD_SET(clientfd, &masterfds);
                    if(clientfd > maxfd)
                        maxfd = clientfd;
                    sockfds.push_back(clientfd);
                }
                /* recognized client */
                else
                {
                    memset(buffer, 0, sizeof(buffer));
                    n = read(temp, buffer, sizeof(buffer));
                    if(n <= 0)
                    {
                        if(n == 0)
                            std::cout << "Connection closed\n";
                        else
                            std::cout << "PROBLEM!\n";
                        close(temp);
                        FD_CLR(temp, &masterfds);
                        sockfds.remove(temp);
                    }
                    else
                    {
                        buffer[n] = '\0';
                        if(handleClient(buffer, temp))
                        {
                            close(temp);
                            FD_CLR(temp, &masterfds);
                            sockfds.remove(temp);
                        }
                    }
                }
            }
        }
    }

    return EXIT_SUCCESS;
}

/*  
   fileName: name of file containing user info
   exit: when filename with name "fileName" is not in current dir
*/
void parseUserFile(char *fileName)
{
    /* file does not exist in current dir */
    std::ifstream file(fileName);
    if(file.is_open() == false)
    {
        printError(INVALID_FILE, fileName, SERVER);
        exit(EXIT_FAILURE);
    }

    /* parse and store user, password, and friends */
    std::string line;
    int lineNum = 1;
    while(getline(file, line))
    {
        std::istringstream ss(line);
        std::string handle, key, amigo;
        /* get user handle */
        getline(ss, handle, '|');
        if((handle.length() == 0) && !EMPTY_HANDLE_ALLOWED)
        {
            printError(EMPTY_HANDLE, std::to_string(lineNum), SERVER);
            continue;
        }
        /* get user key */
        getline(ss, key, '|');
        if((key.length() == 0) && !EMPTY_KEY_ALLOWED)
        {
            printError(EMPTY_KEY, std::to_string(lineNum), SERVER);
            continue;
        }
        /* build new user and add to server's user base */
        User newUser(handle, key);
        while(getline(ss, amigo, ';'))
            newUser.addFriend(amigo);
        userBase.push_back(newUser);
        lineNum++;
    }

    file.close();
}

/*
    fileName: name of file to parse for users and friends
*/
void parseConfigFile(char *fileName)
{
    /* file does not exist in current dir */
    std::ifstream file(fileName);
    if(file.is_open() == false)
    {
        printError(INVALID_FILE, fileName, SERVER);
        exit(EXIT_FAILURE);
    }

    std::string line;
    while(getline(file, line))
    {
        std::istringstream ss(line);
        std::string temp;
        /* get port value */
        getline(ss, temp, ':');
        if(temp.compare("port") != 0)
        {
            printError(PORT_SYNTAX, temp, SERVER);
            continue;
        }
        /* eat any whitespaces */
        ss >> std::ws;
        ss >> port;
        if(port < 0)
        {
            printError(NEGATIVE_PORT, std::to_string(port), SERVER);
            exit(EXIT_FAILURE);
        }
    }

    file.close();
}

/*
    input: what select() read from fd
    respondfd: the fd to write back to
*/
bool handleClient(char *input, int respondfd)
{
    /* std::cout << "INPUT: " << input << "\n"; */
    std::istringstream ss(input);
    std::string cmd, handle, key, ip, port, amigo, msg, data;
    char *result = (char *)malloc(MAX_BUFLEN * sizeof(char));
    char buffer[MAX_BUFLEN];
    bool flag;
    
    /* Every write to server is of form: CMD [arg1] ... [argn] */
    ss >> cmd;
    /* Client is trying to register */
    if(cmd.compare("REGISTER") == 0)
    {
        pthread_mutex_lock(&fileMutex);
        ss >> handle >> key;
        result = registerUser(handle, key);
        /* successful registration */
        if(strcmp(result, "200 REGISTER") == 0)
        {
            std::ofstream file(userFilename, std::ofstream::app);
            if(file.is_open() == false)
            {
                printError(INVALID_FILE, userFilename, SERVER);
                exit(EXIT_FAILURE);
            }
            else
            {
                file << handle << "|" << key << "|\n";
                file.close();
            }
        }
        pthread_mutex_unlock(&fileMutex);
        write(respondfd, result, strlen(result));
    }
    /* Client is trying to login */
    else if(cmd.compare("LOGIN") == 0)
    {
        ss >> handle >> key;
        result = loginUser(handle, key, respondfd);
        /* successful login */
        if(strcmp(result, "200 LOGIN") == 0)
        {
            pthread_mutex_lock(&onlineMutex);
            User loggedUser(handle, key);
            onlineUsers.insert(std::pair<int, User>(respondfd, loggedUser));
            pthread_mutex_unlock(&onlineMutex);
            sendFriends(handle, respondfd);
        }
        else
        {
            write(respondfd, result, strlen(result));            
        }
    }
    /* client logged in and now made themselves discoverable by listen() */
    else if(cmd.compare("DISCOVER") == 0)
    {
        ss >> handle >> ip >> port;
        pthread_mutex_lock(&onlineMutex);
        onlineUsers.at(respondfd).setLocation(ip, port);
        std::vector<User>::iterator uitr, userFound;
        std::vector<std::string>::iterator fitr;
        std::map<int, User>::iterator oitr;
        std::string data2 = "LOCATE";
        flag = false;
        /* tell client's friends they are online by sending location info */
        for(uitr = userBase.begin(); uitr != userBase.end(); ++uitr)
        {
            userFound = uitr;
            if(uitr->getHandle().compare(handle) == 0)
            {
                data.clear();
                for(fitr = uitr->friends.begin(); fitr != uitr->friends.end(); ++fitr)
                {
                    for(oitr = onlineUsers.begin(); oitr != onlineUsers.end(); ++oitr)
                    {
                        if(oitr->second.getHandle().compare(*fitr) == 0)
                        {
                            /* send friend location info */
                            flag = true;
                            data = "LOCATE " + handle + " " + ip + " " + port;
                            std::copy(data.begin(), data.end(), buffer);
                            write(oitr->first, buffer, data.length());
                            data.clear();
                            /* get friend location info */
                            if((data2.length() + oitr->second.getHandle().length()
                                + oitr->second.getIp().length() 
                                + oitr->second.getPort().length() + 2) > MAX_BUFLEN)
                            {
                                std::copy(data2.begin(), data2.end(), buffer);
                                write(respondfd, buffer, data2.length());
                                data2.clear();
                                data2 = "LOCATE";
                            }
                            data2 = data2 + " " + oitr->second.getHandle() + " "
                                    + oitr->second.getIp() + " " + 
                                    oitr->second.getPort();   
                        }
                    }
                }
            }
            if(flag)
            {
                std::copy(data2.begin(), data2.end(), buffer);
                write(respondfd, buffer, data2.length());
                break;
            }
        }
        pthread_mutex_unlock(&onlineMutex);
    }
    /* user asks server to send a friend request */    
    else if(cmd.compare("INVITE") == 0)
    {
        ss >> amigo;
        ss >> std::ws;
        getline(ss, msg);
        pthread_mutex_lock(&onlineMutex);
        std::map<int, User>::iterator mitr;
        flag = false;
        /* check if user getting invite is online */
        for(mitr = onlineUsers.begin(); mitr != onlineUsers.end(); mitr++)
        {
            if(mitr->second.getHandle().compare(amigo) == 0)
            {
                data = "Invite to " + amigo + " sent";
                std::copy(data.begin(), data.end(), result);
                data.clear();
                write(respondfd, result, data.length());
                memset(result, 0, MAX_BUFLEN);
                data = "INVITE " + onlineUsers.at(respondfd).getHandle() + " " + msg;
                respondfd = mitr->first;
                std::copy(data.begin(), data.end(), result);
                write(respondfd, result, data.length());
                flag = true;
                break;
            }
        }
        pthread_mutex_unlock(&onlineMutex);
        /* user was not online to receive invite */
        if(!flag)
        {
            data = "INVITE_FAIL " + amigo;
            std::copy(data.begin(), data.end(), result);
            write(respondfd, result, data.length());
        }
    }
    /* friend request was accepted */
    else if(cmd.compare("ACCEPT") == 0)
    {
        bool online = false;
        int fidx;
        std::string accepter;
        ss >> amigo;
        ss >> std::ws;
        getline(ss, msg);
        addFriends(amigo, onlineUsers.at(respondfd).getHandle());
        pthread_mutex_lock(&onlineMutex);
        std::map<int, User>::iterator mitr;
        /* notify inviter of successful accept */
        for(mitr = onlineUsers.begin(); mitr != onlineUsers.end(); mitr++)
        {
            if(mitr->second.getHandle().compare(amigo) == 0)
            {
                data = "ACCEPT_FRIEND " + onlineUsers.at(respondfd).getHandle()
                       + " " + onlineUsers.at(respondfd).getIp() + " " +
                       onlineUsers.at(respondfd).getPort() + " " + msg;
                std::copy(data.begin(), data.end(), result);
                write(mitr->first, result, data.length());
                online = true;
                fidx = mitr->first;
                break;
            }
        }
        /* notify invitee of successful accept and that inviter is online */
        if(online)
        {
            memset(result, 0, MAX_BUFLEN);
            data.clear();
            data = "ACCEPT_FRIEND " + amigo + " " + onlineUsers.at(fidx).getIp()
                   + " " + onlineUsers.at(fidx).getPort();
            pthread_mutex_unlock(&onlineMutex);
            std::copy(data.begin(), data.end(), result);
            write(respondfd, result, data.length());
        }
        /* notify invitee of successful accept and that inviter is offline */
        else
        {
            pthread_mutex_unlock(&onlineMutex);
            memset(result, 0, MAX_BUFLEN);
            data.clear();
            data = "ACCEPT_FRIEND " + amigo + " OFFLINE";
            std::copy(data.begin(), data.end(), result);
            write(respondfd, result, data.length());
        }
    }
    /* client was terminated */
    else if(cmd.compare("EXIT") == 0)
    {
        pthread_mutex_lock(&onlineMutex);
        onlineUsers.erase(respondfd);
        pthread_mutex_unlock(&onlineMutex);
        std::cout << "User at fd " << respondfd << " terminated connection\n";
        return true;
    }
    /* client logged out */
    else if(cmd.compare("LOGOUT") == 0)
    {
        ss >> handle;
        data = "OFFLINE " + handle;
        std::copy(data.begin(), data.end(), buffer);
        pthread_mutex_lock(&onlineMutex);
        std::vector<User>::iterator itr;
        std::map<int, User>::iterator oitr;
        /* notify friends that user logged out and is no longer online */
        for(oitr = onlineUsers.begin(); oitr != onlineUsers.end(); ++oitr)
        {
            for(itr = userBase.begin(); itr != userBase.end(); ++itr)
            {
                if(oitr->second.getHandle().compare(itr->getHandle()) == 0)
                {
                    write(oitr->first, buffer, data.length());
                    break;
                }
            }
        }
        onlineUsers.erase(respondfd);
        pthread_mutex_unlock(&onlineMutex);
        std::cout << "User " << handle << " logged out\n";
        data = "200 LOGOUT";
        std::copy(data.begin(), data.end(), buffer);
        write(respondfd, buffer, data.length());
    }
    /* client was terminated with SIGINT */
    else if(cmd.compare("KILL") == 0)
    {
        ss >> handle;
        data = "OFFLINE " + handle;
        std::copy(data.begin(), data.end(), buffer);
        pthread_mutex_lock(&onlineMutex);
        std::vector<User>::iterator itr;
        std::vector<std::string>::iterator fitr;
        std::map<int, User>::iterator oitr;
        /* notify friends that use is no longer online */
        for(itr = userBase.begin(); itr != userBase.end(); ++itr)
        {
            if(itr->getHandle().compare(handle) == 0)
            {
                for(fitr = itr->friends.begin(); 
                    fitr != itr->friends.end(); ++fitr)
                {
                    for(oitr = onlineUsers.begin(); 
                        oitr != onlineUsers.end(); ++oitr)
                    {
                        if(oitr->second.getHandle().compare(*fitr) == 0)
                        {
                            write(oitr->first, buffer, data.length());
                            break;
                        }
                    }
                }
                break;
            }
        }
        onlineUsers.erase(respondfd);
        pthread_mutex_unlock(&onlineMutex);
        std::cout << "User " << handle << "'s connection was killed\n";
        return true;
    }
    /* server received an unsupported command, just display it */
    else
    {
        std::cout << "Unrecognized command: " << input << "\n";         
    }

    return false;
}

/*
    h: username
    k: password
*/
char *registerUser(std::string h, std::string k)
{
    char *result = (char *)malloc(MAX_BUFLEN * sizeof(char));
    std::vector<User>::iterator itr;

    /* check if username already exists */
    pthread_mutex_lock(&baseMutex);
    for(itr = userBase.begin(); itr != userBase.end(); itr++)
    {
        if(h.compare(itr->getHandle()) == 0)
        {
            pthread_mutex_unlock(&baseMutex);
            std::cout << "Failed to register " << h 
                      << " with password *****\n";
            strcpy(result, "500 REGISTER");
            return result;
        }
    }

    /* build new user and insert into userBase */
    User newUser(h, k);
    userBase.push_back(newUser);
    pthread_mutex_unlock(&baseMutex);
    std::cout << "Succesfully registered " << h 
              << " with password *****\n";
    strcpy(result, "200 REGISTER");
    return result;
}

/*
    h: username
    k: password
    fd: file descriptor
*/
char *loginUser(std::string h, std::string k, int fd)
{
    char *result = (char *)malloc(MAX_BUFLEN * sizeof(char));

    pthread_mutex_lock(&baseMutex);
    std::vector<User>::iterator itr;
    /* check if username and password match existing user */
    for(itr = userBase.begin(); itr != userBase.end(); itr++)
    {
        if(h.compare(itr->getHandle()) == 0)
        {
            pthread_mutex_unlock(&baseMutex);
            if(k.compare(itr->getKey()) != 0)
            {
                std::cout << "Failed to log in " << h 
                          << " with password *****\n";
                strcpy(result, "500 LOGIN");
                return result;
            }
            else
            {
                pthread_mutex_lock(&onlineMutex);
                std::map<int, User>::iterator ltr;
                /* check if user is already online */
                for(ltr = onlineUsers.begin(); ltr != onlineUsers.end(); ltr++)
                {
                    if(ltr->second.getHandle().compare(h) == 0)
                    {
                        pthread_mutex_unlock(&onlineMutex);
                        strcpy(result, "500 LOGIN");
                        return result;
                    }    
                }
                std::cout << "succesfully log in " << h 
                          << " with password *****\n";
                pthread_mutex_unlock(&onlineMutex);
                strcpy(result, "200 LOGIN");
                return result;
            }
        }
    }
    pthread_mutex_unlock(&baseMutex);

    std::cout << "failed to log in " << h 
              << " with password *****\n";
    strcpy(result, "500 LOGIN");
    return result;
}

/*
    h: username
    fd: file descriptor 
*/

void sendFriends(std::string h, int fd)
{
/* file does not exist in current dir */
    std::ifstream file(userFilename);
    bool done = false;

    if(file.is_open() == false)
    {
        printError(INVALID_FILE, userFilename, SERVER);
        exit(EXIT_FAILURE);
    }

    std::string line;
    char friends[MAX_BUFLEN];
    std::string data;
    /* parse and store user, password, and friends */
    while(getline(file, line) && !done)
    {
        std::istringstream ss(line);
        std::string handle, key, amigo;
        getline(ss, handle, '|');
        if(handle.compare(h) == 0)
        {
            getline(ss, key, '|');
            data = "200 LOGIN";
            while(getline(ss, amigo, ';'))
            {
                if((data.length() + amigo.length()) >= MAX_BUFLEN)
                {
                    std::copy(data.begin(), data.end(), friends);
                    write(fd, friends, data.length());
                    memset(friends, 0, MAX_BUFLEN);
                    data.clear();
                    data = "200 LOGIN";
                }
                else
                {
                    data = data + " " + amigo;
                }
            }
            std::copy(data.begin(), data.end(), friends);
            write(fd, friends, data.length());
            done = true;
        }
    }

    return;
}

/*
    h1: first username
    h2: second username
*/
void addFriends(std::string h1, std::string h2)
{
    pthread_mutex_lock(&baseMutex);
    std::vector<User>::iterator itr;
    bool h1done = false, h2done = false;
    /* add each other to friends list in userBase */
    for(itr = userBase.begin(); itr != userBase.end(); ++itr)
    {
        if(itr->getHandle().compare(h1) == 0)
        {
            itr->addFriend(h2);
            h1done = true;
        }
        if(itr->getHandle().compare(h2) == 0)
        {
            itr->addFriend(h1);
            h2done = true;
        }
        if(h1done && h2done)
        {
            break;
        }
    }
    pthread_mutex_unlock(&baseMutex);

    pthread_mutex_lock(&fileMutex);
    std::ifstream file(userFilename);
    std::ofstream tmp("tmp");
    if(file.is_open() == false)
    {
       printError(INVALID_FILE, userFilename, SERVER);
       exit(EXIT_FAILURE);
    }
    if(tmp.is_open() == false)
    {
       printError(INVALID_FILE, userFilename, SERVER);
       exit(EXIT_FAILURE);
    }

    /* parse file to add h1 as h2's friend and vice verse */
    std::string line;
    while(getline(file, line))
    {    
        if((line.compare(0, h1.length(), h1) == 0) && (line.find(h2) == std::string::npos))
        {
            if(line.back() == '|')
                tmp << line << h2 << "\n";
            else
                tmp << line << ";" << h2 << "\n";
        }
        else if((line.compare(0, h2.length(), h2) == 0) && (line.find(h1) == std::string::npos))
        {
            if(line.back() == '|')
                tmp << line << h1 << "\n";
            else
                tmp << line << ";" << h1 << "\n";
        }
        else
        {
            tmp << line << "\n";
        }
    }
    tmp.close();
    file.close();
    remove(userFilename);
    rename("tmp", userFilename);
    pthread_mutex_unlock(&fileMutex);

    return;
}

/*
    sig: signal
*/
void handleInterrupt(int sig)
{
    pthread_mutex_lock(&onlineMutex);
    std::map<int, User>::iterator itr;
    std::cout << " Server shutting down...\n";
    /* close all fds for each connected client */
    for(itr = onlineUsers.begin(); itr != onlineUsers.end(); ++itr)
    {
        std::cout << "User " << itr->second.getHandle() << "'s connection' at fd " << itr->first << " terminated\n";
        close(itr->first);
    }
    pthread_mutex_unlock(&onlineMutex);
    close(sockfd);
    std::cout << "Server shutdown\n";

    exit(EXIT_SUCCESS);
}
