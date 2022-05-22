# Server API:

(в config.txt запиши путь к mysql-cpp-connector)

: - разделитель

## Для поиска игры:

### на сервер:

settings:user_login:level:num_players_in_team:num_teams

### от сервера, всем:

settings:game_id:team_id:host_login:player1_login:player2_login:player3_login: ...

(для одиночной игры - все игроки; для командной - игроки в команде)

#### затем ведущему:

keyword:new_keyword

## Для сообщения

### на сервер:

msg:user_login:game_id:team_id:text

### от сервера, всем:

msg:user_login:text

### если отгадали, всем:

guess:user_login:team_id:text:user_pts:host_pts

#### затем ведущему:

keyword:new_keyword

## При завершении игры/раунда

### когда раунд закончился, на сервер:

round:game_id

### от сервера, новый раунд, всем:

round:host_login

#### затем ведущему:

keyword:new_keyword

### от сервера, конец игры:

game_over
