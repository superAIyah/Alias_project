#include "connection.h"

#include <vector>
#include <boost/bind/bind.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>

#include <string>
#include <sstream>
#include <regex>

#define WORD_PACK_SIZE 10

Request parse(std::string req_data) {
	Request request;
	std::vector<std::string> request_arr;

//	for testing
	boost::split(request_arr, req_data, [](char c) { return c == ':'; });

//  for deploy
//	std::regex rx("[^\\s]+\r\n");
//	std::sregex_iterator re_it(req_data.begin(), req_data.end(), rx), rxend;
//
//	while(re_it != rxend)
//	{
//		request_arr.push_back(re_it->str());
//		++re_it;
//	}


	request.method = request_arr[0];

	if (request.method == "settings") {
		request.parameters["user_login"] = request_arr[1];
		request.parameters["level"] = request_arr[2];
		request.parameters["num_players"] = request_arr[3];
		request.parameters["num_teams"] = request_arr[4];
		request.parameters["round_duration"] = request_arr[5];
	}
	if (request.method == "msg") {
		request.parameters["user_login"] = request_arr[1];
		request.parameters["game_id"] = request_arr[2];
		request.parameters["team_id"] = request_arr[3];
		request.parameters["text"] = request_arr[4];
		request.parameters["who"] = request_arr[5];//ведущий или отгадывающий
	}
	if (request.method == "round") {
		request.parameters["game_id"] = request_arr[1];
	}
	if (request.method == "auth") {
		request.parameters["user_login"] = request_arr[1];
	}


	return request;
}

std::string SerializeSettings(Request request_) {
	std::stringstream ss;
	ss << "settings\r\n" << request_.parameters.at("level") << "\r\n" << request_.parameters.at("num_players")
	   << "\r\n" << request_.parameters.at("num_teams") << "\r\n" << request_.parameters["round_duration"] << "\r\n";
	return ss.str();
}


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
	Server->Games[game_id_].team_words[team_id_].pop();
	std::stringstream ss;
	ss << "keyword\r\n";
	ss << Server->Games[game_id_].team_words[team_id_].front().word;
	ss << "\r\n";
	std::string buffer = ss.str();

	boost::asio::async_write(*(Server->Games[game_id_].hosts[team_id_].first),
	                         boost::asio::buffer(buffer.data(), buffer.size()),
	                         boost::bind(&Connection::handle_multiwrite,
	                                     shared_from_this(),
	                                     boost::asio::placeholders::error));
}

void Connection::handle_read(const boost::system::error_code &e, std::size_t bytes_transferred) {
	boost::asio::io_service ioService;
//	ioService.run(e);

	if (!e) {
//                вывод в консоль
		std::cout << buffer_.data() << std::endl;

//                парсинг
		Request request = parse(std::string(buffer_.data()));

//              очистка буфера
		buffer_.fill('\0');


		if (request.method == "auth") {
			std::string user_login = request.parameters["user_login"];
//			если пользователь есть в БД
			if (Server->UDBM->has_user(user_login)) {
//				если пользователь уже онлайн
				if (Server->players_online.contains(user_login)) {
					std::stringstream ss;
					ss << "auth\r\n";
					ss << "already_online\r\n";
					std::string buffer = ss.str();
					boost::asio::async_write(socket_, boost::asio::buffer(buffer.data(), buffer.size()),
					                         boost::bind(&Connection::handle_write, shared_from_this(),
					                                     boost::asio::placeholders::error));
				}
//				если пользователь не онлайн
				else {
//					запоминаем логин
					user_login_ = user_login;
//				    добавляем в онлайн
					Server->players_online[user_login] = {-1, -1};

					std::stringstream ss;
					ss << "auth\r\n";
					ss << "ok\r\n";
					std::string buffer = ss.str();
					boost::asio::async_write(socket_, boost::asio::buffer(buffer.data(), buffer.size()),
					                         boost::bind(&Connection::handle_write, shared_from_this(),
					                                     boost::asio::placeholders::error));
				}
			}
//			если пользователя нет в БД
			else {
//				запоминаем логин
				user_login_ = user_login;
//				добавляем в БД
				Server->UDBM->add_user(User(user_login));
//				добавляем в онлайн
				Server->players_online[user_login] = {-1, -1};

				std::stringstream ss;
				ss << "auth\r\n";
				ss << "ok\r\n";
				std::string buffer = ss.str();
				boost::asio::async_write(socket_, boost::asio::buffer(buffer.data(), buffer.size()),
				                         boost::bind(&Connection::handle_write, shared_from_this(),
				                                     boost::asio::placeholders::error));
			}
		}
		else if (request.method == "settings") {

//                    слепляет в строку ключ
			std::string settings_str = SerializeSettings(request);

//                  сокеты и логины
			std::vector<std::pair<boost::asio::ip::tcp::socket *, std::string>> room;

			int num_teams = std::stoi(request.parameters.at("num_teams"));
			int num_players = std::stoi(request.parameters.at("num_players"));

			//количество игроков нужное для начала игры
			int num_players_needed = num_players * num_teams;
//                    добавляет в линию ожидания
			Server->WaitingLine[settings_str].players.emplace_back(&socket_, request.parameters.at("user_login"));


//                    если этот клиент пришел последним, и теперь в комнате достаточно человек
			if (Server->WaitingLine.at(settings_str).players.size() == num_players_needed) {

//                        заполняем комнату
				for (int i = 0; i < num_players_needed; ++i) {
					if (Server->players_online.contains(Server->WaitingLine[settings_str].players[i].second)) {
						room.push_back(Server->WaitingLine[settings_str].players[i]);
					}
					else {
						Server->WaitingLine[settings_str].players.erase(Server->WaitingLine[settings_str].players.begin() + i);
						return handle_write(e);
					}
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
				}
				else { new_game_id = 0; }

				//добавляет игроков в новую игру
				for (int i = 0; i < num_players_needed; i++) {
					int team_id = i % num_teams + 1;
					Server->Games[new_game_id].team_sockets[team_id].push_back(room[i]);
//					запись id игры и команды в players_online
					Server->players_online[room[i].second] = {new_game_id, team_id};

				}


//						добавление слов
				for (int i = 1; i <= num_teams; ++i) {
					//добавляет слова
					std::vector<Word> new_words = Server->WDBM->get_words(std::stoi(request.parameters.at("level")),
					                                                      WORD_PACK_SIZE);
					for (const auto &elem: new_words)
						Server->Games[new_game_id].team_words[i].push(elem);

					//текущее слово которое отгадывает команда
					Server->Games[new_game_id].cur_words[i] = Server->Games[new_game_id].team_words[i].front();
				}

//                        количество игроков в комнате
				Server->Games[new_game_id].num_players = num_players_needed;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                        количество раундов равно количеству игроков в команде, чтобы все игроки побывали ведущими
				Server->Games[new_game_id].rounds_remaining = num_players - 1;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

//                      задали ведущих
				for (int i = 1; i <= num_teams; ++i) {
					Server->Games[new_game_id].hosts[i] = Server->Games[new_game_id].team_sockets[i][0];
				}

//						отправка сообщений
//                      для всех игроков в комнате
				for (int i = 0; i != room.size(); i++) {
//                            команда игрока
					int line_place = i;
					int team_id = line_place % num_teams + 1;

//                            сообщение-ответ
					std::stringstream ss;
					ss << "settings\r\n";
					ss << new_game_id << "\r\n" << team_id << "\r\n";
					if (num_teams == 1) {
						for (int j = 0; j < num_players_needed; ++j)
							ss << room[j].second << "\r\n";
					}
					else {
						for (int j = 0; j < num_players; ++j) {
							ss << Server->Games[new_game_id].team_sockets[team_id][j].second << "\r\n";
						}
					}
//                            ss << "\n";//////////////////////////////////////////////////////////////////
					std::string buffer = ss.str();

//                          написали ответ данному клиенту
					boost::asio::async_write(*(room[i].first),
					                         boost::asio::buffer(buffer.data(), buffer.size()),
					                         boost::bind(&Connection::handle_write, shared_from_this(),
					                                     boost::asio::placeholders::error));
				}

//                      убирает игроков из линии ожидания
				for (int i = 0; i < num_players_needed; ++i) {
					Server->WaitingLine.at(settings_str).players.erase(
							Server->WaitingLine.at(settings_str).players.begin());
				}

//                      отправка нового слова ведущему
				for (int i = 1; i <= Server->Games[new_game_id].team_sockets.size(); ++i) {
					send_kw_2_host(new_game_id, i);
				}

//						длительность раунда в сек
				int round_duration = std::stoi(request.parameters["round_duration"]);
				Server->Games[new_game_id].round_duration = round_duration;


//                        обнуление счетчика для синхронизации раунда
				Server->Games[new_game_id].clients_responded = 0;
//                        обновление таймера
				Server->Games[new_game_id].round_end = time(nullptr) + round_duration;

			}
			else
				handle_write(e);

		}
		else if (request.method == "msg") {

			int game_id = std::stoi(request.parameters.at("game_id"));
			int team_id = std::stoi(request.parameters.at("team_id"));
			std::string text = request.parameters.at("text");
			std::string login = request.parameters.at("user_login");
			std::string word = Server->Games[game_id].team_words[team_id].front().word;
			std::string stem = Server->Games[game_id].team_words[team_id].front().stem;

			//если время раунда еще не закончилось, отправляем сообщение
			if (time(nullptr) < Server->Games[game_id].round_end) {
//                    если сообщение от ведущего
				if (request.parameters.at("who") == "host") {
					std::cout << "^^^HOST^^^\n";

//                        есть ли слово в сообщении
					bool has_stem = false;
					has_stem = text.find(stem) != std::string::npos;
//                        если есть
					if (has_stem) {
//                            std::cout << "VLADOS AHUEL\n";
						std::vector<std::pair<boost::asio::ip::tcp::socket *, std::string>> players = Server->Games[game_id].team_sockets[team_id];

/*
                            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                            //boost::asio::ip::tcp find_host(std::string name, std::vector<std::pair<boost::asio::ip::tcp::socket*, std::string>> players)
//                            for (size_t i = 0; i < players.size(); i++)
//                                if (players[i].second == login)
//                                    socket_host = players[i].first;
                            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
*/

						std::string buffer = "warning\r\nNo spoilers!!!\r\n";
						boost::asio::async_write(socket_,
						                         boost::asio::buffer(buffer.data(), buffer.size()),
						                         boost::bind(&Connection::handle_write, shared_from_this(),
						                                     boost::asio::placeholders::error));
					}
//                        если слова нет
					else {
						//msg:user_login:text
						std::vector<std::pair<boost::asio::ip::tcp::socket *, std::string>> players = Server->Games[game_id].team_sockets[team_id];
//                            сообщение-ответ
						std::stringstream ss;
						ss << "msg\r\n";
						ss << login << "\r\n" << text << "\r\n";
//                            ss << "\n";////////////////////////////////////////////////
						std::string buffer = ss.str();
//                            отправка ответа всем клиентам в команде
						for (size_t i = 0; i < players.size(); i++) {
							boost::asio::async_write(*(players[i].first),
							                         boost::asio::buffer(buffer.data(), buffer.size()),
							                         boost::bind(&Connection::handle_multiwrite, shared_from_this(),
							                                     boost::asio::placeholders::error));
						}
						handle_write(e);
					}
				}
//                    если игрок - не ведущий
				else {
//                        есть ли слово в сообщении
					bool has_word = false;
					has_word = text.find(word) != std::string::npos;
//                        если есть
					if (has_word) {

//                            если отгадали, всем:
//                            guess:user_login:team_id:text:user_pts:host_pts
//
//                            затем ведущему:
//                            keyword:new_keyword

//                          сообщение-ответ
						std::stringstream ss;
						ss << "guess\r\n";
						ss << login << "\r\n" << team_id << "\r\n" << text << "\r\n" << USER_GUESS_POINTS << "\r\n"
						   << HOST_GUESS_POINTS;
//                            ss << "\n";////////////////////////////////////////////////
						std::string buffer = ss.str();

						std::map<int, std::vector<std::pair<boost::asio::ip::tcp::socket *, std::string>>> players = Server->Games[game_id].team_sockets;
//                            отправка всем игрокам в комнате
						for (int i = 1; i <= players.size(); i++) {
							for (int j = 0; j < players[i].size(); j++) {
								boost::asio::async_write(*(players[i][j].first),
								                         boost::asio::buffer(buffer.data(), buffer.size()),
								                         boost::bind(&Connection::handle_multiwrite, shared_from_this(),
								                                     boost::asio::placeholders::error));
							}
						}

//                        отправка нового слова ведущему
						send_kw_2_host(game_id, team_id);

					}
					else {
						//msg:user_login:text
						std::vector<std::pair<boost::asio::ip::tcp::socket *, std::string>> players = Server->Games[game_id].team_sockets[team_id];
//                            сообщение-ответ
						std::stringstream ss;
						ss << "msg\r\n";
						ss << login << "\r\n" << text << "\r\n";
//                            ss << "\n";
						std::string buffer = ss.str();
//                            отправка всем игрокам в команде
						for (size_t i = 0; i < players.size(); i++) {
							boost::asio::async_write(*(players[i].first),
							                         boost::asio::buffer(buffer.data(), buffer.size()),
							                         boost::bind(&Connection::handle_multiwrite, shared_from_this(),
							                                     boost::asio::placeholders::error));
						}
					}
					handle_write(e);
				}
			}
//                    если время раунда закончилось, ничего не отправляем
			else { handle_write(e); }
		}
		else if (request.method == "round") {
			int game_id = std::stoi(request.parameters["game_id"]);
//                    заполнения счетчика клиентов, отчитавшихся о завершении раунда
			Server->Games[game_id].clients_responded += 1;

//                    если данный клиент был последним, кто отчитался, и теперь отчитались все
			if (Server->Games[game_id].clients_responded == Server->Games[game_id].num_players) {

//                        если раунды еще остались, игра не закончена
				if (Server->Games[game_id].rounds_remaining != 0) {

					Server->Games[game_id].rounds_remaining -= 1;

//                      обновили ведущих
					for (int i = 1; i <= Server->Games[game_id].team_sockets.size(); ++i) {
						for (int j = 0; j < Server->Games[game_id].team_sockets[i].size(); ++j) {
							if (Server->Games[game_id].team_sockets[i][j].second == Server->Games[game_id].hosts[i].second) {
								if (j == Server->Games[game_id].team_sockets[i].size() - 1) {
									Server->Games[game_id].hosts[i] = Server->Games[game_id].team_sockets[i][0];
								}
								else {
									Server->Games[game_id].hosts[i] = Server->Games[game_id].team_sockets[i][j + 1];
								}
								break;
							}
						}

					}

//                        для всех команд
					for (int i = 1; i <= Server->Games[game_id].team_sockets.size(); ++i) {
//                        сокеты всех игроков в команде
						std::vector<boost::asio::ip::tcp::socket *> sockets2send;
						for (int j = 0; j < Server->Games[game_id].team_sockets[i].size(); ++j) {
							sockets2send.push_back(Server->Games[game_id].team_sockets[i][j].first);
						}

//                            сообщение-ответ
						std::stringstream ss;
						ss << "round\r\n";
						ss << Server->Games[game_id].hosts[i].second;
						ss << "\r\n";
						std::string buffer = ss.str();

//                          отправка ответа
						for (auto sock: sockets2send) {
							boost::asio::async_write(*sock,
							                         boost::asio::buffer(buffer.data(), buffer.size()),
							                         boost::bind(&Connection::handle_multiwrite, shared_from_this(),
							                                     boost::asio::placeholders::error));
						}
					}

//                        отправка нового слова ведущему
					for (int i = 1; i <= Server->Games[game_id].team_sockets.size(); ++i) {
						send_kw_2_host(game_id, i);
					}

//							длительность раунда в сек
					int round_duration = Server->Games[game_id].round_duration;

//                        обновление таймера
					Server->Games[game_id].round_end = time(nullptr) + round_duration;

//                        обнуление счетчика для синхронизации раунда
					Server->Games[game_id].clients_responded = 0;
				}
//                      игра закончена
				else {
//                        сокеты всех игроков в игре
					std::vector<boost::asio::ip::tcp::socket *> sockets2send;
					for (int i = 1; i <= Server->Games[game_id].team_sockets.size(); ++i) {
						for (int j = 0; j < Server->Games[game_id].team_sockets[i].size(); ++j) {
							sockets2send.push_back(Server->Games[game_id].team_sockets[i][j].first);
//							обнуляем game_id & team_id для игрока
							Server->players_online[Server->Games[game_id].team_sockets[i][j].second] = {-1, -1};
						}
					}

//                          сообщение-ответ
					std::stringstream ss;
					ss << "gameover\r\n";
					std::string buffer = ss.str();

//                          отправка ответа
					for (auto sock: sockets2send) {
						boost::asio::async_write(*sock,
						                         boost::asio::buffer(buffer.data(), buffer.size()),
						                         boost::bind(&Connection::handle_multiwrite, shared_from_this(),
						                                     boost::asio::placeholders::error));
					}

//							удаляет комнату
					Server->Games.erase(game_id);

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
//	если пользователь вышел из приложения
	else if (e.message() == "End of file") {
		std::cout << "Reading  error!\n" << user_login_ << " closed app!\n";
		if (!user_login_.empty()) {
			int game_id = Server->players_online[user_login_].first;
//		если пользователь был в игре
			if (game_id != -1) {
				int team_id = Server->players_online[user_login_].second;
				int num_players_in_team = Server->Games[game_id].team_sockets[team_id].size();
//			удаляем его из игры
				for (int i = 0; i < num_players_in_team; ++i) {
					if (Server->Games[game_id].team_sockets[team_id][i].second == user_login_) {
						Server->Games[game_id].team_sockets[team_id].erase(Server->Games[game_id].team_sockets[team_id].begin() + i);
						break;
					}
				}
//			уменьшаем количество игроков в игре
				Server->Games[game_id].num_players -= 1;
			}
			Server->players_online.erase(user_login_);
		}
	}
}

void Connection::handle_multiwrite(const boost::system::error_code &e) {
	if (!e) {
		return;
	}
	else {
		std::cout << "Multiple Writing  error!\n" << user_login_ << " closed app!\n";

		if (!user_login_.empty()) {
			int game_id = Server->players_online[user_login_].first;
//		если пользователь был в игре
			if (game_id != -1) {
				int team_id = Server->players_online[user_login_].second;
				int num_players_in_team = Server->Games[game_id].team_sockets[team_id].size();
//			удаляем его из игры
				for (int i = 0; i < num_players_in_team; ++i) {
					if (Server->Games[game_id].team_sockets[team_id][i].second == user_login_) {
						Server->Games[game_id].team_sockets[team_id].erase(Server->Games[game_id].team_sockets[team_id].begin() + i);
						break;
					}
				}
//			уменьшаем количество игроков в игре
				Server->Games[game_id].num_players -= 1;
			}
			Server->players_online.erase(user_login_);
		}
	}
}

void Connection::handle_write(const boost::system::error_code &e) {
	if (!e) {
		try {
			socket_.async_read_some(boost::asio::buffer(buffer_),
			                        boost::bind(&Connection::handle_read, shared_from_this(), boost::asio::placeholders::error,
			                                    boost::asio::placeholders::bytes_transferred));
		} catch (...) {
			std::cout << "error caught!";
		}
	}
	else {
		std::cout << "Writing  error!\n" << user_login_ << " closed app!\n";

		if (!user_login_.empty()) {
			int game_id = Server->players_online[user_login_].first;
//		если пользователь был в игре
			if (game_id != -1) {
				int team_id = Server->players_online[user_login_].second;
				int num_players_in_team = Server->Games[game_id].team_sockets[team_id].size();
//			удаляем его из игры
				for (int i = 0; i < num_players_in_team; ++i) {
					if (Server->Games[game_id].team_sockets[team_id][i].second == user_login_) {
						Server->Games[game_id].team_sockets[team_id].erase(Server->Games[game_id].team_sockets[team_id].begin() + i);
						break;
					}
				}
//			уменьшаем количество игроков в игре
				Server->Games[game_id].num_players -= 1;
			}
			Server->players_online.erase(user_login_);
		}
	}
}

