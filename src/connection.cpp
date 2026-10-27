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

namespace cppsqlx {

Connection::Connection(const std::string& connect_string)
    : connection_string_(connect_string),
      conn_(nullptr, &PQfinish) {
    sslmodes_ = {
        {sslmode::DISABLE, SSL_DISABLE},
        {sslmode::ALLOW, SSL_ALLOW},
        {sslmode::PREFER, SSL_PREFER},
        {sslmode::REQUIRE, SSL_REQUIRE},
        {sslmode::VERIFY_CA, SSL_VERIFY_CA},
        {sslmode::VERIFY_FULL, SSL_VERIFY_FULL}
    };
}

Connection::~Connection() {
    // unique_ptr automatically calls PQfinish
}

std::expected<void, std::string> Connection::connect() {
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
    return {};
}

std::expected<void, std::string> Connection::parseConnectionString() {
    // TODO: Implement parsing logic to populate conn_info_
    return {};
}

bool Connection::isConnected() const {
    return conn_ && PQstatus(conn_.get()) == CONNECTION_OK;
}

std::shared_ptr<PGconn> Connection::get() {
    // Non-owning shared_ptr for safe sharing
    return std::shared_ptr<PGconn>(conn_.get(), [](PGconn*) {});
}


std::unique_ptr<Statement> Connection::createStatement() {
    return std::make_unique<Statement>(get());
}


} // namespace cppsqlx
