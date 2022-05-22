#include "connection.h"

#include <vector>
#include <boost/bind/bind.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>

#include <string>
#include <sstream>

#define WORD_PACK_SIZE 3

Request parse(std::string req_data) {
    Request request;
    std::vector<std::string> request_arr;
    boost::split(request_arr, req_data, [](char c) { return c == ':'; });

    request.method = request_arr[0];

    if (request.method == "settings") {
        request.parameters["user_login"] = request_arr[1];
        request.parameters["level"] = request_arr[2];
        request.parameters["num_players"] = request_arr[3];
        request.parameters["num_teams"] = request_arr[4];
    }
    if (request.method == "msg") {
        request.parameters["user_login"] = request_arr[1];
        request.parameters["game_id"] = request_arr[2];
        request.parameters["team_id"] = request_arr[3];
        request.parameters["text"] = request_arr[4];
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
                  requestRouter_(requestRouter){}

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

                //////////////////////////
                // struct Request
                // {
                //     std::string method;
                //     std::unordered_map<std::string, std::string> parameters;
                // };

                for(auto i = request.parameters.begin(); i != request.parameters.end(); i++)
                {
                    std::cout << i->first << "\t" << i->second << "\n";
                }

                /////////////////////////
//                ---------------
                if (request.method == "settings") {
//                    слепляет в строку ключ
                    std::string settings_str = SerializeSettings(request);
//                    добавляет в линию ожидания

                    //количество игроков нужное для начала игры
                    int num_players_needed = std::stoi(request.parameters.at("num_players")) * std::stoi(request.parameters.at("num_teams"));

                    Server->WaitingLine[settings_str].players.push_back( std::make_pair(&socket_, request.parameters.at("user_login")) );


                    int need_players = std::stoi(request.parameters.at("num_players")) * std::stoi(request.parameters.at("num_teams"));

                    if (Server->WaitingLine.at(settings_str).players.size() == need_players)
                    {
                        std::vector<std::pair<boost::asio::ip::tcp::socket*, std::string>> room = Server->WaitingLine[settings_str].players;

                        std::cout << "GAME CREATE = ";
                        for (int i = 0; i < need_players; i++)
                            std::cout << room[i].second << "\t";
                        std::cout << "\n\n";

////////////////////////////////////////////////////////////////////////////////////////////////////////////////ЗАЧЕМ
                        int new_game_id;
                        if(Server->Games.size()!=0)
                            new_game_id = Server->Games.rbegin()->first + 1;
                        else
                            new_game_id = 0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////


                        //добавляет игроков в новую игру
                        int num_teams = std::stoi(request.parameters.at("num_teams"));
                        int num_players = std::stoi(request.parameters.at("num_players"));
                        int j = 0;
                        for (int i = 1; i <= num_teams; i++)
                        {
                            int count = num_players;
                            while (count > 0)
                            {
                                Server->Games[new_game_id].team_sockets[i].push_back(room[j]);
                                count--;
                                j++;
                            }
                            //добавляет слова
                            Server->Games[new_game_id].team_words[i] = Server->WDBM->get_words_str(
                                std::stoi(request.parameters.at("level")), WORD_PACK_SIZE);

                            //текущее слово которое отгадывает команда
                            Server->Games[new_game_id].cur_words[i] = Server->Games[new_game_id].team_words[i][0];
                        }

                        //НЕ УДАЛЯТЬ!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                        // std::cout << "NEW GAME///////////////////////////////\n";
                        // for (int i = 1; i <= num_teams; i++)
                        // {
                        //     std::cout << "team_id=" << i << "\n";
                        //     std::cout << "players:";
                        //     for (auto k = Server->Games[new_game_id].team_sockets[i].begin(); k != Server->Games[new_game_id].team_sockets[i].end(); k++)
                        //         std::cout << k->second << "\t";
                        //     std::cout << "\n";

                        //     std::cout << "words:";
                        //     for (auto k = Server->Games[new_game_id].team_words[i].begin(); k != Server->Games[new_game_id].team_words[i].end(); k++)
                        //         std::cout << *k;
                        //     std::cout << "\n\n\n";
                        // }

                    std::stringstream ss;
                    ss << "settings:";
                    ss << new_game_id << ":" << team_id << ":";
                    for (int i = 0; i < num_players_needed; ++i) {
                        ss << players[i].second << ":";
                    }
                    std::string buffer = ss.str();

                        for(int i = 0; i != room.size(); i++)
                        {
                            boost::asio::async_write(*(room[i].first), boost::asio::buffer(buffer.data(), buffer.size()),
                                                     boost::bind(&Connection::handle_write, shared_from_this(),
                                                                 boost::asio::placeholders::error));
                        }

                        ////убирает игроков из линии ожидания
                        for (int i = 0; i < num_players_needed; ++i)
                        {
                            Server->WaitingLine.at(settings_str).players.erase(
                                    Server->WaitingLine.at(settings_str).players.begin() + i);
                        }

                    }
                    else
                        Connection::handle_write(e);

                } else if (request.method == "msg") {
                    int game_id = std::stoi(request.parameters.at("game_id"));
                    int team_id = std::stoi(request.parameters.at("team_id"));
//                    std::string msgstr = SerializeMsg(request);
                    bool is_word = false;
                    std::vector<std::string> words_in_msg;
                    boost::split(words_in_msg, request.parameters.at("text"),
                                 [](char c) { return c == ' '; });
                    for (int i = 0; i < words_in_msg.size(); ++i) {
                        if (words_in_msg[i] ==
                            Server->Games.at(game_id).cur_words[team_id]) { is_word = true; }
                    }

                    request.parameters["is_word"] = (is_word ? "true" : "false");

//                    std::string buffer = requestRouter_.processRoute(request.method, request);
                    Response mes2send;
                    if(request.parameters.at("is_word") == "true"){
                        mes2send.method = "guess";
                        mes2send.parameters["user_pts"] = 2;
                        mes2send.parameters["host_pts"] = 1;
                    } else {
                        mes2send.method = "msg";
                    }

                    mes2send.parameters["text"] = request.parameters.at("text");
                    mes2send.parameters["user_login"] = request.parameters.at("user_login");
                    mes2send.parameters["team_id"] = request.parameters.at("team_id");

                    std::vector<boost::asio::ip::tcp::socket *> sockets;
                    std::string buffer;
                    if(mes2send.method == "guess"){
                        for(int i=1; i<=Server->Games.at(game_id).team_sockets.size(); ++i) {
                            for(int j=0; j<Server->Games.at(game_id).team_sockets[i].size(); ++j) {
                                sockets.push_back(Server->Games.at(game_id).team_sockets[i][j]);
                            }
                        }
                        std::stringstream ss;
                        ss << "guess:";
                        ss << mes2send.parameters["user_login"] << ":" << mes2send.parameters["text"]
                        << mes2send.parameters["user_pts"] <<":"<<mes2send.parameters["host_pts"];
                        buffer = ss.str();
                    } else {
                        sockets = Server->Games.at(game_id).team_sockets[std::stoi(mes2send.parameters["team_id"])];
                        std::stringstream ss;
                        ss << "msg:";
                        ss << mes2send.parameters["user_login"] << ":" << mes2send.parameters["text"];
                        buffer = ss.str();
                    }

                    for (auto sock: sockets) {
                        boost::asio::async_write(*sock,
                                                 boost::asio::buffer(buffer.data(), buffer.size()),
                                                 boost::bind(&Connection::handle_multiwrite,
                                                             shared_from_this(),
                                                             boost::asio::placeholders::error));
                    }
                    handle_write(e);
                }
//                ----------------

//                std::string buffer = requestRouter_.processRoute(request.method, request);
//                boost::asio::async_write(socket_, boost::asio::buffer(buffer.data(), buffer.size()),
//                                         boost::bind(&Connection::handle_write, shared_from_this(),
//                                                     boost::asio::placeholders::error));
            }
        }

        void Connection::handle_multiwrite(const boost::system::error_code &e) {
            if (!e) {
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