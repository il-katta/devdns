#pragma once

#include <string>
#include <pqxx/pqxx>
#include <unistd.h>

#include "logger.h"

class Storage {
    std::string _connectionString;
    std::unique_ptr<pqxx::connection> _conn;
public:
    Storage() : Storage(secure_getenv("CONNECTION_STRING")) {}

    Storage(const std::string &connectionString) {
        this->_connectionString = connectionString;
        this->_conn = this->_new_connection();
        this->_prepare();
    }

    Storage(
            const std::string &db_name,
            const std::string &db_host,
            const std::string &db_port,
            const std::string &db_user,
            const std::string &db_password,
            const std::string &db_extra_connection_parameters = ""
    ) : Storage(fmt::format("host={} port={} dbname={} user={} password={} {}", db_host, db_port, db_name, db_user, db_password, db_extra_connection_parameters))  {
        // TODO: url encode parameters
    }

    ~Storage() {
        try {
            if (this->_conn) {
                log_debug("Storage: closing connection {}", this->_conn->sock());
                if (this->_conn->is_open()) {
#if PQXX_VERSION_MAJOR >= 7
                    this->_conn->close();
#else
                    this->_conn->disconnect();
#endif
                }
                //this->_conn.release();
            }
        } catch (std::exception &ex) {
            log_error("~Storage: {}", ex.what());
        }
    }

    static void waitConnection() {
        waitConnection(secure_getenv("CONNECTION_STRING"));
    }

    static void waitConnection(const std::string &connectionString, std::int32_t maxRetry = 100) {
        try {
            pqxx::connection c(connectionString);
            if (c.is_open()) {
#if PQXX_VERSION_MAJOR >= 7
                c.close();
#else
                c.disconnect();
#endif
            } else {
                throw pqxx::broken_connection();
            }
        } catch (const pqxx::broken_connection &e) {
            if (maxRetry == 0) {
                throw;
            }
            log_info("Storage: waiting for db connection ... retry - {} - {}", std::to_string(maxRetry), e.what());
            sleep(1);
            waitConnection(connectionString, maxRetry - 1);
        }
    }


private:

    pqxx::result _db_exec(const std::string &sql) {
        auto *txn = new pqxx::work(*this->_conn);
        try {
            pqxx::result result{txn->exec(sql)};
            txn->commit();
            delete txn;
            return result;
        } catch (const std::exception &) {
            txn->abort();
            delete txn;
            throw;
        }
    }

    template<typename... Args>
    void _log(Args &&...args) {
        std::string tmp = "Storage.{} ( ";
        auto c = sizeof...(Args);
        for (int i = 1; i < c; i++) {
            if (i < c - 1) {
                tmp += "{}, ";
            } else {
                tmp += "{} ";
            }
        }
        tmp += ")";
        log_debug(tmp, args...);
    }

    template<typename... Args>
    pqxx::result _db_exec_prepared(Args &&...args) {
        _log(args...);
        auto *txn = new pqxx::work(*this->_conn);
        try {
            pqxx::result result{txn->exec_prepared(args...)};
            txn->commit();
            delete txn;
            return result;
        } catch (const std::exception &) {
            txn->abort();
            delete txn;
            throw;
        }
    }

    std::unique_ptr<pqxx::connection> _new_connection() {
        Storage::waitConnection(this->_connectionString);
        auto conn = std::make_unique<pqxx::connection>(this->_connectionString);
        return conn;
    }

    void _prepare() {
        this->_conn->prepare(
                "all_records",
                "select * from records"
        );
    }
};
