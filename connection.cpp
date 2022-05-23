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
    if (request.method == "round") {
        request.parameters["game_id"] = request_arr[1];
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

        void Connection::send_kw_2_host(int game_id_, int team_id_) {
            std::stringstream ss;
            ss << "keyword:";
            ss << Server->Games[game_id_].team_words[team_id_].front();
            std::string buffer = ss.str();
            Server->Games[game_id_].team_words[team_id_].pop();

            boost::asio::async_write(*(Server->Games[game_id_].hosts[team_id_].first),
                                     boost::asio::buffer(buffer.data(), buffer.size()),
                                     boost::bind(&Connection::handle_multiwrite,
                                                 shared_from_this(),
                                                 boost::asio::placeholders::error));
        }

        void Connection::handle_read(const boost::system::error_code &e, std::size_t bytes_transferred) {
            if (!e) {
                std::cout << buffer_.data() << std::endl;
                Request request = parse(std::string(buffer_.data()));
//              очистка буфера
                buffer_.fill('\0');

                //////////////////////////
                // struct Request
                // {
                //     std::string method;
                //     std::unordered_map<std::string, std::string> parameters;
                // };

//                for (auto i = request.parameters.begin(); i != request.parameters.end(); i++) {
//                    std::cout << i->first << "\t" << i->second << "\n";
//                }

                /////////////////////////
//                ---------------
                if (request.method == "settings") {
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
                    if (Server->WaitingLine.at(settings_str).players.size() == num_players_needed) {

//                        заполняем комнату
                        for (int i = 0; i < num_players_needed; ++i) {
                            room.push_back(Server->WaitingLine[settings_str].players[i]);
                        }

/*//////////////////////////////////////////////////////////////////////////////////////
//                        std::cout << "GAME CREATE = ";
//                        for (int i = 0; i < num_players_needed; i++)
//                            std::cout << room[i].second << "\t";
//                        std::cout << "\n\n";
///////////////////////////////////////////////////////////////////////////////////////*/

//                      id игры
                        int new_game_id;
                        if (Server->Games.size() != 0) {
//                            последний+1
                            new_game_id = Server->Games.rbegin()->first + 1;
                        } else { new_game_id = 0; }

                        //добавляет игроков в новую игру
                        for (int i = 0; i < num_players_needed; i++) {
                            int team_id = i % num_teams + 1;
                            Server->Games[new_game_id].team_sockets[team_id].push_back(room[i]);

                            //добавляет слова
                            std::vector<std::string> new_words = Server->WDBM->get_words_str(
                                    std::stoi(request.parameters.at("level")), WORD_PACK_SIZE);
                            for (const auto &elem: new_words)
                                Server->Games[new_game_id].team_words[team_id].push(elem);

                            //текущее слово которое отгадывает команда
                            Server->Games[new_game_id].cur_words[team_id] = Server->Games[new_game_id].team_words[team_id].front();
                        }
//                        количество игроков в комнате
                        Server->Games[new_game_id].num_players = num_players_needed;

//                      задали ведущих
                        for (int i = 1; i <= num_teams; ++i) {
                            Server->Games[new_game_id].hosts[i] = Server->Games[new_game_id].team_sockets[i][0];
                        }


                        /*НЕ УДАЛЯТЬ!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                         std::cout << "NEW GAME///////////////////////////////\n";
                         for (int i = 1; i <= num_teams; i++)
                         {
                             std::cout << "team_id=" << i << "\n";
                             std::cout << "players:";
                             for (auto k = Server->Games[new_game_id].team_sockets[i].begin(); k != Server->Games[new_game_id].team_sockets[i].end(); k++)
                                 std::cout << k->second << "\t";
                             std::cout << "\n";

                             std::cout << "words:";
                             for (auto k = Server->Games[new_game_id].team_words[i].begin(); k != Server->Games[new_game_id].team_words[i].end(); k++)
                                 std::cout << *k;
                             std::cout << "\n\n\n";
                         }*/

//                      для всех игроков в комнате
                        for (int i = 0; i != room.size(); i++) {
//                            команда игрока
                            int line_place = i;
                            int team_id = line_place % num_teams + 1;

//                            сообщение-ответ
                            std::stringstream ss;
                            ss << "settings:";
                            ss << new_game_id << ":" << team_id << ":";
                            for (int j = 0; j < num_players_needed; ++j) {
                                ss << room[j].second << ":";
                            }
                            std::string buffer = ss.str();

//                          написали ответ данному клиенту
                            boost::asio::async_write(*(room[i].first),
                                                     boost::asio::buffer(buffer.data(), buffer.size()),
                                                     boost::bind(&Connection::handle_write,
                                                                 shared_from_this(),
                                                                 boost::asio::placeholders::error));
                        }

                        //убирает игроков из линии ожидания
                        for (int i = 0; i < num_players_needed; ++i) {
                            Server->WaitingLine.at(settings_str).players.erase(
                                    Server->WaitingLine.at(settings_str).players.begin());
                        }

//                        отправка нового слова ведущему
                        for (int i = 1; i <= Server->Games[new_game_id].team_sockets.size(); ++i) {
                            send_kw_2_host(new_game_id, i);
                        }

//                        обнуление счетчика для синхронизации раунда
                        Server->Games[new_game_id].clients_responded = 0;

                    } else
                        handle_write(e);

                }
                else if (request.method == "msg") {

                    int game_id = std::stoi(request.parameters.at("game_id"));
                    int team_id = std::stoi(request.parameters.at("team_id"));

//                  проверка на наличие отгадки
                    bool is_word = false;
                    std::vector<std::string> words_in_msg;
                    boost::split(words_in_msg, request.parameters.at("text"),
                                 [](char c) { return (c == ' '); });
                    for (int i = 0; i < words_in_msg.size(); ++i) {
                        if (words_in_msg[i] ==
                            Server->Games.at(game_id).cur_words[team_id]) { is_word = true; }
                    }

                    request.parameters["is_word"] = (is_word ? "true" : "false");

//                    std::string buffer = requestRouter_.processRoute(request.method, request);
                    Response mes2send;
                    if (request.parameters.at("is_word") == "true") {
                        mes2send.method = "guess";
                        mes2send.parameters["user_pts"] = USER_GUESS_POINTS;
                        mes2send.parameters["host_pts"] = HOST_GUESS_POINTS;
                    } else {
                        mes2send.method = "msg";
                    }

                    mes2send.parameters["text"] = request.parameters.at("text");
                    mes2send.parameters["user_login"] = request.parameters.at("user_login");
                    mes2send.parameters["team_id"] = request.parameters.at("team_id");

                    std::vector<boost::asio::ip::tcp::socket *> sockets2send;
                    std::string buffer;
                    if (mes2send.method == "guess") {
                        for (int i = 1; i <= Server->Games.at(game_id).team_sockets.size(); ++i) {
                            for (int j = 0; j < Server->Games.at(game_id).team_sockets[i].size(); ++j) {
                                sockets2send.push_back(Server->Games.at(game_id).team_sockets[i][j].first);
                            }
                        }
                        std::stringstream ss;
                        ss << "guess:";
                        ss << mes2send.parameters["user_login"] << ":" << mes2send.parameters["text"] << ":"
                           << mes2send.parameters["user_pts"] << ":" << mes2send.parameters["host_pts"];
                        buffer = ss.str();
                    } else {
                        for (int j = 0; j < Server->Games.at(game_id).team_sockets[std::stoi(
                                mes2send.parameters["team_id"])].size(); ++j) {
                            sockets2send.push_back(Server->Games.at(game_id).team_sockets[std::stoi(
                                    mes2send.parameters["team_id"])][j].first);
                        }

                        std::stringstream ss;
                        ss << "msg:";
                        ss << mes2send.parameters["user_login"] << ":" << mes2send.parameters["text"];
                        buffer = ss.str();
                    }

                    for (auto sock: sockets2send) {
                        boost::asio::async_write(*sock,
                                                 boost::asio::buffer(buffer.data(), buffer.size()),
                                                 boost::bind(&Connection::handle_multiwrite,
                                                             shared_from_this(),
                                                             boost::asio::placeholders::error));
                    }
                    handle_write(e);
                }
                else if (request.method == "round") {
                    int game_id = std::stoi(request.parameters["game_id"]);
//                    заполнения счетчика клиентов, отчитавшихся о завершении раунда
                    Server->Games[game_id].clients_responded += 1;
//                    если данный клиент был последним, кто отчитался, и теперь отчитались все
                    if (Server->Games[game_id].clients_responded == Server->Games[game_id].num_players) {
//                      обновили ведущих
                        for (int i = 1; i <= Server->Games[game_id].team_sockets.size(); ++i) {
                            for (int j = 0; j < Server->Games[game_id].team_sockets[i].size(); ++j) {
                                if (Server->Games[game_id].team_sockets[i][j].second ==
                                    Server->Games[game_id].hosts[i].second) {
                                    Server->Games[game_id].hosts[i] = Server->Games[game_id].team_sockets[i][
                                            j + 1];
                                    break;
                                }
                            }

                        }

                        for (int i = 1; i <= Server->Games[game_id].team_sockets.size(); ++i) {
//                        сокеты всех игроков в игре
                            std::vector<boost::asio::ip::tcp::socket *> sockets2send;
                            for (int j = 0; j < Server->Games[game_id].team_sockets[i].size(); ++j) {
                                sockets2send.push_back(Server->Games[game_id].team_sockets[i][j].first);
                            }

//                            сообщение-ответ
                            std::stringstream ss;
                            ss << "round:";
                            ss << Server->Games[game_id].hosts[i].second;
                            std::string buffer = ss.str();

//                          отправка ответа
                            for (auto sock: sockets2send) {
                                boost::asio::async_write(*sock,
                                                         boost::asio::buffer(buffer.data(), buffer.size()),
                                                         boost::bind(&Connection::handle_multiwrite,
                                                                     shared_from_this(),
                                                                     boost::asio::placeholders::error));
                            }
                        }

//                        отправка нового слова ведущему
                        for (int i = 1; i <= Server->Games[game_id].team_sockets.size(); ++i) {
                            send_kw_2_host(game_id, i);
                        }

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
