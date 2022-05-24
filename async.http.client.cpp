#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

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
  std::string readUntil(char until, const char* arr, int &counter)
    {
        std::string result = "";
        while (arr[counter] != until) {
            result += recieved_[counter++];
        }

        return result;
    }

    void static CopyFromStringToCharArray(const std::string& str, size_t str_size, char* arr)
    {
        for (size_t counter = 0; counter < str_size; ++counter)
        {
            arr[counter] = str[counter];
        }
    }

    std::string PrepareStringTasksResponse(std::vector<Task> arr, UserDBManager& UDBM)
    {
        std::string result = "";
        size_t size = arr.size();
        if (arr.size() > 0)
        {
            for (size_t counter = 0; counter < size - 1; ++counter)
            {
                result += std::to_string(arr[counter].id) + ":" + arr[counter].head + ":" + UDBM.get_user(arr[counter].assigner_id).login
                          + ":" + UDBM.get_user(arr[counter].executor_id).login + ":" + (arr[counter].completion ? "1":"0") + ":";
            }
            result += std::to_string(arr[size - 1].id) + ":" + arr[size - 1].head + ":" + UDBM.get_user(arr[size - 1].assigner_id).login
                      + ":" + UDBM.get_user(arr[size - 1].executor_id).login + ":" + (arr[size-1].completion ? "1":"0");
        }

        return result;
    }

    void EmptyBuffer(char* arr)
    {
        for (size_t counter = 0; counter < max_length; ++counter)
        {
            arr[counter] = '\r';
        }
    }

    void Process()
    {
        UserDBManager UDBM(DB_conn.session);
        TaskDBManager TDBM(DB_conn.session);

        int i = 0, task_id = 0;
        std::string request, username, head, assigner, executor, task_state;
        request = readUntil(':', recieved_, i);

        ++i;

        if (request == "authorization") {
            username = readUntil('\r', recieved_, i);

            User usr(username);
            UDBM.add_user(usr);
            int id = UDBM.search_user(username).id;
            to_send = username + ":" + std::to_string(id) + "\r" + "\n";
            CopyFromStringToCharArray(to_send, to_send.size(), sent_);

            send_required = true;
        }
        else {
            if (request == "add") {
                head = readUntil(':', recieved_, i);
                ++i;
                assigner = readUntil(':', recieved_, i);
                ++i;
                executor = readUntil('\r', recieved_, i);
                try{
                    int assigner_id = UDBM.search_user(assigner).id;
                    int executor_id = UDBM.search_user(executor).id;
                    Task tsk(head, assigner_id, executor_id);
                    TDBM.add_task(tsk);
                }
                catch (DB_except) {}

                send_required = false;
            }
            else {
                if (request == "get") {
                    std::vector<Task> tasks;
                    username = readUntil('\r', recieved_, i);
                    int id = UDBM.search_user(username).id;
                    tasks = TDBM.get_user_tasks(id);
                    to_send = std::to_string(tasks.size()) + ":"
                              + PrepareStringTasksResponse(tasks, UDBM)
                              + "\r" + "\n";
                    CopyFromStringToCharArray(to_send, to_send.size(), sent_);

                    send_required = true;
                } else {
                    if (request == "edit") {
                        task_id = std::stoi(readUntil(':', recieved_, i));
                        ++i;
                        head = readUntil(':', recieved_, i);
                        ++i;
                        assigner = readUntil(':', recieved_, i);
                        ++i;
                        executor = readUntil(':', recieved_, i);
                        ++i;
                        task_state = readUntil('\r', recieved_, i);
                        try {
                            if (task_state == "0") {
                                TDBM.incomplete_task(task_id);
                            } else {
                                TDBM.complete_task(task_id);
                            }
                        }
                        catch (DB_except) {}

                        send_required = false;
                    }
                }
            }
        }
        EmptyBuffer(recieved_);
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

  void handle_read_content(const boost::system::error_code& err)
  {
    if (!err)
    {
      // Continue reading remaining data until EOF.
      boost::asio::async_read(socket_, response_buf_,
                              boost::asio::transfer_at_least(1),
                              boost::bind(&Client::handle_read_content, this,
                                          boost::asio::placeholders::error));
    }
    else if (err == boost::asio::error::eof)
    {
      std::istream response_stream(&response_buf_);
      response_.body = std::string(std::istreambuf_iterator<char>(response_stream), std::istreambuf_iterator<char>());
      std::cout << response_;
    }
    else if (err != boost::asio::error::eof)
    {
      std::cout << "Error: " << err << "\n";
    }
  }

 private:
  tcp::resolver resolver_;
  tcp::socket socket_;

  boost::asio::streambuf response_buf_;

  Request request_;
  Response response_;
};

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 4)
    {
      std::cout << "Usage: async_client <server> <port> <path>\n";
      std::cout << "Example:\n";
      std::cout << "  async_client www.boost.org /LICENSE_1_0.txt\n";
      return 1;
    }

    boost::asio::io_context io_context;

    Client c(io_context, argv[1], argv[2], argv[3]);
    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cout << "Exception: " << e.what() << "\n";
  }

  return 0;
}