
#include "cppsqlx/sqlexpression.hpp"
#include <sstream>

namespace cppsqlx
{

SQLExpression::SQLExpression()
    : expression_("")
{
}

SQLExpression::SQLExpression(std::string expr)
    : expression_(std::move(expr))
{
}

SQLExpression::SQLExpression(int value)
    : expression_(std::to_string(value))
{
}

SQLExpression::SQLExpression(std::string column, std::string tableAlias, bool doubleQuoted)
{
    if (doubleQuoted)
    {
        const auto qCol = quoteIdentifier(column);
        if (tableAlias.empty())
        {
            expression_ = qCol;
        }
        else
        {
            const auto qAlias = quoteIdentifier(tableAlias);
            expression_ = qAlias + "." + qCol;
        }
    }
    else
    {
        expression_ = tableAlias.empty() ? column : (tableAlias + "." + column);
    }
}

std::string SQLExpression::alias() const
{
    return alias_;
}

SQLExpression SQLExpression::as(const std::string& alias) const
{
    SQLExpression copy(*this);
    copy.alias_ = alias;
    return copy;
}

bool SQLExpression::empty() const noexcept
{
    return expression_.empty();
}

// 1. Modifiez toSQLWithAlias pour qu'elle passe le flag à toSQL()
std::string SQLExpression::toSQLWithAlias(bool quoted) const {
    if (alias_.empty()) {
        return toSQL(quoted); // 💡 On passe le flag à toSQL
    }
    std::string formattedAlias = quoted ? quoteIdentifier(alias_) : alias_;
    return toSQL(quoted) + " AS " + formattedAlias; // 💡 On passe le flag à toSQL
}




std::string SQLExpression::toSQL(bool quoted) const {
    if (!quoted) {
        std::string raw = expression_;
        raw.erase(std::remove(raw.begin(), raw.end(), '"'), raw.end());
        return raw;
    }
    
    // Logique d'origine lorsque quoted == true (conserve les guillemets "q1"."c12")
    return expression_; 
}



// Operators
SQLExpression SQLExpression::operator+(const SQLExpression& rhs) const
{
    return wrapBinary(rhs, "+");
}

SQLExpression SQLExpression::operator-(const SQLExpression& rhs) const
{
    return wrapBinary(rhs, "-");
}

SQLExpression SQLExpression::operator*(const SQLExpression& rhs) const
{
    return wrapBinary(rhs, "*");
}

SQLExpression SQLExpression::operator/(const SQLExpression& rhs) const
{
    return wrapBinary(rhs, "/");
}

SQLExpression SQLExpression::operator&&(const SQLExpression& rhs) const
{
    return wrapBinary(rhs, "AND");
}

SQLExpression SQLExpression::operator||(const SQLExpression& rhs) const
{
    return wrapBinary(rhs, "OR");
}

// Comparisons
SQLExpression SQLExpression::equal(const SQLExpression& rhs) const
{
    return wrapBinary(rhs, "=");
}

SQLExpression SQLExpression::notEqual(const SQLExpression& rhs) const
{
    return wrapBinary(rhs, "<>");
}

SQLExpression SQLExpression::greater(const SQLExpression& rhs) const
{
    return wrapBinary(rhs, ">");
}

SQLExpression SQLExpression::less(const SQLExpression& rhs) const
{
    return wrapBinary(rhs, "<");
}

SQLExpression SQLExpression::greaterOrEqual(const SQLExpression& rhs) const
{
    return wrapBinary(rhs, ">=");
}

SQLExpression SQLExpression::lessOrEqual(const SQLExpression& rhs) const
{
    return wrapBinary(rhs, "<=");
}

// LIKE
SQLExpression SQLExpression::like(const std::string& pattern) const
{
    return SQLExpression("(" + expression_ + " LIKE " + quoteString(pattern) + ")");
}

SQLExpression SQLExpression::notLike(const std::string& pattern) const
{
    return SQLExpression("(" + expression_ + " NOT LIKE " + quoteString(pattern) + ")");
}

// IN
SQLExpression SQLExpression::in(const std::vector<std::string>& values) const
{
    if (values.empty())
    {
        return SQLExpression("(1 = 0)");
    }
    std::ostringstream oss;
    oss << "(" << expression_ << " IN (";
    for (size_t i = 0; i < values.size(); ++i)
    {
        oss << quoteString(values[i]);
        if (i + 1 < values.size())
        {
            oss << ", ";
        }
    }
    oss << "))";
    return SQLExpression(oss.str());
}

SQLExpression SQLExpression::in(const std::vector<int>& values) const
{
    if (values.empty())
    {
        return SQLExpression("(1 = 0)");
    }
    std::ostringstream oss;
    oss << "(" << expression_ << " IN (";
    for (size_t i = 0; i < values.size(); ++i)
    {
        oss << values[i];
        if (i + 1 < values.size())
        {
            oss << ", ";
        }
    }
    oss << "))";
    return SQLExpression(oss.str());
}

SQLExpression SQLExpression::in(const std::vector<SQLExpression>& values) const
{
    if (values.empty())
    {
        return SQLExpression("(1 = 0)");
    }
    std::ostringstream oss;
    oss << "(" << expression_ << " IN (";
    for (size_t i = 0; i < values.size(); ++i)
    {
        oss << values[i].toSQL();
        if (i + 1 < values.size())
        {
            oss << ", ";
        }
    }
    oss << "))";
    return SQLExpression(oss.str());
}

// BETWEEN
SQLExpression SQLExpression::between(const SQLExpression& low, const SQLExpression& high) const
{
    return SQLExpression("(" + expression_ + " BETWEEN " + low.expression_ + " AND " + high.expression_ + ")");
}

// Functions
SQLExpression SQLExpression::avg() const
{
    return wrapFunction("AVG");
}

SQLExpression SQLExpression::sum() const
{
    return wrapFunction("SUM");
}

SQLExpression SQLExpression::count() const
{
    return wrapFunction("COUNT");
}

SQLExpression SQLExpression::max() const
{
    return wrapFunction("MAX");
}

SQLExpression SQLExpression::min() const
{
    return wrapFunction("MIN");
}

SQLExpression SQLExpression::upper() const
{
    return wrapFunction("UPPER");
}

SQLExpression SQLExpression::lower() const
{
    return wrapFunction("LOWER");
}

SQLExpression SQLExpression::coalesce(const std::string& fallback) const
{
    return SQLExpression("COALESCE(" + expression_ + ", " + quoteString(fallback) + ")");
}

SQLExpression SQLExpression::coalesce(std::initializer_list<SQLExpression> args)
{
    std::ostringstream oss;
    oss << "COALESCE(";
    size_t count = 0;
    for (const auto& arg : args)
    {
        oss << arg.toSQL();
        if (++count < args.size())
        {
            oss << ", ";
        }
    }
    oss << ")";
    return SQLExpression(oss.str());
}

SQLExpression SQLExpression::round() const
{
    return wrapFunction("ROUND");
}

SQLExpression SQLExpression::round(int decimals) const
{
    return SQLExpression("ROUND(" + expression_ + ", " + std::to_string(decimals) + ")");
}

SQLExpression SQLExpression::power(int p) const
{
    return SQLExpression("POWER(" + expression_ + ", " + std::to_string(p) + ")");
}

SQLExpression SQLExpression::nullif(const SQLExpression& other) const
{
    return SQLExpression("NULLIF(" + expression_ + ", " + other.expression_ + ")");
}

SQLExpression SQLExpression::caseExpr(
    const std::vector<std::pair<SQLExpression, SQLExpression>>& whenThenPairs,
    const SQLExpression& elseExpr)
{
    std::ostringstream oss;
    oss << "CASE ";
    for (const auto& pair : whenThenPairs)
    {
        oss << "WHEN " << pair.first.toSQL() << " THEN " << pair.second.toSQL() << " ";
    }
    oss << "ELSE " << elseExpr.toSQL() << " END";
    return SQLExpression(oss.str());
}

SQLExpression SQLExpression::isNull() const
{
    return SQLExpression("(" + expression_ + " IS NULL)");
}

SQLExpression SQLExpression::isNotNull() const
{
    return SQLExpression("(" + expression_ + " IS NOT NULL)");
}

// Static helpers
std::string SQLExpression::quoteIdentifier(const std::string& ident)
{
    std::string out;
    out.reserve(ident.size() + 2);
    out.push_back('"');
    for (char c : ident)
    {
        if (c == '"')
        {
            out.push_back('"');
            out.push_back('"');
        }
        else
        {
            out.push_back(c);
        }
    }
    out.push_back('"');
    return out;
}

std::string SQLExpression::quoteString(const std::string& s)
{
    std::string out;
    out.reserve(s.size() + 2);
    out.push_back('\'');
    for (char c : s)
    {
        if (c == '\'')
        {
            out.push_back('\'');
            out.push_back('\'');
        }
        else
        {
            out.push_back(c);
        }
    }
    out.push_back('\'');
    return out;
}

// Private helpers
SQLExpression SQLExpression::wrapFunction(const std::string& func) const
{
    SQLExpression res(func + "(" + expression_ + ")");
    res.alias_ = alias_;
    return res;
}

SQLExpression SQLExpression::wrapBinary(const SQLExpression& rhs, const std::string& op) const
{
    return SQLExpression("(" + expression_ + " " + op + " " + rhs.expression_ + ")");
}

} // namespace cppsqlx
