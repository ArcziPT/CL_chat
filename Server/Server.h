#ifndef SERVER_H
#define SERVER_H

#include "channel/Channel.h"
#include "../include/DataManager/DataManager.h"
#include "../include/TextHelper/Text.h"
#include "../include/ClassFuncMap/ClassFuncMap.h"
#include "../include/InterThreadQueue/InterThreadQueue.h"

#include <map>
#include <mutex>
#include <ncurses.h>

using string_vector = std::vector<std::string>;

class Server{
public:
    void start();

private:
    //thread printing on the screen
    void start_screen_th();  
    
    //main loop with input
    void start_input();
    void check_input();
    bool running = true;
    const std::string nline = "<Server>:";

    //channels
    int next_ch = 0;
    int channels_count = 0;
    std::map<int, Channel*> active_channels;
    std::map<int, std::thread> channel_threads;
    void start_channel(int ch);
    void stop_channel(int ch);

    //main channel (lobby)
    Channel* main_channel;
    std::thread main_thread;  
    void start_main_channel();

    //send msg to all active channels
    void send_to_channels(const std::string& msg);

    //data
    DataManager data_manager;
    
    //pass msg to screen_thread to print it on the screen
    InterThreadQueue<Text> msg_queue;
    void print(const std::string& tag, const std::string& msg, int color);
    void print(const std::vector<Text>& texts);

    //ncurses
    void init_ncurses();
    std::mutex ncurses_mutex;
    WINDOW* nline_win;
    WINDOW* input_win;
    WINDOW* msg_win;

    //ncurses colors
    const int color_error = 1;
    const int color_server = 2;

    //command map
    ClassFuncMap<std::string, Server, const string_vector&> command_func_map;
    void init_command_func_map();

    void c_start(const string_vector& vec);
    void c_stop(const string_vector& vec);
    void c_print(const string_vector& vec);
    void c_quit(const string_vector& vec);
    void c_show(const string_vector& vec);
    void c_send(const string_vector& vec);
    void c_add(const string_vector& vec);
    void c_remove(const string_vector& vec);
    void c_help(const string_vector& vec);
};

#endif