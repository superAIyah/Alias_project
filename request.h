#ifndef BOOST_ASIO_SERVER_REQUEST_H
#define BOOST_ASIO_SERVER_REQUEST_H
#include <unordered_map>

struct Request
{
    std::string method;
    std::unordered_map<std::string, std::string> parameters;
};

inline std::ostream& operator<<(std::ostream& os, const Request& request)
{
    os << request.method << std::endl;
    for (auto const& x : request.parameters){
        os << x.first<<": "<<x.second << std::endl;
    }
    return os;
}

inline std::string Request2String(const Request &request)
{
    std::stringstream ss;
    ss << request.method<<"\r\n";
    std::unordered_map<std::string, std::string> buffer;
    for (auto const& x : request.parameters){
        buffer[x.first] = x.second;
    }
    for (auto const& x : buffer){
        if(x.first != "is_word"){
            ss<<x.second << "\r\n";
        }
    }

    return ss.str();
}

#endif //BOOST_ASIO_SERVER_REQUEST_H
