#include "Channel.h"
#include "../../include/TextHelper/TextHelper.h"
#include "../../include/DataManager/DataManager.h"

#include <string>
#include <algorithm>

void Channel::start(/*InterThreadQueue<Text>* msg_queue*/){
    //this->msg_queue = msg_queue;
    init_action_func_map();

    main_socket.start(port/*, msg_queue*/);

    while(isRunning){
        auto disconnected = main_socket.get_disconnected();
        logout_users(disconnected);

        main_socket.refresh();
        auto req = main_socket.get_request();
        while(req != nullptr){ //if req == nullptr, there is no requests
            parse_request(*req);
            delete req;
            req = main_socket.get_request(); //get next request
        }
    }
}

void Channel::parse_request(const Request& req){
    auto lines = TextHelper::get_lines(req.msg);
    auto tags = TextHelper::get_tags(lines);

    if(!TextHelper::does_contain(tags, "action"))
        return;

    auto action = TextHelper::get_tag_value(lines, "action");

    if(action_func_map.has_key(action))
        action_func_map(this, action, lines, req);
}

unsigned long long Channel::get_new_session_id(){
    unsigned long long id;

    std::random_device rd;
    /* Random number generator */
    std::default_random_engine generator(rd());

    /* Distribution on which to apply the generator */
    std::uniform_int_distribution<long long unsigned> distribution(0,0xFFFFFFFFFFFFFFFF);
        
    do{
        id = distribution(generator);
    }while(sessions.count(id) != 0 and id == 0);

    return id;
}

void Channel::logout_user(int client_id){
    auto it = std::find_if(active_users.begin(), active_users.end(), [client_id](User* u) { return client_id == u->client_id; });
        
    if(it == active_users.end())
        return;

    User* user = *it;

    if(logging)
        std::cout<<user->username<<" logged out\n";

    active_users.erase(it);

    sessions.erase(user->session_id);
    delete user;
}

void Channel::logout_users(std::vector<int>& client_ids){
    for(auto client_id : client_ids){
        logout_user(client_id);
    }
}

void Channel::stop(){
    main_socket.stop();

    for(auto u : active_users){
        delete u;
    }
    isRunning = false;
}

void Channel::send(const std::string& msg){
    Request res;
    auto str = std::string("action:send\nwho:<SERVER>\nmsg:") + msg;
    res.msg = str;
    for(auto user : active_users){
        res.client_id = user->client_id;
        main_socket.send_response(res);
    }
}

std::string Channel::print_data(){
    return id + " " + name + " " + std::to_string(port) + " " + (logging ? std::string("true") : std::string("false"));
}

void Channel::init_action_func_map(){
    action_func_map.add_entry("login", &Channel::a_login);
    action_func_map.add_entry("connect", &Channel::a_connect);
    action_func_map.add_entry("logout", &Channel::a_logout);
    action_func_map.add_entry("send", &Channel::a_send);
    action_func_map.add_entry("register", &Channel::a_register);
    action_func_map.add_entry("register_data", &Channel::a_register_data);
    action_func_map.add_entry("ban", &Channel::a_ban);
    action_func_map.add_entry("add", &Channel::a_add);
    action_func_map.add_entry("permission", &Channel::a_permission);
}

void Channel::a_login(const string_vector& vec, const Request& req){
    auto tags = TextHelper::get_tags(vec);

    if(logging){
        if(!TextHelper::does_contain(tags, "username") or !TextHelper::does_contain(tags, "hash"))
            return;

        auto username = TextHelper::get_tag_value(vec, "username");
        auto hash = TextHelper::get_tag_value(vec, "hash");

        DataManager dm;
        auto users = dm.get_users("username", username);

        if(users.empty())
            return;

        auto user = users[0];

        if((std::find(users_ids.begin(), users_ids.end(), user->id) == users_ids.end() and !users_ids.empty())){
            Request res;
            res.client_id = req.client_id;
            res.msg = "action:access\nmsg:access denied";

            main_socket.send_response(res);
            return;
        }

        if(std::find(banned_ids.begin(), banned_ids.end(), user->id) != banned_ids.end()){
            Request res;
            res.client_id = req.client_id;
            res.msg = "action:access\nmsg:" + username + " is banned";

            main_socket.send_response(res);
            return;
        }

        if(user->hash == hash){
            active_users.push_back(user);
            user->client_id = req.client_id;
            auto session_id = get_new_session_id();

            user->session_id = session_id;
            sessions.insert(std::pair<unsigned long long, User*>(session_id, user));

            Request res;
            res.client_id = req.client_id;
                
            std::string msg = "action:login_response\nsession_id:";
            msg += std::to_string(session_id);
            msg += std::string("\nusername:") + user->username;
            res.msg = msg;

            //print(name + "(Channel)", "new user -> " + user->username);

            main_socket.send_response(res);
        }else{
            Request res;
            res.client_id = req.client_id;
            res.msg = "action:access\nmsg:wrong password";

            main_socket.send_response(res);
        }
    } else{
        auto guest = new User();
        guest->username = "losowe";

        guest->client_id = req.client_id;
        auto session_id = get_new_session_id();

        guest->session_id = session_id;
        sessions.insert(std::pair<unsigned long long, User*>(session_id, guest));

        active_users.push_back(guest);

        Request res;
        res.client_id = req.client_id;
                
        std::string msg = "action:login_response\nsession_id:";
        msg += std::to_string(session_id);
        msg += std::string("\nusername:") + guest->username;
        res.msg = msg;

        main_socket.send_response(res);
    }
}

void Channel::a_connect(const string_vector& vec, const Request& req){
    Request res;
    res.msg = "action:login\nlogging:" + (logging ? std::string("true") : std::string("false"));
    res.client_id = req.client_id;
        
    main_socket.send_response(res);
}

void Channel::a_logout(const string_vector& vec, const Request& req){
    auto tags = TextHelper::get_tags(vec);
    if(!TextHelper::does_contain(tags, "session_id"))
        return;
        //send failure info

    auto session_id = stoull(TextHelper::get_tag_value(vec, "session_id"));
    auto user = sessions[session_id];
    int client_id = user->client_id;

    //print(name + "(Channel)", "user logged out -> " + user->username);
    logout_user(client_id);
        
    Request res;
    res.client_id = req.client_id;
    std::string msg = "status:OK\n";
    res.msg = msg;

    main_socket.send_response(res);
}

void Channel::a_send(const string_vector& vec, const Request& req){
    auto tags = TextHelper::get_tags(vec);
    if(!TextHelper::does_contain(tags, "session_id") or !TextHelper::does_contain(tags, "msg"))
        return;

    auto msg = TextHelper::get_tag_value(vec, "msg");
    auto session_id = stoull(TextHelper::get_tag_value(vec, "session_id"));

    if(sessions.count(session_id) == 0){
        Request res;
        res.client_id = req.client_id;
        res.msg = "action:access\nmsg:access denied";

        main_socket.send_response(res);
        return;
    }

    auto user = sessions[session_id];

    std::string str = "action:send\nwho:";
    str += "<" + user->username + ">";
    str += "\nmsg:" + msg; 
    Request channel_msg;
    channel_msg.msg = str;
        
    for(auto a_user : active_users){
        if(*a_user == *user)
            continue;
        channel_msg.client_id = a_user->client_id;
        main_socket.send_response(channel_msg);
    }

    Request res;
    std::string str2 = "status:OK\n";
    res.msg = str2;
    res.client_id = user->client_id;

    main_socket.send_response(res);
}

void Channel::a_register(const string_vector& vec, const Request& req){
    Request res;
    res.client_id = req.client_id;

    res.msg = "action:register";
    main_socket.send_response(res);
}

void Channel::a_register_data(const string_vector& vec, const Request& req){
    auto username = TextHelper::get_tag_value(vec, "username");
    auto password = TextHelper::get_tag_value(vec, "password");

    auto user = new User();
    user->username = username;
    user->hash = password;

    DataManager dm;
    bool registered = dm.add_user(user);

    Request res;
    res.client_id = req.client_id;
    res.msg = "action:registert_response\nregistered:" + (registered ? std::string("true") : std::string("false"));
    main_socket.send_response(res); 

    delete user;
}

void Channel::a_ban(const string_vector& vec, const Request& req){
    auto session_id = stoull(TextHelper::get_tag_value(vec, "session_id"));
    auto user = sessions[session_id];

    if(std::find(admins_ids.begin(), admins_ids.end(), user->id) != admins_ids.end()){
        auto ban_username = TextHelper::get_tag_value(vec, "ban_username");

        DataManager dm;
        auto users = dm.get_users("username", ban_username);
        if(users.empty())
            return;
            
        dm.ban_user(users[0]->id, this);
        banned_ids.push_back(users[0]->id);

        for(auto u : users){
            delete u;
        }

        Request res;
        res.client_id = req.client_id;
        res.msg = "action:ban\nstatus:ok";
        main_socket.send_response(res);
    }else{
        Request res;
        res.client_id = req.client_id;
        res.msg = "action:ban\nstatus:fail";
        main_socket.send_response(res);
    }
}

void Channel::a_add(const string_vector& vec, const Request& req){
    auto session_id = stoull(TextHelper::get_tag_value(vec, "session_id"));
    auto user = sessions[session_id];

    if(std::find(admins_ids.begin(), admins_ids.end(), user->id) != admins_ids.end()){
        auto add_username = TextHelper::get_tag_value(vec, "add_username");

        DataManager dm;
        auto users = dm.get_users("username", add_username);
        if(users.empty())
            return;
            
        dm.add_user_to_channel(users[0]->id, this);
        users_ids.push_back(users[0]->id);

        for(auto u : users){
            delete u;
        }

        Request res;
        res.client_id = req.client_id;
        res.msg = "action:add\nstatus:ok";
        main_socket.send_response(res);
    }else{
        Request res;
        res.client_id = req.client_id;
        res.msg = "action:add\nstatus:fail";
        main_socket.send_response(res);
    }
}

void Channel::a_permission(const string_vector& vec, const Request& req){
    auto session_id = stoull(TextHelper::get_tag_value(vec, "session_id"));
    auto user = sessions[session_id];

    if(std::find(admins_ids.begin(), admins_ids.end(), user->id) != admins_ids.end()){
        auto username = TextHelper::get_tag_value(vec, "username");

        DataManager dm;
        auto users = dm.get_users("username", username);
        if(users.empty())
            return;
            
        auto admin = TextHelper::get_tag_value(vec, "admin");
        if(admin == "true"){
            dm.change_permission(true, users[0]->id, this);
            admins_ids.push_back(users[0]->id);
        } else{
            dm.change_permission(false, users[0]->id, this);
            admins_ids.erase(std::find(admins_ids.begin(), admins_ids.end(), users[0]->id));
        }          

        for(auto u : users){
            delete u;
        }

        Request res;
        res.client_id = req.client_id;
        res.msg = "action:permission\nstatus:ok";
        main_socket.send_response(res);
    }else{
        Request res;
        res.client_id = req.client_id;
        res.msg = "action:permission\nstatus:fail";
        main_socket.send_response(res);
    } 
}

/*void Channel::print(const std::string& tag, const std::string& msg){
    Text text(tag, msg, 1);
    msg_queue->push(text);
}*/