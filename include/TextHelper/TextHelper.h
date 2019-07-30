#ifndef TEXTHELPER_H
#define TEXTHELPER_H

#include <string>
#include <vector>

namespace TextHelper{
    std::vector<std::string> get_lines(const std::string& str);
    bool does_contain(const std::string& str, const std::string& p);
    bool does_contain(const std::vector<std::string>& lines, const std::string& p);

    std::string get_tag(const std::string& str);
    std::vector<std::string> get_tags(const std::vector<std::string>& lines);

    std::string get_tag_value(const std::string& str, const std::string& tag);
    std::string get_tag_value(const std::vector<std::string>& lines, const std::string& tag);

    std::vector<std::string> split(const std::string& s, char delimiter);
    std::string merge(const std::vector<std::string>& strs);
    std::string merge_newline(const std::vector<std::string>& strs);
}

#endif