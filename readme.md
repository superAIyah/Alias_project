# Server API:

(в config.txt запиши путь к mysql-cpp-connector)

: - разделитель

на сервер:

settings:user_login:level:num_players_in_team:num_teams

от сервера:

settings:game_id:team_id:player1:player2:player3: ...

для одиночной игры, все игроки

для командной, игроки в команде

на сервер:

msg:user_login:game_id:team_id:text

от сервера:

msg:user_login:text

или

guess:user_login:text:user_pts:host_pts

когда раунд закончился, на сервер:

round:game_id:team_id

когда игра закончилась, на сервер:

gameover:game_id 
