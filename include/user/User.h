#ifndef USER_H
#define USER_H

#include <string>

struct User{
    //from DB
    int id;
    std::string username;
    std::string hash;

    //dynamic data
    int session_id;
    int client_id;

    bool operator==(const User& u){
        return (this->id == u.id and this->client_id == u.client_id);
    }

    std::string print_data(){
        return id + " " + username;
    }
};

#endif