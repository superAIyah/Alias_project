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
        request.parameters["game_id"] = request_arr[1];
        request.parameters["team_id"] = request_arr[2];
        request.parameters["msg"] = request_arr[3];
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
                  requestRouter_(requestRouter){
        }

        boost::asio::ip::tcp::socket &Connection::socket() {
            return socket_;
        }

        void Connection::start() {
            socket_.async_read_some(boost::asio::buffer(buffer_),
                                    boost::bind(&Connection::handle_read, shared_from_this(),
                                                boost::asio::placeholders::error,
                                                boost::asio::placeholders::bytes_transferred));
        }

        void Connection::handle_read(const boost::system::error_code &e,
                                     std::size_t bytes_transferred) {
            if (!e) {
                std::cout << buffer_.data() << std::endl;
                Request request = parse(std::string(buffer_.data()));

//                ---------------
                if (request.method == "settings") {
//                    слепляет в строку ключ
                    std::string settings_str = SerializeSettings(request);
//                    добавляет в линию ожидания

                    Server->WaitingLine[settings_str].players.push_back(
                            std::make_pair(&socket_, request.parameters.at("user_login")));
                    int line_place = Server->WaitingLine.at(settings_str).players.size()-1;
                    int num_players_needed = std::stoi(request.parameters.at("num_players")) *
                                             std::stoi(request.parameters.at("num_teams"));
//                    сокеты и логины игроков
                    std::vector<std::pair<boost::asio::ip::tcp::socket *, std::string>> players;
//                    ожидает
                    while (Server->WaitingLine.at(settings_str).players.size() < num_players_needed) {
//                        sleep(1);
                    }
//                   team_id found
                    int num_teams = std::stoi(request.parameters.at("num_teams"));
                    int team_id;
                    if (num_teams == 0) {
                        team_id = 0;
                    } else {
                        team_id = line_place % num_teams + 1;
                    }

//                    записывает игроков
                    for (int i = 0; i < num_players_needed; ++i) {
//                        if (Server->WaitingLine.at(settings_str)[i].first == &socket_) {
                        players.push_back(Server->WaitingLine.at(settings_str).players[i]);
//                        }
                    }
                    Server->WaitingLine.at(settings_str).creating_game.lock();
//                    убирает игроков из линии ожидания
                    for (int i = 0; i < num_players_needed; ++i) {
                        if (Server->WaitingLine.at(settings_str).players[i].first == &socket_) {
                            Server->WaitingLine.at(settings_str).players.erase(
                                    Server->WaitingLine.at(settings_str).players.begin() + i);
                            break;
                        }
                    }
                    Server->WaitingLine.at(settings_str).creating_game.unlock();

                    int new_game_id;
                    Server->WaitingLine.at(settings_str).creating_game.lock();
                    bool game_exists = false;
//                    checks if game exists
                    for(int i=0; i<Server->Games.size(); ++i){
                        for(int j = 0; j<Server->Games[i].team_sockets.size(); ++j){
                            for(int k=0; k<Server->Games[i].team_sockets[j+1].size(); ++k) {
                                if (Server->Games[i].team_sockets[j+1][k] == &socket_) {
                                    game_exists = true;
                                    new_game_id = i;
                                    break;
                                }
                            }
                        }
                    }
                    if(!game_exists) {
                        if (Server->Games.size() != 0) {
                            new_game_id = Server->Games.rbegin()->first + 1;
                        } else {
                            new_game_id = 0;
                        }

//                    добавляет игроков в новую игру
                        for (int i = 0; i < num_players_needed; ++i) {
                            Server->Games[new_game_id].team_sockets[i % num_teams + 1].push_back(
                                    players[i].first);
                        }
//                    добавляет слова
                        for (int i = 0; i < num_teams; ++i) {
                            Server->Games[new_game_id].team_words[i] = Server->WDBM->get_words_str(
                                    std::stoi(request.parameters.at("level")), WORD_PACK_SIZE);
                            Server->Games[
                                    new_game_id].cur_words[i] = Server->Games[
                                    new_game_id].team_words[i][0];
                        }
                    }
                    Server->WaitingLine.at(settings_str).creating_game.unlock();

                    std::stringstream ss;
                    ss << "settings:";
                    ss << new_game_id << ":" << team_id << ":";
                    for (int i = 0; i < num_players_needed; ++i) {
                        ss << players[i].second << ":";
                    }
                    std::string buffer = ss.str();

                    boost::asio::async_write(socket_, boost::asio::buffer(buffer.data(), buffer.size()),
                                             boost::bind(&Connection::handle_write, shared_from_this(),
                                                         boost::asio::placeholders::error));

                } else if (request.method == "msg") {
                    int game_id = std::stoi(request.parameters.at("game_id"));
                    int team_id = std::stoi(request.parameters.at("team_id"));
//                    std::string msgstr = SerializeMsg(request);
                    bool is_word = false;
                    std::vector<std::string> words_in_msg;
                    boost::split(words_in_msg, request.parameters.at("msg"), [](char c) { return c == ' '; });
                    for (int i = 0; i < words_in_msg.size(); ++i) {
                        if (words_in_msg[i] ==
                            Server->Games.at(game_id).cur_words[team_id]) { is_word = true; }
                    }

                    request.parameters["is_word"] = (is_word ? "true" : "false");

                    std::string buffer = requestRouter_.processRoute(request.method, request);

                    std::vector<boost::asio::ip::tcp::socket *> sockets = Server->Games.at(
                            game_id).team_sockets[team_id];
                    for (auto sock: sockets) {
                        boost::asio::async_write(*sock, boost::asio::buffer(buffer.data(), buffer.size()),
                                                 boost::bind(&Connection::handle_multiwrite,
                                                             shared_from_this(),
                                                             boost::asio::placeholders::error));
                    }
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
