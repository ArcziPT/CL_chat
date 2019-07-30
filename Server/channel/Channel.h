#ifndef CHANNEL_H
#define CHANNEL_H

#include <iostream>
#include <map>
#include <mutex>
#include <random>

#include "../../include/ClassFuncMap/ClassFuncMap.h"
#include "../../include/user/User.h"
#include "../../include/socket/Socket.h"
#include "../../include/InterThreadQueue/InterThreadQueue.h"
#include "../../include/TextHelper/Text.h"

using string_vector = std::vector<std::string>;

class Server;
class Socket;

class Channel{
public:
    void start(/*InterThreadQueue<Text>* msg_queue*/);
    void stop();

    void send(const std::string& msg);

    //data
    std::string print_data();

    int id;
    std::string name;
    int port;
    bool logging;
    std::string banned_file;
    std::string user_file;
    std::string admins_file;

    std::vector<int> banned_ids;
    std::vector<int> admins_ids;
    std::vector<int> users_ids;
    int ch;

private:
    bool isRunning = true;

    //print
    //InterThreadQueue<Text>* msg_queue;
    //void print(const std::string& tag, const std::string& msg);

    //users
    std::map<unsigned long long, User*> sessions;
    std::vector<User*> active_users;

    unsigned long long get_new_session_id();
    void logout_user(int client_id);
    void logout_users(std::vector<int>& client_ids);

    //socket, requests
    Socket main_socket;
    void parse_request(const Request& req);

    //action map
    ClassFuncMap<std::string, Channel, const string_vector&, const Request&> action_func_map;
    void init_action_func_map();

    void a_login(const string_vector& vec, const Request& req);
    void a_connect(const string_vector& vec, const Request& req);
    void a_logout(const string_vector& vec, const Request& req);
    void a_send(const string_vector& vec, const Request& req);
    void a_register(const string_vector& vec, const Request& req);
    void a_register_data(const string_vector& vec, const Request& req);
    void a_ban(const string_vector& vec, const Request& req);
    void a_add(const string_vector& vec, const Request& req);
    void a_permission(const string_vector& vec, const Request& req);
};

#endif