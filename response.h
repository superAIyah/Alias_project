#ifndef BOOST_ASIO_SERVER_RESPONSE_H
#define BOOST_ASIO_SERVER_RESPONSE_H
#include <unordered_map>

struct Response
{
  std::string method;
  std::unordered_map<std::string, std::string> parameters;
};

inline std::ostream& operator<<(std::ostream& os, const Response& response)
{
  os << response.method << std::endl;
  for (auto const& x : response.parameters){
    os << x.first<<": "<<x.second << std::endl;
  }
  return os;
}

inline std::string Response2String(const Response &response)
{
  std::stringstream ss;
  ss << response.method<<"\r\n";
  std::unordered_map<std::string, std::string> buffer;
  for (auto const& x : response.parameters){
    buffer[x.first] = x.second;
  }
  for (auto const& x : buffer){
    ss<<x.second << "\r\n";
  }

  return ss.str();
}
#endif //BOOST_ASIO_SERVER_RESPONSE_H