/*###############################
  #        David Miller         #
  #       COP 5570 - CPD        #
  #         Project 2           #
  #        Nov 29 2018          #
  ###############################*/

#ifndef USER_HEADER
#define USER_HEADER

#include <string>
#include <vector>

/* for some reason I get compilation errors when removing const key word so
   I guess they;re staying there :) */
class User
{
public:
    User(const std::string &, const std::string &);       /* constructor */
    void addFriend(const std::string &);                  /* add a friend */
    void setLocation(const std::string &,                 /* set ip and port */
                     const std::string &);                
    std::string getHandle() const;                        /* get user handle */
    std::string getKey() const;                           /* get user key */
    std::string getIp() const;                            /* get ip adddress */
    std::string getPort() const;                          /* get port number */
    std::vector<std::string> friends;                     /* friends vector */

private:
    std::string handle;                                   /* user's handle */
    std::string key;                                      /* user's password */
    std::string ip;                                       /* ip address */
    std::string port;                                     /* port number */
};

#endif
