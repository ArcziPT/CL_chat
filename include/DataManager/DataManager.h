#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <iostream>
#include <vector>
#include <map>

#include "../user/User.h"
#include "../../Server/channel/Channel.h"

class DataManager{
public:
    std::vector<User*> get_users(const std::string& col_name, const std::string& val);
    void set_user(User* user);
    bool add_user(User* user);
    void remove_user(User* user);

    void change_permission(bool admin, int id, Channel* channel);
    void ban_user(int id, Channel* channel);
    void add_user_to_channel(int id, Channel* channel);

    std::vector<Channel*> get_channels(const std::string& col_name, const std::string& val);
    void set_channel(Channel* channel);
    bool add_channel(Channel* channel);
    void remove_channel(Channel* channel);

private:
    int get_new_user_id();
    int get_new_channel_id();

    std::vector<int> get_ids(const std::string& filename);

    std::string open(const std::string& file);
    void append(const std::string& data, const std::string& filename);
    void replace(const std::string& data, const std::string& filename);

    std::map<std::string, std::string> files = {
        {"channels", "channels.txt"},
        {"users", "users.txt"}
    };
};

#endif