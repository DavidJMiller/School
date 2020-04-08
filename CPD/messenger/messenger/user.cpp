/*###############################
  #        David Miller         #
  #       COP 5570 - CPD        #
  #         Project 2           #
  #        Nov 29 2018          #
  ###############################*/

#include "user.h"

/* 
   h: handle for user
   k: key for user
*/
User::User(const std::string &h, const std::string &k)
{
    handle = h;
    key = k;
}

/* 
   f: friend to be added to friends vector
   return: return if f is already a friend or after f is added as a friend
*/
void User::addFriend(const std::string &f)
{
    std::vector<std::string>::iterator itr = friends.begin();
    for(; itr != friends.end(); ++itr)
        if(f.compare(*itr) == 0)
            return;

    friends.push_back(f);
    return;
}

/* 
   i: ip address
   p: port number
*/
void User::setLocation(const std::string &i, const std::string &p)
{
    ip = i;
    port = p;
    return;
}

/*  
   return: handle of user
*/
std::string User::getHandle() const
{
    return handle;
}

/*
   return: key of user
*/
std::string User::getKey() const
{
    return key;
}

/*
   return: key of user
*/
std::string User::getIp() const
{
    return ip;
}

/*
   return: key of user
*/
std::string User::getPort() const
{
    return port;
}
