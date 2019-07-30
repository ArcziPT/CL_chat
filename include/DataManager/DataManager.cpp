#include "DataManager.h"
#include "../TextHelper/TextHelper.h"

#include <iostream>
#include <fstream>

std::vector<User*> DataManager::get_users(const std::string& col_name, const std::string& val){
    std::vector<User*> users;
    auto file = open(files["users"]);

    if(col_name == "hash"){
        return users;
    }

    auto lines = TextHelper::get_lines(file);
    //first line contains column names
    auto col_names = TextHelper::split(lines[0], ' ');
    auto pos = std::find(col_names.begin(), col_names.end(), col_name) - col_names.begin();

    for(auto it = lines.begin() + 1; it != lines.end(); it++){
        auto values = TextHelper::split(*it, ' ');

        if(values[pos] == val or val == "*"){
            auto user = new User();
            
            user->username = values[1];
            user->id = std::stoi(values[0]);
            user->hash = values[2];
            
            users.push_back(user);
        }
    }
    return users;
}

void DataManager::set_user(User* user){
    auto file = open(files["users"]);
    auto data = std::to_string(user->id) + std::string(" ") + user->username + std::string(" ") + user->hash + std::string("\n");

    auto users = TextHelper::get_lines(file);
    //users[0] contains column names
    auto it = std::find_if(users.begin() + 1, users.end(), [user](auto& u) -> bool{return user->id == stoi(TextHelper::split(u, ' ')[0]);});
    users.emplace(it, data);
    users.erase(it);

    auto new_file = TextHelper::merge_newline(users);
    replace(new_file, files["users"]);
}

bool DataManager::add_user(User* user){
    auto users = get_channels("name", user->username);
    if(!users.empty())
        return false;


    auto file = files["users"];
    auto data = std::to_string(get_new_user_id()) + std::string(" ") + user->username + std::string(" ") + user->hash + std::string("\n");   
    append(data, file);
    return true;
}

void DataManager::remove_user(User* user){
    auto file = open(files["users"]);

    auto users = TextHelper::get_lines(file);
    auto it = std::find_if(users.begin() + 1, users.end(), [user](auto& u) -> bool{return user->id == stoi(TextHelper::split(u, ' ')[0]);});
    users.erase(it);

    auto new_file = TextHelper::merge(users);
    replace(new_file, files["user"]);
}

void DataManager::change_permission(bool admin, int id, Channel* channel){
    if(admin){
        auto data = std::to_string(id) + std::string("\n");
        append(data, channel->admins_file);
    } else{
        auto file = open(channel->admins_file);

        auto ids = TextHelper::get_lines(file);
        auto it = std::find(ids.begin(), ids.end(), std::to_string(id));
        ids.erase(it);

        auto new_file = TextHelper::merge(ids);
        replace(new_file, channel->admins_file);
    }
}

void DataManager::ban_user(int id, Channel* channel){
    auto data = std::to_string(id) + std::string("\n");
    append(data, channel->banned_file);
}

void DataManager::add_user_to_channel(int id, Channel* channel){
    auto data = std::to_string(id) + std::string("\n");
    append(data, channel->user_file);
}

int DataManager::get_new_user_id(){
    auto file = open(files["users"]);
    auto users = TextHelper::get_lines(file);

    return std::stoi(TextHelper::split(users[users.size() - 1], ' ')[0]) + 1;
}

std::vector<Channel*> DataManager::get_channels(const std::string& col_name, const std::string& val){
    std::vector<Channel*> channels;
    auto file = open(files["channels"]);

    auto lines = TextHelper::get_lines(file);
    //first line contains column names
    auto col_names = TextHelper::split(lines[0], ' ');
    auto pos = std::find(col_names.begin(), col_names.end(), col_name) - col_names.begin();

    for(auto it = lines.begin() + 1; it != lines.end(); it++){
        auto values = TextHelper::split(*it, ' ');

        if(values[pos] == val or val == "*"){
            auto channel = new Channel();

            channel->id = std::stoi(values[0]);
            channel->name = values[1];
            channel->port = std::stoi(values[2]);
            channel->logging = (values[3] == "1" ? true : false);
            
            channel->banned_ids = get_ids(values[4]);
            channel->users_ids = get_ids(values[5]);
            channel->admins_ids = get_ids(values[6]);

            channel->banned_file = values[4];
            channel->user_file = values[5];
            channel->admins_file = values[6];

            channels.push_back(channel);
        }
    }
    return channels;
}

void DataManager::set_channel(Channel* channel){
    auto file = open(files["channels"]);
    auto data = std::to_string(channel->id) + std::string(" ") + channel->name + std::string(" ") + std::to_string(channel->port) + std::string(" ") + (channel->logging == 1 ? std::string("1") : std::string("0")) + std::string(" ") + channel->banned_file + std::string(" ") + channel->user_file + std::string(" ") + channel->admins_file + std::string("\n");

    auto channels = TextHelper::get_lines(file);

    auto it = std::find_if(channels.begin() + 1, channels.end(), [channel](auto& c) -> bool{return channel->id == stoi(TextHelper::split(c, ' ')[0]);});
    channels.emplace(it, data);
    channels.erase(it);

    auto new_file = TextHelper::merge(channels);
    replace(new_file, files["channels"]);
}

bool DataManager::add_channel(Channel* channel){
    auto channels = get_channels("name", channel->name);
    if(!channels.empty())
        return false;
    
    auto file = files["channels"];
    auto data = std::to_string(get_new_channel_id()) + std::string(" ") + channel->name + std::string(" ") + std::to_string(channel->port) + std::string(" ") + (channel->logging == 1 ? std::string("1") : std::string("0")) + std::string(" ") + channel->banned_file + std::string(" ") + channel->user_file + std::string(" ") + channel->admins_file + std::string("\n");
    append(data, file);
    return true;
}

void DataManager::remove_channel(Channel* channel){
    auto file = open(files["channels"]);

    auto channels = TextHelper::get_lines(file);
    auto it = std::find_if(channels.begin() + 1, channels.end(), [channel](auto& c) -> bool{return channel->id == stoi(TextHelper::split(c, ' ')[0]);});
    channels.erase(it);

    auto new_file = TextHelper::merge(channels);
    replace(new_file, files["channels"]);
}

int DataManager::get_new_channel_id(){
    auto file = open(files["channels"]);
    auto channels = TextHelper::get_lines(file);

    return std::stoi(TextHelper::split(channels[channels.size() - 1], ' ')[0]) + 1;
}

std::string DataManager::open(const std::string& file){
    std::ifstream t(file);
    t.seekg(0, std::ios::end);
    size_t size = t.tellg();
    std::string buffer(size, ' ');
    t.seekg(0);
    t.read(&buffer[0], size);

    return buffer;
}

void DataManager::append(const std::string& data, const std::string& file){
    std::ofstream outfile;

    outfile.open(file, std::ios_base::app);
    outfile << data;
    outfile.close();
}

void DataManager::replace(const std::string& data, const std::string& file){
    std::ofstream outfile;

    outfile.open(file);
    outfile << data;
    outfile << std::flush;
    outfile.close();
}

std::vector<int> DataManager::get_ids(const std::string& filename){
    if(filename == "*"){
        return std::vector<int>(0); 
    }else{  
        auto file = open(filename);
        auto t_ids = TextHelper::split(file, '\n');

        std::vector<int> ids;

        for(auto id : t_ids){
            ids.push_back(stoi(id));
        }

        return ids;
    }
}