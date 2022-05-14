#pragma once

#include <mysqlx/xdevapi.h>
#include <string>
#include <vector>
#include <mutex>
#include "tables.h"

// переменные подключения
//const std::string HOST = "localhost";
//const int PORT = 33060;
//const std::string USER_NAME = "root";
//const std::string PWD = "1111";
//const std::string SCHEMA_NAME = "test";

//установка соединения
class DBConnection {
public:
//    bool res = true;
    mysqlx::Session session /*= mysqlx::Session(HOST*//*, PORT*//*, USER_NAME, PWD)*/;
    mysqlx::Schema schema /*= session.createSchema("test", true)*/;

    DBConnection(std::string host, std::string username, std::string pwd, std::string schema);
    ~DBConnection();

    bool IsHealthy();
};

// исключение, связанное с ограничениями БД
class DB_except : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class ConnectionProxy;

class IConnectionPool {
    friend ConnectionProxy;
public:

    virtual ConnectionProxy get_connection() = 0;

    virtual void release_connection(ConnectionProxy &&proxy) = 0;

    virtual std::size_t size() const = 0;

    virtual std::size_t size_idle() const = 0;

    virtual std::size_t size_busy() const = 0;

    virtual void FillPool(std::vector<std::unique_ptr<DBConnection> > &&connections) = 0;

private:
    virtual void release_connection(DBConnection *connection) = 0;
};

class ConnectionPool : public IConnectionPool {
    friend ConnectionProxy;
public:

    ConnectionProxy get_connection() override;

    void release_connection(ConnectionProxy &&proxy) override;

    std::size_t size() const override;

    std::size_t size_idle() const override;

    std::size_t size_busy() const override;

//    void heart_beat();

    void FillPool(std::vector<std::unique_ptr<DBConnection> > &&connections) override;

    ~ConnectionPool();

private:
    void release_connection(DBConnection *connection) override;

    mutable std::mutex m_connections_mtx;
    std::unordered_map<DBConnection *, std::unique_ptr<DBConnection> > m_connections_idle;
    std::unordered_map<DBConnection *, std::unique_ptr<DBConnection> > m_connections_busy;
};

class ConnectionProxy final {
    friend ConnectionPool;

public:
    ConnectionProxy(ConnectionProxy &&other) noexcept;

    ConnectionProxy &operator=(ConnectionProxy &&other) noexcept;

    ~ConnectionProxy();

    DBConnection *operator->();

    DBConnection &operator*();

    bool valid() const;

private:
    ConnectionProxy(ConnectionPool *pool, DBConnection *connection) noexcept;

    ConnectionPool *m_pool;
    DBConnection *m_connection;
};


