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

#include "cppsqlx/connection.hpp"
#include "cppsqlx/statement.hpp" // 🟢 Crucial : Placé ici, il rend le type Statement COMPLET pour tout le fichier
#include <libpq-fe.h> 
#include <utility>

namespace cppsqlx {

namespace {
    // Helper interne pour convertir l'enum sslmode en chaîne
    constexpr std::string_view to_string_view(sslmode mode) noexcept {
        switch (mode) {
            case sslmode::DISABLE:     return ssl_constants::DISABLE;
            case sslmode::ALLOW:       return ssl_constants::ALLOW;
            case sslmode::PREFER:      return ssl_constants::PREFER;
            case sslmode::REQUIRE:     return ssl_constants::REQUIRE;
            case sslmode::VERIFY_CA:   return ssl_constants::VERIFY_CA;
            case sslmode::VERIFY_FULL: return ssl_constants::VERIFY_FULL;
        }
        return "";
    }
} // namespace anonyme

// --- Implémentation du Deleter personnalisé ---
void PgConnDeleter::operator()(PGconn* conn) const noexcept {
    if (conn) {
        PQfinish(conn);
    }
}

// --- Constructeur & Destructeur ---
Connection::Connection(std::string connect_string)
    : connection_string_(std::move(connect_string)),
      conn_(nullptr) {
}

// 🟢 Correction : Déclaré explicitement ici. 
// Maintenant que statement.hpp est inclus au-dessus, le compilateur sait comment 
// détruire ou gérer correctement les types liés à Statement sans générer d'erreur "sizeof" ou "incomplete type".
Connection::~Connection() = default; 

// --- Cycle de vie de la connexion ---
std::expected<void, std::string> Connection::connect() {
    if (isConnected()) {
        return {};
    }

    PGconn* rawConn = PQconnectdb(connection_string_.c_str());
    if (PQstatus(rawConn) != CONNECTION_OK) {
        std::string err = PQerrorMessage(rawConn);
        PQfinish(rawConn); 
        return std::unexpected(err);
    }
    
    conn_.reset(rawConn);
    return {};
}

std::expected<void, std::string> Connection::close() {
    conn_.reset();
    conn_info_.is_valid = false;
    return {};
}

std::expected<void, std::string> Connection::parseConnectionString() {
    // TODO: Implémenter la logique de parsing pour remplir conn_info_
    return {};
}

bool Connection::isConnected() const noexcept {
    return conn_ && (PQstatus(conn_.get()) == CONNECTION_OK);
}

// --- Accesseurs et Fabrique ---
PGconn* Connection::get() const noexcept {
    return conn_.get();
}

std::unique_ptr<Statement> Connection::createStatement() {
    // 🟢 Fonctionne parfaitement maintenant car le constructeur de Statement(PGconn*) 
    // et sa définition complète sont connus du compilateur dans ce fichier.
    return std::make_unique<Statement>(get());
}

} // namespace cppsqlx
