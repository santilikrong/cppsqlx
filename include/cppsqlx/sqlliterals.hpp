#ifndef CPPSQLX_SQLLITERALS_HPP
#define CPPSQLX_SQLLITERALS_HPP

#include "cppsqlx/sqlexpression.hpp"
#include <string>
#include <string_view>
#include <cstddef>
#include <cstdint>

namespace cppsqlx::literals {

// Literal for SQL string (quoted)
inline cppsqlx::SQLExpression operator""_sql(const char* str, std::size_t len) {
    return cppsqlx::SQLExpression(cppsqlx::SQLExpression::quoteString(std::string(str, len)));
}

// Literal for SQL string non quoted
inline cppsqlx::SQLExpression operator""_sqlx(const char* str, std::size_t len) {
    return cppsqlx::SQLExpression(std::string_view(str, len));
}

// Literal for SQL integer
inline cppsqlx::SQLExpression operator""_sql(unsigned long long value) {
    return cppsqlx::SQLExpression(static_cast<int64_t>(value));
}

// Literal for column reference (with optional alias prefix)
inline cppsqlx::SQLExpression operator""_col(const char* str, std::size_t len) {
    std::string_view s(str, len);
    
    if (auto dotPos = s.find('.'); dotPos != std::string_view::npos) {
        std::string_view alias = s.substr(0, dotPos);
        std::string_view column = s.substr(dotPos + 1);
        return cppsqlx::SQLExpression(std::string(column), std::string(alias), true);
    }
    return cppsqlx::SQLExpression(std::string(s), "", true);
}

// Common SQL constants
// 🟢 CORRECTION : Utilisation de std::string_view explicite pour éliminer l'ambiguïté des surcharges de constructeurs
inline cppsqlx::SQLExpression true_sql() noexcept { return cppsqlx::SQLExpression(std::string_view("TRUE")); }
inline cppsqlx::SQLExpression false_sql() noexcept { return cppsqlx::SQLExpression(std::string_view("FALSE")); }
inline cppsqlx::SQLExpression null_sql() noexcept { return cppsqlx::SQLExpression(std::string_view("NULL")); }

} // namespace cppsqlx::literals

#endif // CPPSQLX_SQLLITERALS_HPP