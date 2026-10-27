
#ifndef CPPSQLX_SQLLITERALS_HPP
#define CPPSQLX_SQLLITERALS_HPP

#include "cppsqlx/sqlexpression.hpp"
#include <string>

namespace cppsqlx::literals {

// Literal for SQL string (quoted)
inline cppsqlx::SQLExpression operator""_sql(const char* str, std::size_t) {
    return cppsqlx::SQLExpression(cppsqlx::SQLExpression::quoteString(std::string(str)));
}

// Literal for SQL string non quoted
inline cppsqlx::SQLExpression operator""_sqlx(const char* str, std::size_t) {
    return cppsqlx::SQLExpression(cppsqlx::SQLExpression(std::string(str)));
}

// Literal for SQL integer
inline cppsqlx::SQLExpression operator""_sql(unsigned long long value) {
    return cppsqlx::SQLExpression(static_cast<int>(value));
}

// Literal for column reference (with optional alias prefix)
inline cppsqlx::SQLExpression operator""_col(const char* str, std::size_t) {
    std::string s(str);
    auto dotPos = s.find('.');
    if (dotPos != std::string::npos) {
        std::string alias = s.substr(0, dotPos);
        std::string column = s.substr(dotPos + 1);
        return cppsqlx::SQLExpression(column, alias, true);
    }
    return cppsqlx::SQLExpression(s, "", true);
}

// Common SQL constants
inline cppsqlx::SQLExpression true_sql() { return cppsqlx::SQLExpression("TRUE"); }
inline cppsqlx::SQLExpression false_sql() { return cppsqlx::SQLExpression("FALSE"); }
inline cppsqlx::SQLExpression null_sql() { return cppsqlx::SQLExpression("NULL"); }

} // namespace cppsqlx::literals

#endif // CPPSQLX_SQLLITERALS_HPP
