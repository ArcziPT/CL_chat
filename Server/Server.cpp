#include "Server.h"
#include "../include/TextHelper/TextHelper.h"

#include <algorithm>

void Server::start(){
    init_command_func_map();

    init_ncurses();

    std::thread screen(&Server::start_screen_th, this);

    start_input();
    endwin();
    screen.join();
}

void Server::start_screen_th(){
    while(running){
        if(!msg_queue.empty()){
            msg_queue.start_data_access();
            ncurses_mutex.lock();
            for(auto text : msg_queue){
                wattron(msg_win, COLOR_PAIR(text.color));
                wprintw(msg_win, text.tag.c_str());
                wprintw(msg_win, " ");
                wattroff(msg_win, COLOR_PAIR(text.color));
                wprintw(msg_win, text.msg.c_str());
                wprintw(msg_win, "\n");
                wrefresh(msg_win);
            }

            wcursyncup(input_win);
            wrefresh(input_win);
            ncurses_mutex.unlock();
        
            msg_queue.clear();
            msg_queue.end_data_access();
        }
    }
}

void Server::start_input(){
    //start main channel
    auto channels = data_manager.get_channels("name", "main");
    if(!channels.empty())
        main_channel = channels[0];
    
    else{
        print("Server:", "no main channel entry in channels.txt", color_error);
    }


    std::thread t(&Server::start_main_channel, this);
    main_thread = std::move(t);

    while(running){
        check_input();
    }
}

void Server::check_input(){
    ncurses_mutex.lock();
    wclear(input_win);
    wrefresh(input_win);
    ncurses_mutex.unlock();

    std::string line = "";
    char c;
    while((c = wgetch(input_win)) != '\n'){
        line += c;
    }

    ncurses_mutex.lock();
    wclear(input_win);
    wrefresh(input_win);
    ncurses_mutex.unlock();

    if(line.empty())
        return;

    auto strs = TextHelper::split(line, ' ');
    auto command = strs[0];

    if(command_func_map.has_key(command))
        command_func_map(this, command, strs);
}

void Server::send_to_channels(const std::string& msg){
    for(auto channel : active_channels){
        channel.second->send(msg);
    }
    main_channel->send(msg);
}

void Server::start_main_channel(){
    main_channel->start(/*&msg_queue*/);
}

void Server::start_channel(int ch){
    channels_count++;
    active_channels[ch]->start(/*&msg_queue*/);
}

void Server::stop_channel(int ch){
    channels_count--;

    active_channels[ch]->stop();

    channel_threads[ch].join();

    delete active_channels[ch];
    active_channels.erase(ch);
    channel_threads.erase(ch);
}

void Server::print(const std::string& tag, const std::string& msg, int color){
    Text text;
    text.tag = tag;
    text.msg = msg;
    text.color = color;
    
    msg_queue.push(text);
}

void Server::print(const std::vector<Text>& texts){
    msg_queue.push(texts);
}

void Server::init_ncurses(){
    initscr();

    cbreak();
    start_color();

    init_pair(color_error, COLOR_RED, COLOR_BLACK);
    init_pair(color_server, COLOR_GREEN, COLOR_BLACK);

    int x, y;
    getmaxyx(stdscr, y, x);
    nline_win = newwin(1, 10, y-1, 0);
    wattron(nline_win, COLOR_PAIR(color_server));
    wprintw(nline_win, nline.c_str());
    wrefresh(nline_win);
    input_win = newwin(1, x - 10, y-1, 10);
    msg_win = newwin(y-1, x, 0, 0);
    scrollok(msg_win, true);
}

void Server::init_command_func_map(){
    command_func_map.add_entry("!start", &Server::c_start);
    command_func_map.add_entry("!stop", &Server::c_stop);
    command_func_map.add_entry("!print", &Server::c_print);
    command_func_map.add_entry("!quit", &Server::c_quit);
    command_func_map.add_entry("!show", &Server::c_show);
    command_func_map.add_entry("!send", &Server::c_send);
    command_func_map.add_entry("!add", &Server::c_add);
    command_func_map.add_entry("!remove", &Server::c_remove);
    command_func_map.add_entry("!help", &Server::c_help);
}

void Server::c_start(const string_vector& vec){
    auto name = vec[1];
    auto channels = data_manager.get_channels("name", name);
    if(channels.empty()){
        print("start", "no such channel exist", color_error);
        return;
    }
        
    auto channel = channels[0];

    print("SERVER:", channel->print_data(), color_server);
        
    next_ch++;
    active_channels.insert(std::pair<int, Channel*>(next_ch, channel));
    channel->ch = next_ch;
        
    std::thread t1(&Server::start_channel, this, next_ch);
    channel_threads.insert(std::pair<int, std::thread>(next_ch, std::move(t1)));
}

void Server::c_stop(const string_vector& vec){
    //!stop <ch/id/name/port>
    auto name = vec[1];
    auto it = std::find_if(active_channels.begin(), active_channels.end(), [name](auto& channel) -> bool{return name == channel.second->name;});

    if(it == active_channels.end())
        return;

    stop_channel((*it).first);
}

void Server::c_print(const string_vector& vec){
    //forward
    print(vec[1], vec[2], color_server);
}

void Server::c_quit(const string_vector& vec){
    for(auto channel : active_channels){
        stop_channel(channel.first);
    }

    running = false;
}

void Server::c_show(const string_vector& vec){
    if(vec.size() < 3){
        print("show:", "!show <channels/users> <col_name>=<value> <-active>", color_error);
        return;
    }

    std::vector<Text> texts;

    if(vec[1] == "users"){
        //    !show users <col_name>=<value> <-active>
        //strs   0    1             2     
        //temp                0         1             
        auto temp = TextHelper::split(vec[2], '=');

        if(temp.size() < 2){
            print("show:", "<col_name>=<value>", color_error);
            return;
        }

        auto col_name = temp[0];
        auto val = temp[1];

        auto users = data_manager.get_users(col_name, val);
                
        for(auto user : users){
            Text text;
            text.msg = user->print_data();
            text.color = color_server;

            texts.push_back(text);
            delete user;
        }
    }

    else if(vec[1] == "channels"){
        //    !show channels <col_name>=<value> <-active>
        //strs   0      1              2            3
        //temp                   0         1 
        texts.push_back(Text("SERVER:", "id  |  name  |  port  |  users_file", color_server));

        auto temp = TextHelper::split(vec[2], '=');

        if(temp.size() < 2){
            print("show:", "<col_name>=<value>", color_error);
            return;
        }

        auto all = true;
        if(vec.size() == 4){
            if(vec[3] == "-active")
                all = false;
        }

        auto col_name = temp[0];
        auto val = temp[1];

        if(all){
            auto channels = data_manager.get_channels(col_name, val);

            for(auto channel : channels){
                Text text;
                text.msg = channel->print_data();
                text.color = color_server;

                texts.push_back(text);
                delete channel;
            }
        }else{
                //TODO
        }
    }

    print(texts);
}

void Server::c_send(const string_vector& vec){
    auto msg = TextHelper::merge(std::vector<std::string>(vec.begin() + 1, vec.end()));
    send_to_channels(msg);
}

void Server::c_add(const string_vector& vec){
    if(vec[1] == "user"){
        //!add user <username> <hash>
        auto user = new User();
        user->username = vec[2];
        user->hash = vec[3];

        data_manager.add_user(user);
    }
    else if(vec[1] == "channel"){
        //!add channel <name> <port> <users>
        auto channel = new Channel();
        channel->name = vec[2];
        channel->port = stoi(vec[3]);
        channel->user_file = vec[4];

        data_manager.add_channel(channel);
    }
}

void Server::c_remove(const string_vector& vec){
    if(vec[1] == "user"){
        //!remove user
        auto temp = TextHelper::split(vec[2], '=');

        if(temp.size() < 2)
            return;

        auto col_name = temp[0];
        auto val = temp[1];

        auto users = data_manager.get_users(col_name, val);
                
        for(auto user : users){
            data_manager.remove_user(user);
            //kick user
            delete user;
        }
    }

    else if(vec[1] == "channel"){
        //!remove channel <name> <port> <users>
        auto temp = TextHelper::split(vec[2], '=');

        if(temp.size() < 2)
            return;

        auto col_name = temp[0];
        auto val = temp[1];

        auto channels = data_manager.get_channels(col_name, val);

        for(auto channel : channels){
            data_manager.remove_channel(channel);
            active_channels.erase(std::find_if(active_channels.begin(), active_channels.end(), [channel](auto c) -> bool{return c.second->id == channel->id;}));
            delete channel;
        }
            
    }
}

void Server::c_help(const string_vector& vec){
    auto commands = command_func_map.get_keys();
    std::vector<Text> texts;

    Text text;
    text.tag = "command";
    for(auto& c : commands){
        text.msg = c;
        text.color = color_server;
        texts.push_back(text);
    }

    print(texts);
}
