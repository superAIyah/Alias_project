#include "connection.h"

#include <vector>
#include <boost/bind/bind.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>

#include <string>
#include <sstream>

#define WORD_PACK_SIZE 3

Request parse(std::string req_data)
{
    Request request;
    std::vector<std::string> request_arr;
    boost::split(request_arr, req_data, [](char c) { return c == ':'; });

    request.method = request_arr[0];

    if (request.method == "settings")
    {
        request.parameters["user_login"] = request_arr[1];
        request.parameters["level"] = request_arr[2];
        request.parameters["num_players"] = request_arr[3];
        request.parameters["num_teams"] = request_arr[4];
    }
    if (request.method == "msg")
    {
        request.parameters["user_login"] = request_arr[1];
        request.parameters["game_id"] = request_arr[2];
        request.parameters["team_id"] = request_arr[3];
        request.parameters["text"] = request_arr[4];
        request.parameters["who"] = request_arr[5];//ведущий или отгдывающий
    }


    return request;
}

std::string SerializeSettings(Request request_) {
    std::stringstream ss;
    ss << "settings:" << request_.parameters.at("level") << ":" << request_.parameters.at("num_players")
       << ":" << request_.parameters.at("num_teams");
    return ss.str();
}

namespace http {
    namespace server3 {

        Connection::Connection(boost::asio::io_context &io_context,
                               Router<std::string(*)(const Request &request)> &requestRouter)
                : strand_(boost::asio::make_strand(io_context)),
                  socket_(strand_),
                  requestRouter_(requestRouter) {}

        boost::asio::ip::tcp::socket &Connection::socket() { return socket_; }

        void Connection::start() {
            socket_.async_read_some(boost::asio::buffer(buffer_),
                                    boost::bind(&Connection::handle_read, shared_from_this(),
                                                boost::asio::placeholders::error,
                                                boost::asio::placeholders::bytes_transferred));
        }

        void Connection::handle_read(const boost::system::error_code &e, std::size_t bytes_transferred)
        {
            if (!e)
            {
                std::cout << buffer_.data() << std::endl;
                Request request = parse(std::string(buffer_.data()));
                if (request.method == "settings")
                {
//                    слепляет в строку ключ
                    std::string settings_str = SerializeSettings(request);

//                  сокеты и логины
                    std::vector<std::pair<boost::asio::ip::tcp::socket *, std::string>> room;

                    int num_teams = std::stoi(request.parameters.at("num_teams"));
                    int num_players = std::stoi(request.parameters.at("num_players"));

                    //количество игроков нужное для начала игры
                    int num_players_needed = num_players * num_teams;
//                    добавляет в линию ожидания
                    Server->WaitingLine[settings_str].players.push_back(
                            std::make_pair(&socket_, request.parameters.at("user_login")));


//                    если этот клиент пришел последним, и теперь в комнате достаточно человек
                    if (Server->WaitingLine.at(settings_str).players.size() == num_players_needed)
                    {

//                        заполняем комнату
                        for (int i = 0; i < num_players_needed; ++i)
                            room.push_back(Server->WaitingLine[settings_str].players[i]);


                        std::cout << "GAME CREATE = ";
                        for (int i = 0; i < num_players_needed; i++)
                            std::cout << room[i].second << "\t";
                        std::cout << "\n\n";

//                      id игры
                        int new_game_id;
                        if (Server->Games.size() != 0)
                        {
//                            последний+1
                            new_game_id = Server->Games.rbegin()->first + 1;
                        }
                        else { new_game_id = 0; }


                        //добавляет игроков в новую игру


                        for (int i = 0; i < num_players_needed; i++)
                        {
                            int team_id = i % num_teams + 1;
                            Server->Games[new_game_id].team_sockets[team_id].push_back(room[i]);
                            //добавляет слова
                            std::vector<std::string> new_words = Server->WDBM->get_words_str(
                                    std::stoi(request.parameters.at("level")), WORD_PACK_SIZE);
                            for (const auto& elem: new_words)
                                Server->Games[new_game_id].team_words[team_id].push(elem);

                            //текущее слово которое отгадывает команда
                            std::string word = Server->Games[new_game_id].team_words[team_id].front();
                        }

                        //НЕ УДАЛЯТЬ!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                        std::cout << "NEW GAME///////////////////////////////\n";
                        for (int i = 1; i <= num_teams; i++)
                        {
                            std::cout << "team_id=" << i << "\n";
                            std::cout << "players:";
                            for (auto k = Server->Games[new_game_id].team_sockets[i].begin(); k != Server->Games[new_game_id].team_sockets[i].end(); k++)
                                std::cout << k->second << "\t";
                            std::cout << "\n";

                            std::cout << "words:";
                            std::queue<std::string> q = Server->Games[new_game_id].team_words[i];
                            while(!q.empty())
                            {
                                std::string val = q.front();
                                q.pop();
                                std::cout << val << " ";
                            }
                            std::cout << "\n\n\n";
                        }



                        for (int i = 0; i != room.size(); i++)
                        {
                            int line_place = i;
                            int team_id = line_place % num_teams + 1;
                            std::stringstream ss;
                            ss << "settings:";
                            ss << new_game_id << ":" << team_id << ":";
                            for (int j = 0; j < num_players_needed; ++j)
                                ss << room[j].second << ":";
                            ss << "\n";//////////////////////////////////////////////////////////////////
                            std::string buffer = ss.str();


                            boost::asio::async_write(*(room[i].first),
                                                     boost::asio::buffer(buffer.data(), buffer.size()),
                                                     boost::bind(&Connection::handle_write,
                                                                 shared_from_this(),
                                                                 boost::asio::placeholders::error));
                        }

                        ////убирает игроков из линии ожидания
                        for (int i = 0; i < num_players_needed; ++i)
                            Server->WaitingLine.at(settings_str).players.erase(
                                    Server->WaitingLine.at(settings_str).players.begin() + i);

                    } else
                        Connection::handle_write(e);

                }
                else if (request.method == "msg")
                {
                    //Здесь код со временем

                    //если время раунда еще не закончилось
                    if (request.parameters.at("who") == "host")
                    {
                        std::cout << "HOST\n\n";

                        int game_id = std::stoi(request.parameters.at("game_id"));
                        int team_id = std::stoi(request.parameters.at("team_id"));
                        std::string message = request.parameters.at("text");
                        std::string login = request.parameters.at("user_login");
                        std::string word = Server->Games[game_id].team_words[team_id].front();

                        bool is_word = false;
                        is_word = message.find(word) != std::string::npos ? true : false;
                        if (is_word == true)
                        {
                            std::cout << "VLADOS AHUEL\n";
                            boost::asio::ip::tcp::socket* socket_presenter = nullptr;
                            std::vector<std::pair<boost::asio::ip::tcp::socket*, std::string>> players = Server->Games[game_id].team_sockets[team_id];
                            
                            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                            //boost::asio::ip::tcp find_presenter(std::string name, std::vector<std::pair<boost::asio::ip::tcp::socket*, std::string>> players)
                                for (size_t i = 0; i < players.size(); i++)
                                    if (players[i].second == login)
                                        socket_presenter = players[i].first;
                            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                            
                            std::string buffer = "Without spoilers!!!\n";
                            boost::asio::async_write(*socket_presenter,
                                                     boost::asio::buffer(buffer.data(), buffer.size()),
                                                     boost::bind(&Connection::handle_write,
                                                                 shared_from_this(),
                                                                 boost::asio::placeholders::error));
                        }
                        else
                        {
                            //msg:user_login:text
                            std::vector<std::pair<boost::asio::ip::tcp::socket*, std::string>> players = Server->Games[game_id].team_sockets[team_id];
                            std::stringstream ss;
                            ss << "msg:";
                            ss << login << ":" << message;
                            ss << "\n";////////////////////////////////////////////////
                            std::string buffer = ss.str();
                            for (size_t i = 0; i < players.size(); i++)
                            {
                                boost::asio::async_write(*(players[i].first),
                                                     boost::asio::buffer(buffer.data(), buffer.size()),
                                                     boost::bind(&Connection::handle_multiwrite,
                                                                 shared_from_this(),
                                                                 boost::asio::placeholders::error));
                            }
                            handle_write(e);
                        }
                    }
                    else
                    {

                        int game_id = std::stoi(request.parameters.at("game_id"));
                        int team_id = std::stoi(request.parameters.at("team_id"));
                        std::string message = request.parameters.at("text");
                        std::string login = request.parameters.at("user_login");
                        std::string word = Server->Games[game_id].team_words[team_id].front();

                        bool is_word = false;
                        is_word = message.find(word) != std::string::npos ? true : false;
                        if (is_word == true)
                        {
                            // если отгадали, ведущему:
                            // guess:user_login:team_id:user_pts:host_pts:key_word

                            // или если отгадали, всем остальным:
                            // guess:user_login:team_id:text:user_pts:host_pts
                            std::stringstream ss;
                            ss << "guess:";
                            ss << login << ":"  << team_id << ":" << message << ":" << USER_GUESS_POINTS << ":" << HOST_GUESS_POINTS;
                            ss << "\n";////////////////////////////////////////////////
                            std::string buffer = ss.str();
                            std::map<int, std::vector< std::pair<boost::asio::ip::tcp::socket*, std::string>>> players = Server->Games[game_id].team_sockets;
                            for (int i = 1; i <= players.size(); i++)
                            {
                                
                                for (int j = 0; j < players[i].size(); j++)
                                {
                                    boost::asio::async_write(*(players[i][j].first),
                                                     boost::asio::buffer(buffer.data(), buffer.size()),
                                                     boost::bind(&Connection::handle_multiwrite,
                                                                 shared_from_this(),
                                                                 boost::asio::placeholders::error));
                                }
                            }
                            handle_write(e);
                            
                            
                            //ТУТ НАДО ДАТЬ ВЕДУЩЕМУ НОВОЕ СЛОВО
                            std::stringstream ss_host;
                            Server->Games[game_id].team_words[team_id].pop();
                            std::string new_word = Server->Games[game_id].team_words[team_id].front();
                            ss_host << "guess:" << Server->Games[game_id].hosts[team_id] << ":" << team_id  << ":" << USER_GUESS_POINTS << ":" << HOST_GUESS_POINTS  << ":" << new_word;

                        }
                        else
                        {
                            //msg:user_login:text
                            std::vector<std::pair<boost::asio::ip::tcp::socket*, std::string>> players = Server->Games[game_id].team_sockets[team_id];
                            std::stringstream ss;
                            ss << "msg:";
                            ss << login << ":" << message;
                            ss << "\n";
                            std::string buffer = ss.str();
                            for (size_t i = 0; i < players.size(); i++)
                            {
                                boost::asio::async_write(*(players[i].first),
                                                     boost::asio::buffer(buffer.data(), buffer.size()),
                                                     boost::bind(&Connection::handle_multiwrite,
                                                                 shared_from_this(),
                                                                 boost::asio::placeholders::error));
                            }
                            handle_write(e);
                        } 
                
                    }
                }
            }
        }

        void Connection::handle_multiwrite(const boost::system::error_code &e)
        {
            if (!e)
            {
                return;
            }
        }

        void Connection::handle_write(const boost::system::error_code &e) {
            if (!e) {
                socket_.async_read_some(boost::asio::buffer(buffer_),
                                        boost::bind(&Connection::handle_read, shared_from_this(),
                                                    boost::asio::placeholders::error,
                                                    boost::asio::placeholders::bytes_transferred));
            }
        }


    } // namespace server3
} // namespace http
