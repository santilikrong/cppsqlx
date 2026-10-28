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
#include <libpq-fe.h> // 🟢 Crucial : permet à PQexec et PQerrorMessage de compiler correctement
#include <utility>

namespace cppsqlx {

// Constructeur mis à jour pour accepter le pointeur brut
Statement::Statement(PGconn* conn) : conn_(conn) {}

std::expected<void, std::string> Statement::executeUpdate(const std::string& sql) {
    if (!conn_) return std::unexpected("Statement error: No active database connection.");

    // Remplacement de conn_.get() par conn_ directement
    PGresult* res = PQexec(conn_, sql.c_str()); 
    ExecStatusType status = PQresultStatus(res);

    if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK) {
        std::string err = PQerrorMessage(conn_);
        PQclear(res);
        return std::unexpected(err);
    }

    PQclear(res);
    return {};
}

std::expected<Resultset, std::string> Statement::execute(const std::string& sql) {
    if (!conn_) return std::unexpected("Statement error: No active database connection.");

    PGresult* res = PQexec(conn_, sql.c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::string err = PQerrorMessage(conn_);
        PQclear(res);
        return std::unexpected(err);
    }

    // Instanciation de votre Resultset (à adapter selon le constructeur de votre Resultset)
    return Resultset(res); 
}

std::expected<void, std::string> Statement::prepareStatement(const std::string& stmtName, const std::string& sql, int paramCount) {
    if (!conn_) return std::unexpected("Statement error: No active database connection.");

    PGresult* res = PQprepare(conn_, stmtName.c_str(), sql.c_str(), paramCount, nullptr);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::string err = PQerrorMessage(conn_);
        PQclear(res);
        return std::unexpected(err);
    }

    PQclear(res);
    return {};
}

std::expected<void, std::string> Statement::executePrepared(const std::string& stmtName, const std::vector<std::string>& params) {
    if (!conn_) return std::unexpected("Statement error: No active database connection.");

    // Construction du tableau de pointeurs C requis par libpq
    std::vector<const char*> paramValues;
    paramValues.reserve(params.size());
    for (const auto& param : params) {
        paramValues.push_back(param.c_str());
    }

    PGresult* res = PQexecPrepared(
        conn_, 
        stmtName.c_str(), 
        static_cast<int>(params.size()), 
        paramValues.data(), 
        nullptr, 
        nullptr, 
        0
    );

    ExecStatusType status = PQresultStatus(res);
    if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK) {
        std::string err = PQerrorMessage(conn_);
        PQclear(res);
        return std::unexpected(err);
    }

    PQclear(res);
    return {};
}

std::expected<void, std::string> Statement::executeBatch(const std::vector<std::string>& sqlStatements) {
    if (!conn_) return std::unexpected("Statement error: No active database connection.");

    for (const auto& sql : sqlStatements) {
        auto result = executeUpdate(sql);
        if (!result) {
            return result; // Relance l'erreur rencontrée
        }
    }
    return {};
}

} // namespace cppsqlx
