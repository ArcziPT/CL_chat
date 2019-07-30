#ifndef TEXT_H
#define TEXT_H

#include <iostream>

struct Text{
    Text(){};

    Text(const std::string& tag, const std::string& msg, int color){
        this->tag = tag;
        this->msg = msg;
        this->color = color;
    };

    std::string tag;
    std::string msg;
    int color;
};

#endif