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

#ifndef STATEMENT_HPP
#define STATEMENT_HPP

#include <string>
#include <vector>
#include <expected>
#include "cppsqlx/resultset.hpp"

// Déclaration anticipée de libpq pour éviter les fuites d'en-têtes C
struct pg_conn;
typedef struct pg_conn PGconn;

namespace cppsqlx {

class Statement {
public:
    // Le Statement observe la connexion (non-owning pointer).
    explicit Statement(PGconn* conn);
    ~Statement() = default;

    // Règle des 5 : On bloque la copie
    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;
    Statement(Statement&&) noexcept = default;
    Statement& operator=(Statement&&) noexcept = default;

    std::expected<void, std::string>         executeUpdate(const std::string& sql);
    std::expected<Resultset, std::string>    execute(const std::string& sql); // Orthographe correcte : Resultset
    std::expected<void, std::string>         prepareStatement(const std::string& stmtName, const std::string& sql, int paramCount);
    std::expected<void, std::string>         executePrepared(const std::string& stmtName, const std::vector<std::string>& params);
    std::expected<void, std::string>         executeBatch(const std::vector<std::string>& sqlStatements);

private:
    PGconn* conn_; // Pointeur brut d'observation
};

} // namespace cppsqlx

#endif // STATEMENT_HPP
