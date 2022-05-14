#include "DBConnection.h"

//установление соединения, создание БД если нет
DBConnection::DBConnection(std::string host, std::string username, std::string pwd, std::string schema) : session(mysqlx::Session(host/*, PORT*/, username, pwd)),
                               schema(session.createSchema(schema, true)) {
    session.sql("USE test;").execute();
//    session.sql("CREATE TABLE IF NOT EXISTS user (id INT PRIMARY KEY AUTO_INCREMENT, login VARCHAR(20) UNIQUE, username VARCHAR(30), password VARCHAR(30));").execute();
//    session.sql(
//            "CREATE TABLE IF NOT EXISTS task (id INT PRIMARY KEY AUTO_INCREMENT, head VARCHAR(100) NOT NULL, body VARCHAR(2500), completion BOOL DEFAULT 0, assigner_id INT NOT NULL, FOREIGN KEY (assigner_id) REFERENCES user(id), executor_id INT NOT NULL, FOREIGN KEY (executor_id) REFERENCES user(id));").execute();
//    session.sql(
//            "CREATE TABLE IF NOT EXISTS message (id INT PRIMARY KEY AUTO_INCREMENT, text VARCHAR(100) NOT NULL, task_id INT NOT NULL, FOREIGN KEY (task_id) REFERENCES task(id), from_id INT NOT NULL, FOREIGN KEY (from_id) REFERENCES user(id));").execute();

    session.sql(
            "CREATE TABLE IF NOT EXISTS users (id INT PRIMARY KEY AUTO_INCREMENT, login VARCHAR(40) NOT NULL UNIQUE, username VARCHAR(50), password VARCHAR(50), num_of_wins INT DEFAULT 0, num_of_losses INT DEFAULT 0, points INT DEFAULT 0);").execute();
    session.sql(
            "CREATE TABLE IF NOT EXISTS words (word VARCHAR(30) NOT NULL PRIMARY KEY, stem VARCHAR(30) NOT NULL, level INT NOT NULL, CONSTRAINT level_constraint CHECK (level IN (1, 2, 3)));").execute();
}

DBConnection::~DBConnection() {
    session.close();
}

bool DBConnection::IsHealthy() {
    mysqlx::RowResult res = session.sql("SHOW TABLES;").execute();
    std::string first_table = res.fetchOne()[0].get<std::string>();
    std::string second_table = res.fetchOne()[0].get<std::string>();
    if (first_table == "users" && second_table == "words") {
        return true;
    } else {
        return false;
    }
}

ConnectionProxy::ConnectionProxy(ConnectionPool *pool, DBConnection *connection) noexcept
        : m_pool{pool}, m_connection{connection} {}

ConnectionProxy::ConnectionProxy(ConnectionProxy &&other) noexcept
        : m_pool{other.m_pool}, m_connection{other.m_connection} {
    other.m_pool = nullptr;
    other.m_connection = nullptr;
}

ConnectionProxy &ConnectionProxy::operator=(ConnectionProxy &&other) noexcept {
    if (this == &other) {
        // prevent self-assignment
        return *this;
    }

    m_pool = other.m_pool;
    m_connection = other.m_connection;
    other.m_pool = nullptr;
    other.m_connection = nullptr;
    return *this;
}

ConnectionProxy::~ConnectionProxy() {
    if (m_pool != nullptr) {
        m_pool->release_connection(m_connection);
    }
}

DBConnection *ConnectionProxy::operator->() { return m_connection; }

DBConnection &ConnectionProxy::operator*() { return *m_connection; }

bool ConnectionProxy::valid() const {
    if ((m_pool != nullptr) && (m_connection != nullptr)) {
        std::unique_lock<std::mutex> lock{m_pool->m_connections_mtx};
        return m_pool->m_connections_busy.count(m_connection) > 0;
    }

    return false;
}

namespace {

    bool check_connect(DBConnection &connection) {
        if (!connection.IsHealthy()) {
            return false;
        }

        return true;
    }

}    // namespace


void ConnectionPool::FillPool(std::vector<std::unique_ptr<DBConnection> > &&connections) {
    for (auto &connection: connections) {
        DBConnection *key = connection.get();
        m_connections_idle.emplace(key, std::move(connection));
    }
}

ConnectionPool::~ConnectionPool() = default;

ConnectionProxy ConnectionPool::get_connection() {
    std::unique_lock lock{m_connections_mtx};

    if (m_connections_idle.empty()) {
        return ConnectionProxy{nullptr, nullptr};
    }

    for (auto &item: m_connections_idle) {
        if (!check_connect(*item.second)) {
            continue;
        }

        ConnectionProxy proxy{this, item.first};
        auto node = m_connections_idle.extract(item.first);
        m_connections_busy.insert(std::move(node));
        return proxy;
    }

    return ConnectionProxy{nullptr, nullptr};
}

void ConnectionPool::release_connection(ConnectionProxy &&proxy) {
    release_connection(proxy.m_connection);
}

void ConnectionPool::release_connection(DBConnection *connection) {
    if (connection == nullptr) {
        return;
    }

    std::unique_lock lock{m_connections_mtx};
    if (auto it = m_connections_busy.find(connection); it != m_connections_busy.end()) {
        check_connect(*it->second);
        auto node = m_connections_busy.extract(it);
        m_connections_idle.insert(std::move(node));
    }
}

std::size_t ConnectionPool::size() const {
    std::unique_lock lock{m_connections_mtx};
    return m_connections_busy.size() + m_connections_idle.size();
}

std::size_t ConnectionPool::size_idle() const {
    std::unique_lock lock{m_connections_mtx};
    return m_connections_idle.size();
}

std::size_t ConnectionPool::size_busy() const {
    std::unique_lock lock{m_connections_mtx};
    return m_connections_busy.size();
}

//void ConnectionPool::heart_beat() {
//    std::unique_lock lock{m_connections_mtx};
//
//    // Only send heart beat on idle connections,
//    // busy connections should be busy for a reason.
//    for ( auto& connection : m_connections_idle ) {
//        connection.second->heart_beat();
//    }
//}

