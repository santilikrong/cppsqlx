#include "cppsqlx/sqlexpression.hpp"
#include <sstream>
#include <algorithm>

// 🟢 AJOUT CRUCIAL : Permet d'utiliser l'opérateur ""sv pour créer des std::string_view
// et ainsi lever toute ambiguïté pour le compilateur sur les chaînes littérales.
using namespace std::string_view_literals;

namespace cppsqlx
{

// Constructeurs
SQLExpression::SQLExpression(std::string expr) noexcept
    : expression_(std::move(expr))
{
}

SQLExpression::SQLExpression(std::string_view expr)
    : expression_(expr)
{
}

SQLExpression::SQLExpression(int64_t value)
    : expression_(std::to_string(value))
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

// Accesseurs
const std::string& SQLExpression::alias() const noexcept
{
    return alias_;
}

SQLExpression SQLExpression::as(std::string_view alias) const
{
    SQLExpression copy(*this);
    copy.alias_ = alias;
    return copy;
}

bool SQLExpression::empty() const noexcept
{
    return expression_.empty();
}

std::string SQLExpression::toSQLWithAlias(bool quoted) const {
    if (alias_.empty()) {
        return toSQL(quoted);
    }
    std::string formattedAlias = quoted ? quoteIdentifier(alias_) : alias_;
    return toSQL(quoted) + " AS " + formattedAlias;
}

std::string SQLExpression::toSQL(bool quoted) const {
    if (!quoted) {
        std::string raw = expression_;
        raw.erase(std::remove(raw.begin(), raw.end(), '"'), raw.end());
        return raw;
    }
    return expression_; 
}

// Opérateurs mathématiques et logiques
SQLExpression SQLExpression::operator+(const SQLExpression& rhs) const { return wrapBinary(rhs, "+"sv); }
SQLExpression SQLExpression::operator-(const SQLExpression& rhs) const { return wrapBinary(rhs, "-"sv); }
SQLExpression SQLExpression::operator*(const SQLExpression& rhs) const { return wrapBinary(rhs, "*"sv); }
SQLExpression SQLExpression::operator/(const SQLExpression& rhs) const { return wrapBinary(rhs, "/"sv); }
SQLExpression SQLExpression::operator&&(const SQLExpression& rhs) const { return wrapBinary(rhs, "AND"sv); }
SQLExpression SQLExpression::operator||(const SQLExpression& rhs) const { return wrapBinary(rhs, "OR"sv); }

// Comparaisons
SQLExpression SQLExpression::equal(const SQLExpression& rhs) const { return wrapBinary(rhs, "="sv); }
SQLExpression SQLExpression::notEqual(const SQLExpression& rhs) const { return wrapBinary(rhs, "<>"sv); }
SQLExpression SQLExpression::greater(const SQLExpression& rhs) const { return wrapBinary(rhs, ">"sv); }
SQLExpression SQLExpression::less(const SQLExpression& rhs) const { return wrapBinary(rhs, "<"sv); }
SQLExpression SQLExpression::greaterOrEqual(const SQLExpression& rhs) const { return wrapBinary(rhs, ">="sv); }
SQLExpression SQLExpression::lessOrEqual(const SQLExpression& rhs) const { return wrapBinary(rhs, "<="sv); }

// LIKE
SQLExpression SQLExpression::like(std::string_view pattern) const
{
    return SQLExpression("(" + expression_ + " LIKE " + quoteString(pattern) + ")");
}

SQLExpression SQLExpression::notLike(std::string_view pattern) const
{
    return SQLExpression("(" + expression_ + " NOT LIKE " + quoteString(pattern) + ")");
}

// Clauses IN et BETWEEN
SQLExpression SQLExpression::in(const std::vector<std::string>& values) const
{
    // 🟢 Correction du type littéral ambigu par l'utilisation de ""sv
    if (values.empty()) return SQLExpression("(1 = 0)"sv);
    
    std::ostringstream oss;
    oss << "(" << expression_ << " IN (";
    for (size_t i = 0; i < values.size(); ++i)
    {
        oss << quoteString(values[i]);
        if (i + 1 < values.size()) oss << ", ";
    }
    oss << "))";
    return SQLExpression(oss.str());
}

SQLExpression SQLExpression::in(const std::vector<int>& values) const
{
    // 🟢 Correction du type littéral ambigu par l'utilisation de ""sv
    if (values.empty()) return SQLExpression("(1 = 0)"sv);
    
    std::ostringstream oss;
    oss << "(" << expression_ << " IN (";
    for (size_t i = 0; i < values.size(); ++i)
    {
        oss << values[i];
        if (i + 1 < values.size()) oss << ", ";
    }
    oss << "))";
    return SQLExpression(oss.str());
}

SQLExpression SQLExpression::in(const std::vector<SQLExpression>& values) const
{
    // 🟢 Correction du type littéral ambigu par l'utilisation de ""sv
    if (values.empty()) return SQLExpression("(1 = 0)"sv);
    
    std::ostringstream oss;
    oss << "(" << expression_ << " IN (";
    for (size_t i = 0; i < values.size(); ++i)
    {
        oss << values[i].toSQL();
        if (i + 1 < values.size()) oss << ", ";
    }
    oss << "))";
    return SQLExpression(oss.str());
}

SQLExpression SQLExpression::between(const SQLExpression& low, const SQLExpression& high) const
{
    return SQLExpression("(" + expression_ + " BETWEEN " + low.expression_ + " AND " + high.expression_ + ")");
}

// Fonctions d'agrégation et SQL
SQLExpression SQLExpression::avg() const { return wrapFunction("AVG"sv); }
SQLExpression SQLExpression::sum() const { return wrapFunction("SUM"sv); }
SQLExpression SQLExpression::count() const { return wrapFunction("COUNT"sv); }
SQLExpression SQLExpression::max() const { return wrapFunction("MAX"sv); }
SQLExpression SQLExpression::min() const { return wrapFunction("MIN"sv); }
SQLExpression SQLExpression::upper() const { return wrapFunction("UPPER"sv); }
SQLExpression SQLExpression::lower() const { return wrapFunction("LOWER"sv); }
SQLExpression SQLExpression::round() const { return wrapFunction("ROUND"sv); }

SQLExpression SQLExpression::coalesce(std::string_view fallback) const
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
        if (++count < args.size()) oss << ", ";
    }
    oss << ")";
    return SQLExpression(oss.str());
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

// Helpers de Quoting Statiques et Optimisés
std::string SQLExpression::quoteIdentifier(std::string_view ident)
{
    std::string out;
    out.reserve(ident.size() + 2);
    out.push_back('"');
    for (char c : ident)
    {
        if (c == '"') { out.push_back('"'); out.push_back('"'); }
        else out.push_back(c);
    }
    out.push_back('"');
    return out;
}

std::string SQLExpression::quoteString(std::string_view s)
{
    std::string out;
    out.reserve(s.size() + 2);
    out.push_back('\'');
    for (char c : s)
    {
        if (c == '\'') { out.push_back('\''); out.push_back('\''); }
        else out.push_back(c);
    }
    out.push_back('\'');
    return out;
}

std::string SQLExpression::quoteString(std::string s)
{
    return quoteString(std::string_view(s));
}

// Helpers Privés
SQLExpression SQLExpression::wrapFunction(std::string_view func) const
{
    return SQLExpression(std::string(func) + "(" + expression_ + ")");
}

SQLExpression SQLExpression::wrapBinary(const SQLExpression& rhs, std::string_view op) const
{
    return SQLExpression("(" + expression_ + " " + std::string(op) + " " + rhs.expression_ + ")");
}

} // namespace cppsqlx
