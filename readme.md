# Server API:

(в config.txt запиши путь к mysql-cpp-connector)

: - разделитель

## Для поиска игры:

### на сервер:

settings:user_login:level:num_players_in_team:num_teams

### от сервера, ведущему:

settings:game_id:team_id:host_login:key_word:player1_login:player2_login:player3_login: ...

(по host_login клиент определяет, что он ведущий, и считывает key_word)

### от сервера, всем остальным:

settings:game_id:team_id:host_login:player1_login:player2_login:player3_login: ...

(для одиночной игры - все игроки; для командной - игроки в команде)

## Для сообщения

### на сервер:

msg:user_login:game_id:team_id:text

### от сервера, всем:

msg:user_login:text

### если отгадали, ведущему:

guess:user_login:team_id:text:user_pts:host_pts:key_word

### или если отгадали, всем остальным:

guess:user_login:team_id:text:user_pts:host_pts

## При завершении игры/раунда

### когда раунд закончился, на сервер:

round:game_id

### от сервера, новый раунд, ведущему:

round:host_login:key_word

### от сервера, новый раунд, всем остальным:

round:host_login

### от сервера, конец игры:

game_over
