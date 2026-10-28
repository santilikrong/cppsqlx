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
#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <expected>

// Déclaration anticipée du type de libpq pour éviter d'inclure les en-têtes C
// dans l'interface publique de votre bibliothèque C++.
struct pg_conn;
typedef struct pg_conn PGconn;

namespace cppsqlx {

// Déclaration anticipée de Statement (évite d'inclure statement.hpp si non requis)
class Statement;

// Constantes typées et scopées remplaçant les #define
namespace ssl_constants {
    constexpr std::string_view REQUIRE     = "require";
    constexpr std::string_view ALLOW       = "allow";
    constexpr std::string_view DISABLE     = "disable";
    constexpr std::string_view PREFER      = "prefer";
    constexpr std::string_view VERIFY_CA   = "verify-ca";
    constexpr std::string_view VERIFY_FULL = "verify-full";
}

enum class sslmode { DISABLE, ALLOW, PREFER, REQUIRE, VERIFY_CA, VERIFY_FULL };

struct connection_info {
    bool is_valid = false;
    std::optional<std::string> user;
    std::optional<std::string> password;
    std::optional<std::string> host;
    std::optional<std::string> hostaddr;
    std::optional<int> port;
    std::optional<std::string> dbname;
    std::optional<int> connect_timeout;
    std::optional<std::string> application_name;
    std::optional<int> keepalives;
    std::optional<int> keepalives_idle;
    std::optional<int> keepalives_interval;
    std::optional<int> keepalives_count;
    std::optional<int> tcp_user_timeout;
    std::optional<sslmode> ssl_mode; // Utilisation de l'enum typé
    std::optional<std::string> sslcert;
    std::optional<std::string> sslkey;
    std::optional<std::string> sslkeylogfile;
    std::optional<std::string> sslpassword;
    std::optional<std::string> sslcertmode;
    std::optional<std::string> passfile;
};

// Custom deleter personnalisé pour PGconn permettant de différer l'appel 
// à PQfinish dans le fichier d'implémentation (.cpp)
struct PgConnDeleter {
    void operator()(PGconn* conn) const noexcept;
};

class Connection {
public:
    explicit Connection(std::string connect_string);
    ~Connection();

    // Règle des 5 : Empêcher la copie involontaire (gestion de ressource unique)
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
    Connection(Connection&&) noexcept = default;
    Connection& operator=(Connection&&) noexcept = default;

    std::expected<void, std::string> connect();
    std::expected<void, std::string> close();
    std::unique_ptr<Statement> createStatement();
    std::expected<void, std::string> parseConnectionString();
    
    bool isConnected() const noexcept;

    // Renvoyer un pointeur nu pour l'observation externe sécurisée.
    // L'appelant ne possède pas la ressource.
    PGconn* get() const noexcept;

private:
    std::string connection_string_;
    std::unique_ptr<PGconn, PgConnDeleter> conn_{nullptr};
    connection_info conn_info_;
};

} // namespace cppsqlx

#endif // CONNECTION_HPP
