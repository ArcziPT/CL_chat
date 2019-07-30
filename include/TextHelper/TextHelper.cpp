#include "TextHelper.h"
#include <algorithm>
#include <iostream>
#include <sstream>

namespace TextHelper{
    
    std::vector<std::string> get_lines(const std::string& str){
        return split(str, '\n');
    }


    bool does_contain(const std::string& str, const std::string& p){
        return str.find(p) == std::string::npos ? false : true;
    }
    

    bool does_contain(const std::vector<std::string>& lines, const std::string& p){
        for(auto& line : lines){
            if(does_contain(line, p))
                return true;
        }

        return false;
    }

    std::string get_tag(const std::string& str){
        auto semi = str.find(":");

        if(semi != std::string::npos)
            return std::string(str.begin(), str.begin() + semi);

        return std::string(); 
    }

    std::vector<std::string> get_tags(const std::vector<std::string>& lines){
        std::vector<std::string> tags;

        for(auto& line : lines){
            auto tag = get_tag(line);
            if(tag != std::string(""))
                tags.push_back(tag);
        }

        return tags;
    }


    std::string get_tag_value(const std::string& str, const std::string& tag){
        auto pos = str.find(tag);

        if(pos != std::string::npos){
            auto semi  = str.find(":", pos);
            if(semi != std::string::npos)
                return std::string(str.begin() + semi + 1, str.end());
        }

        return std::string(); 
    }

    std::string get_tag_value(const std::vector<std::string>& lines, const std::string& tag){
        std::string value;

        for(auto& line : lines){
            value = get_tag_value(line, tag);
            if(value != std::string(""))
                return value;
        }

        return std::string();
    }

    std::vector<std::string> split(const std::string& s, char delimiter)
    {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter)){
            tokens.push_back(token);
        }
        return tokens;
    }

    std::string merge(const std::vector<std::string>& strs){
        std::string str;
        for(auto& s : strs){
            str += s;
        }
        return str;
    }

    std::string merge_newline(const std::vector<std::string>& strs){
        std::string str;
        for(auto& s : strs){
            str += s + std::string("\n");
        }
        return str;
    }
}