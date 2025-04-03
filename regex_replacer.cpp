#include <string>
#include <map>
#include <regex>
#include <sstream>
#include <iostream>

std::string prepareValue(const std::string &key, const std::map<std::string, std::string> &item)
{
    const auto it = item.find(key);
    if (it == item.end())
        return {};

    return it->second;
}

std::string format(const std::string &fmt, const std::map<std::string, std::string> &item)
{
    std::stringstream str;
    std::regex r(R"(\{\{\s*(\S+)\s*\}\})");
    auto rbegin = std::sregex_iterator(fmt.begin(), fmt.end(), r);
    auto rend = std::sregex_iterator();
    size_t pos = 0;

    for (auto it = rbegin; it != rend; ++it) {
        str << it->prefix().str();
        str << prepareValue(it->str(1), item);
        pos = it->position() + it->length();
    }

    if (pos < fmt.size())
        str << fmt.substr(pos);

    return str.str();
}

int main(){
    const std::string input_string = "Hello {{ name }}! I'm your friend. What do you know about {{ value }}?";
    const std::map<std::string, std::string> values = 
    { 
        { "name", "Vasya" },
        { "value", "tigers" }
    };

    std::cout << "Template string: " << input_string << std::endl;
    std::cout << "Result string: " << format(input_string, values) << std::endl;

    return 0;
}