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

#include "cppsqlx/join.hpp"
#include <sstream>
#include <stdexcept>

namespace cppsqlx
{

Join::Join(const Dataset& leftds)
    : leftds_(leftds)
{
}

Join::~Join() = default;

Join& Join::to(const Dataset& rightds)
{
    rightds_ = rightds;
    return *this;
}

Join& Join::on(const SQLExpression& exp)
{
    sqlx_ = exp;
    return *this;
}

Join& Join::on(const std::string& leftCol, const std::string& rightCol)
{
    sqlx_ = SQLExpression(leftCol + " = " + rightCol);
    return *this;
}

Join& Join::setType(JoinType type)
{
    type_ = type;
    return *this;
}

const Dataset& Join::leftds() const { return leftds_; }
const Dataset& Join::rightds() const { return rightds_; }
const SQLExpression& Join::expression() const { return sqlx_; }
JoinType Join::type() const { return type_; }

std::string Join::toSQL() const
{
    std::ostringstream sql;
    switch (type_)
    {
        case JoinType::INNER: sql << "INNER JOIN "; break;
        case JoinType::LEFT:  sql << "LEFT JOIN "; break;
        case JoinType::RIGHT: sql << "RIGHT JOIN "; break;
        case JoinType::FULL:  sql << "FULL JOIN "; break;
    }

    sql << rightds_.joinidentifier();
    if (!sqlx_.empty())
    {
        sql << " ON " << sqlx_.toSQL();
    }
    return sql.str();
}

} // namespace cppsqlx
