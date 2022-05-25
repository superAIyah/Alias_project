#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <unordered_map>

#include "response.h"
#include "request.h"

using boost::asio::ip::tcp;


class Client
{
 public:
  Client(boost::asio::io_context& io_context,
         const std::string& server, const std::string& port, const std::string& path)
      : resolver_(io_context),
        socket_(io_context)
  {
    request_.method = "GET";
    request_.path = path;
    request_.http_version = "HTTP/1.0";
    request_.host = server;


    // Start an asynchronous resolve to translate the server and service names
    // into a list of endpoints.
    resolver_.async_resolve(server, port,
                            boost::bind(&Client::handle_resolve, this,
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::results));
  }

 private:
  typedef struct
  {
    std::string login;
    int difficulty;
    int num_of_players;
    int num_of_teams;
    int round_duration;
  } send_settings_struct;

  typedef struct 
  {
    int game_id;
    int team_id;
    std::vector <std::string> users_login;
    std::string host;
  } get_settings_struct;

struct Request
{
    std::string method;
    std::unordered_map<std::string, std::string> parameters;
}; 


void parse_settings(std::string req_data)
{
	std::vector<std::string> request_arr;
	boost::split(request_arr, req_data, [](char c) { return c == ':'; });

	settings.game_id = std::stoi(request_arr[1]);
	settings.team_id = std::stoi(request_arr[2]);
  settings.host = request_arr[3];
  std::vector<std::string> users_login;
  for (size_t i = 3; i != request_arr.size(); i++)
    users_login.push_back(request_arr[i]);
  settings.users_login = users_login;
}

// 	if (request.method == "msg")
//   {
// 		request.parameters["user_login"] = request_arr[1];
// 		request.parameters["game_id"] = request_arr[2];
// 		request.parameters["team_id"] = request_arr[3];
// 		request.parameters["text"] = request_arr[4];
// 		request.parameters["who"] = request_arr[5];//ведущий или отгадывающий
// 	}
// 	if (request.method == "round") {
// 		request.parameters["game_id"] = request_arr[1];
// 	}


// 	return request;
// }


  void Process()
  {
//    std::string protocol = get_from_gui();
    if (protocol == "settings")
    {
      // Получил данные от Феди
      // char[10000] send = data – Данные в строке для Влада

//      функция которая считывает ообщение от сервера пусть хранится у тебя в переменной под названием recieve_
// recieve - settings:game_id:team_id:player1_login:player2_login:player3_login: 
      parse_settings(std::string(recieve_)); //parse_settings запихнул данные в атрибут settings
      //
      // Федя-Мансур
      //
    }
    else
    {
    
    }
  }
  void handle_resolve(const boost::system::error_code& err,
                      const tcp::resolver::results_type& endpoints)
  {
    if (!err)
    {
      // Attempt a connection to each endpoint in the list until we
      // successfully establish a connection.
      boost::asio::async_connect(socket_, endpoints,
                                 boost::bind(&Client::handle_connect, this,
                                             boost::asio::placeholders::error));
    }
    else
    {
      std::cout << "Error: " << err.message() << "\n";
    }
  }

  void handle_connect(const boost::system::error_code& err)
  {
    if (!err)
    {
      std::string buffer = Request2String(request_);

      // The connection was successful. Send the request.
      boost::asio::async_write(socket_, boost::asio::buffer(buffer.data(), buffer.size()),
                               boost::bind(&Client::handle_write_request, this,
                                           boost::asio::placeholders::error));
    }
    else
    {
      std::cout << "Error: " << err.message() << "\n";
    }
  }

  void handle_write_request(const boost::system::error_code& err)
  {
    if (!err)
    {
      // Read the response status line. The response_ streambuf will
      // automatically grow to accommodate the entire line. The growth may be
      // limited by passing a maximum size to the streambuf constructor.
      boost::asio::async_read_until(socket_, response_buf_, "\r\n",
                                    boost::bind(&Client::handle_read_status_line, this,
                                                boost::asio::placeholders::error));
    }
    else
    {
      std::cout << "Error: " << err.message() << "\n";
    }
  }

  void handle_read_status_line(const boost::system::error_code& err)
  {
    if (!err)
    {
      // Check that response is OK.
      std::istream response_stream(&response_buf_);
      response_stream >> response_.http_version;
      response_stream >> response_.status_code;
      std::getline(response_stream, response_.status_message);

      if (response_.status_code != 200)
      {
        std::cout << "Response returned with status code ";
        std::cout << response_.status_code << "\n";
        return;
      }

      // Read the response headers, which are terminated by a blank line.
      boost::asio::async_read_until(socket_, response_buf_, "\r\n\r\n",
                                    boost::bind(&Client::handle_read_headers, this,
                                                boost::asio::placeholders::error));
    }
    else
    {
      std::cout << "Error: " << err << "\n";
    }
  }

  void handle_read_headers(const boost::system::error_code& err)
  {
    if (!err)
    {
      // Process the response headers.
      std::istream response_stream(&response_buf_);

      std::string header;
      while (std::getline(response_stream, header) && header != "\r")
        response_.headers.push_back(header);

      // Start reading remaining data until EOF.
      boost::asio::async_read(socket_, response_buf_,
                              boost::asio::transfer_at_least(1),
                              boost::bind(&Client::handle_read_content, this,
                                          boost::asio::placeholders::error));
    }
    else
    {
      std::cout << "Error: " << err << "\n";
    }
  }

void handle_read_headers(const boost::system::error_code& err)
  {
    std::cout << "Exception: " << e.what() << "\n";
  }

  return 0;
}