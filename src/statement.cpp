/*
MIT License

Copyright (c) 2025 Y-HRAH KRONG Santili

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "cppsqlx/statement.hpp"

namespace cppsqlx {

Statement::Statement(std::shared_ptr<PGconn> conn) : conn_(std::move(conn)) {}

std::expected<void, std::string> Statement::executeUpdate(const std::string& sql) {
    auto res = PQexec(conn_.get(), sql.c_str());
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::string err = PQerrorMessage(conn_.get());
        PQclear(res);
        return std::unexpected(err);
    }
    PQclear(res);
    return {};
}

std::expected<Resultset, std::string> Statement::execute(const std::string& sql) {
    auto res = PQexec(conn_.get(), sql.c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::string err = PQerrorMessage(conn_.get());
        PQclear(res);
        return std::unexpected(err);
    }
    return {Resultset(res)};
}


std::expected<void, std::string> Statement::prepareStatement(const std::string& stmtName, const std::string& sql, int paramCount) {
    auto res = PQprepare(conn_.get(), stmtName.c_str(), sql.c_str(), paramCount, nullptr);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::string err = PQerrorMessage(conn_.get());
        PQclear(res);
        return std::unexpected(err);
    }
    PQclear(res);
    return {};
}

std::expected<void, std::string> Statement::executePrepared(const std::string& stmtName, const std::vector<std::string>& params) {
    const char* paramValues[params.size()];
    for (size_t i = 0; i < params.size(); ++i) paramValues[i] = params[i].c_str();
    auto res = PQexecPrepared(conn_.get(), stmtName.c_str(), static_cast<int>(params.size()), paramValues, nullptr, nullptr, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::string err = PQerrorMessage(conn_.get());
        PQclear(res);
        return std::unexpected(err);
    }
    PQclear(res);
    return {};
}

std::expected<void, std::string> Statement::executeBatch(const std::vector<std::string>& sqlStatements) {
    for (const auto& sql : sqlStatements) {
        auto res = PQexec(conn_.get(), sql.c_str());
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            std::string err = PQerrorMessage(conn_.get());
            PQclear(res);
            return std::unexpected(err);
        }
        PQclear(res);
    }
    return {};
}

} // namespace cppsqlx