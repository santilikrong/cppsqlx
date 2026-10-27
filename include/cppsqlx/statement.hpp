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

#include <cppsqlx/cppsqlx.hpp>
#include "cppsqlx/resultset.hpp"

namespace cppsqlx {

class Statement {
public:
    explicit Statement(std::shared_ptr<PGconn> conn);
    ~Statement() = default;

    std::expected<void, std::string>         executeUpdate(const std::string& sql);
    std::expected<Resultset, std::string>    execute(const std::string& sql);
    std::expected<void, std::string>         prepareStatement(const std::string& stmtName, const std::string& sql, int paramCount);
    std::expected<void, std::string>         executePrepared(const std::string& stmtName, const std::vector<std::string>& params);
    std::expected<void, std::string>         executeBatch(const std::vector<std::string>& sqlStatements);

private:
    std::shared_ptr<PGconn> conn_;
};

} // namespace cppsqlx

#endif // STATEMENT_HPP