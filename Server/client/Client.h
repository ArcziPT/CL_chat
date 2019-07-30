#ifndef CLIENT_H
#define CLIENT_H

#include "../../include/socket/Socket.h"
#include "../../include/socket/Request.h"
#include "../../include/TextHelper/Text.h"
#include "../../include/ClassFuncMap/ClassFuncMap.h"

#include <mutex>
#include <ncurses.h>

using string_vector = std::vector<std::string>;

class Client{
public:
    void start();    

private:
    //socket, requests
    Socket main_socket;
    void parse_request(const Request& req);

    //bools
    bool running = true;
    bool connected = false;

    //client data
    unsigned long long session_id = 0;

    //printing thread
    void start_screen_th();

    //print
    void print(const std::string& tag, const std::string& msg, int color);
    void print(const std::vector<Text>& texts);
    std::mutex screen_queue_mutex;
    std::vector<Text> screen_queue;

    //input synch
    std::mutex input_mutex;
    bool req_input = false;

    //input
    std::string nline = "<Client>: ";
    void check_input();

    //ncurses
    void init_ncurses();
    WINDOW* input_win;
    WINDOW* msg_win;
    WINDOW* nline_win;

    //use when using ncurses(writing to terminal)
    std::mutex ncurses_mutex;

    //ncurses colors
    const int color_msg = 1;
    const int color_server = 2;
    const int color_error = 3;
    const int color_client = 4;

    //command map
    ClassFuncMap<std::string, Client, const string_vector&> command_func_map;
    void init_command_func_map();

    void c_connect(const string_vector& vec);
    void c_quit(const string_vector& vec);
    void c_register(const string_vector& vec);
    void c_ban(const string_vector& vec);
    void c_add(const string_vector& vec);
    void c_admin(const string_vector& vec);

    //action map
    ClassFuncMap<std::string, Client, const string_vector&, const Request&> action_func_map;
    void init_action_func_map();

    void a_send(const string_vector& vec, const Request& req);
    void a_login(const string_vector& vec, const Request& req);
    void a_login_response(const string_vector& vec, const Request& req);
    void a_register(const string_vector& vec, const Request& req);
    void a_register_response(const string_vector& vec, const Request& req);
    void a_ban(const string_vector& vec, const Request& req);
    void a_add(const string_vector& vec, const Request& req);
    void a_permission(const string_vector& vec, const Request& req);
    void a_access(const string_vector& vec, const Request& req);

    //...
    void connect(const std::string& ip, int port);
};

#endif