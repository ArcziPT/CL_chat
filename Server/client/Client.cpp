#include <iostream>

#include "Client.h"
#include "../../include/TextHelper/TextHelper.h"

void Client::start(){
    init_action_func_map();
    init_command_func_map();

    init_ncurses();

    std::thread screen(&Client::start_screen_th, this);

    while(running){
        check_input();
    }

    endwin();
}

void Client::start_screen_th(){
    while(running){
        if(connected){
            auto dis = main_socket.get_disconnected();
            if(!dis.empty())
                exit(-1);//disconnected from server

            main_socket.refresh();
            auto req = main_socket.get_request();
            while(req != nullptr){ //if req == nullptr, there is no requests
                parse_request(*req);
                delete req;
                req = main_socket.get_request();
            }
        }

        screen_queue_mutex.lock();
        ncurses_mutex.lock();

        for(auto text : screen_queue){
            wattron(msg_win, COLOR_PAIR(text.color));
            wprintw(msg_win, text.tag.c_str());
            wattroff(msg_win, COLOR_PAIR(text.color));
            wprintw(msg_win, " ");
            wprintw(msg_win, text.msg.c_str());
            wprintw(msg_win, "\n");
            wrefresh(msg_win);
        }

        if(!screen_queue.empty()){
            wcursyncup(input_win);
            wrefresh(input_win);
        }

        screen_queue.clear();

        ncurses_mutex.unlock();
        screen_queue_mutex.unlock();
    }
}

void Client::check_input(){
    ncurses_mutex.lock();
    wclear(input_win);
    wrefresh(input_win);
    ncurses_mutex.unlock();

    input_mutex.lock();
    std::string line = "";
    char c;
    while((c = wgetch(input_win)) != '\n'){
        line += c;
    }
    input_mutex.unlock();

    ncurses_mutex.lock();
    wclear(input_win);
    wrefresh(input_win);
    ncurses_mutex.unlock();

    if(line[0] != '!'){  //if not command
        Request req;
        if(session_id == 0 or !connected)
            return;

        print(nline, line, color_client);

        req.msg = std::string("action:send\nsession_id:") + std::to_string(session_id) + std::string("\nmsg:") + line;
        req.client_id = 0;
        main_socket.send_response(req);

        return;
    }

    auto strs = TextHelper::split(line, ' ');
    //TODO: error check
    auto command = strs[0];

    if(command_func_map.has_key(command))
        command_func_map(this, command, strs);
}

void Client::connect(const std::string& ip, int port){
    main_socket._connect(ip, port);
    connected = true;
}

void Client::parse_request(const Request& req){
    auto lines = TextHelper::get_lines(req.msg);
    auto tags = TextHelper::get_tags(lines);

    if(!TextHelper::does_contain(lines, "action"))
        return;

    auto action = TextHelper::get_tag_value(lines, "action");

    if(action_func_map.has_key(action))
        action_func_map(this, action, lines, req);
}

void Client::print(const std::string& tag, const std::string& msg, int color){
    Text text;
    text.tag = tag;
    text.msg = msg;
    //set color
    text.color = color;
    
    screen_queue_mutex.lock();

    screen_queue.push_back(text);

    screen_queue_mutex.unlock();
}

void Client::print(const std::vector<Text>& texts){
    screen_queue_mutex.lock();

    screen_queue.insert(screen_queue.end(), texts.begin(), texts.end());

    screen_queue_mutex.unlock();
}

void Client::init_ncurses(){
    initscr();
    cbreak();
    start_color();

    init_pair(color_msg, COLOR_BLUE, COLOR_BLACK);
    init_pair(color_server, COLOR_GREEN, COLOR_BLACK);
    init_pair(color_error, COLOR_RED, COLOR_BLACK);
    init_pair(color_client, COLOR_YELLOW, COLOR_BLACK);

    int x, y;
    getmaxyx(stdscr, y, x);

    nline_win = newwin(1, 10, y-1, 0);
    wattron(nline_win, COLOR_PAIR(color_client));
    wprintw(nline_win, nline.c_str());
    wrefresh(nline_win);
    input_win = newwin(1, x-10, y-1, 10);
    msg_win = newwin(y-1, x, 0, 0);
    scrollok(msg_win, true);
}

void Client::init_command_func_map(){
    command_func_map.add_entry("!connect", &Client::c_connect);
    command_func_map.add_entry("!quit", &Client::c_quit);
    command_func_map.add_entry("!register", &Client::c_register);
    command_func_map.add_entry("!ban", &Client::c_ban);
    command_func_map.add_entry("!add", &Client::c_add);
    command_func_map.add_entry("!admin", &Client::c_admin);
}

void Client::c_connect(const string_vector& vec){
    //if connected -> disconect
    //connect to other

    auto ip = vec[1];
    auto port = stoi(vec[2]);

    connect(ip, port);

    if(connected){
        Request req;
        req.msg = "action:connect\n";
        req.client_id = 0;

        main_socket.send_response(req);
    }
}

void Client::c_quit(const string_vector& vec){
    //disconnect
    //close everything
    main_socket.stop();
    running = false;
}

void Client::c_register(const string_vector& vec){
    Request req;
    req.client_id = 0;
    req.msg = "action:register";
    main_socket.send_response(req);
}

void Client::c_ban(const string_vector& vec){
    Request req;
    req.client_id = 0;
    req.msg = "action:ban\nban_username:" + vec[1] + "\nsession_id:" + std::to_string(session_id);
    main_socket.send_response(req);
}

void Client::c_add(const string_vector& vec){
    Request req;
    req.client_id = 0;
    req.msg = "action:add\nadd_username:" + vec[1] + "\nsession_id:" + std::to_string(session_id);
    main_socket.send_response(req);
}

void Client::c_admin(const string_vector& vec){
    Request req;
    req.client_id = 0;
    req.msg = "action:permission\nusername:" + vec[1] + "\nadmin:" + vec[2] + "\nsession_id:" + std::to_string(session_id);
    main_socket.send_response(req);
}

void Client::init_action_func_map(){
    action_func_map.add_entry("send", &Client::a_send);
    action_func_map.add_entry("login", &Client::a_login);
    action_func_map.add_entry("login_response", &Client::a_login_response);
    action_func_map.add_entry("register", &Client::a_register);
    action_func_map.add_entry("register_response", &Client::a_register_response);
    action_func_map.add_entry("ban", &Client::a_ban);
    action_func_map.add_entry("add", &Client::a_add);
    action_func_map.add_entry("permission", &Client::a_permission);
    action_func_map.add_entry("access", &Client::a_access);
}

void Client::a_send(const string_vector& vec, const Request& req){
    auto who = TextHelper::get_tag_value(vec, "who");
    auto msg = TextHelper::get_tag_value(vec, "msg");
    if(who == "<SERVER>")
        print(who, msg, color_server);
    else
        print(who, msg, color_msg);
}

void Client::a_login(const string_vector& vec, const Request& req){
    auto logging = true;
    if(TextHelper::get_tag_value(vec, "logging") == "false")
        logging = false;
        
    Request res;
    res.msg = std::string("action:login");
    res.client_id = req.client_id;
        
    if(logging){
        req_input = true;
        ncurses_mutex.lock();
        input_mutex.lock();
        std::string username = "", password = "";

        wclear(nline_win);
        wprintw(nline_win, "Username:");
        wrefresh(nline_win);

        char c;
        while((c = wgetch(input_win)) != '\n'){
            username += c;
        }

        wclear(nline_win);
        wprintw(nline_win, "Password:");
        wrefresh(nline_win);
        wclear(input_win);
        wrefresh(input_win);

        while((c = wgetch(input_win)) != '\n'){
            password += c;
        }

        wclear(nline_win);
        wprintw(nline_win, nline.c_str());
        wrefresh(nline_win);
        wcursyncup(input_win);
        wrefresh(input_win);
        wclear(input_win);
        wrefresh(input_win);

        input_mutex.unlock();
        ncurses_mutex.unlock();
        req_input = false;

        res.msg = std::string("action:login\nusername:") + username + std::string("\nhash:") + password;
    }

    main_socket.send_response(res);  
}

void Client::a_login_response(const string_vector& vec, const Request& req){
    session_id = stoull(TextHelper::get_tag_value(vec, "session_id"));
    nline = std::string("<") + TextHelper::get_tag_value(vec, "username") + std::string(">: ");

    ncurses_mutex.lock();
    wclear(nline_win);
    wprintw(nline_win, nline.c_str());
    wrefresh(nline_win);
    wcursyncup(input_win);
    wrefresh(input_win);
    ncurses_mutex.unlock();
}

void Client::a_register(const string_vector& vec, const Request& req){
    req_input = true;
    input_mutex.lock();
    ncurses_mutex.lock();

    std::string username = "", password = "";

    wclear(nline_win);
    wprintw(nline_win, "Username:");
    wrefresh(nline_win);

    char c;
    while((c = wgetch(input_win)) != '\n'){
        username += c;
    }

    wclear(nline_win);
    wprintw(nline_win, "Password:");
    wrefresh(nline_win);
    wclear(input_win);
    wrefresh(input_win);

    while((c = wgetch(input_win)) != '\n'){
        password += c;
    }

    wclear(nline_win);
    wprintw(nline_win, nline.c_str());
    wrefresh(nline_win);
    wcursyncup(input_win);
    wrefresh(input_win);
    wclear(input_win);
    wrefresh(input_win);

    Request res;
    res.msg = std::string("action:register_data\nusername:") + username + std::string("\npassword:") + password;
    res.client_id = req.client_id;

    input_mutex.unlock();
    ncurses_mutex.unlock();
    req_input = false;

    main_socket.send_response(res);
}

void Client::a_register_response(const string_vector& vec, const Request& req){
    auto registered = TextHelper::get_tag_value(vec, "registered");

    if(registered == "true")
        print("SERVER:", "You have been registered", color_server);
    else
        print("SERVER:", "Username is already taken", color_server);
    
}

void Client::a_ban(const string_vector& vec, const Request& req){
    print("SERVER:", TextHelper::get_tag_value(vec, "status"), color_server);
}

void Client::a_add(const string_vector& vec, const Request& req){
    print("SERVER:", TextHelper::get_tag_value(vec, "status"), color_server);
}

void Client::a_permission(const string_vector& vec, const Request& req){
    print("SERVER:", TextHelper::get_tag_value(vec, "status"), color_server);
}

void Client::a_access(const string_vector& vec, const Request& req){
    print("SERVER:", TextHelper::get_tag_value(vec, "msg"), color_server);
}