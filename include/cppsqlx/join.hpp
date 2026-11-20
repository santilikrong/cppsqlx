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
#ifndef CPPSQLX_JOIN_HPP
#define CPPSQLX_JOIN_HPP

#include <string>
#include <optional>
#include "cppsqlx/sqlexpression.hpp"
#include "cppsqlx/dataset.hpp"

namespace cppsqlx
{

enum class JoinType { INNER, LEFT, RIGHT, FULL };

class Join
{
public:
    explicit Join(const Dataset& leftds);
    ~Join();

    // Fluent API
    Join& to(const Dataset& rightds);
    Join& on(const SQLExpression& exp);
    Join& on(const std::string& leftCol, const std::string& rightCol);
    Join& setType(JoinType type);

    // Accessors
    const Dataset& leftds() const;
    const Dataset& rightds() const;
    const SQLExpression& expression() const;
    JoinType type() const;

    // SQL generation
    std::string toSQL() const;

private:
    Dataset leftds_;
    Dataset rightds_;
    SQLExpression sqlx_;
    JoinType type_ = JoinType::INNER;
};

} // namespace cppsqlx

#endif // CPPSQLX_JOIN_HPP
