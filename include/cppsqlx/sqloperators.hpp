
#ifndef CPPSQLX_SQLOPERATORS_HPP
#define CPPSQLX_SQLOPERATORS_HPP

#include <string_view>
#include <array>
#include <optional>
#include <ostream>

namespace cppsqlx
{

// Strongly typed enum for SQL operators (modernized names)
enum class SQLOperator
{
    Equal,
    LessOrEqual,
    GreaterOrEqual,
    Less,
    Greater,
    NotEqual,
    NotLess,
    NotGreater,
    And,
    Or,
    Not
};

struct SQLOperatorInfo
{
    SQLOperator op;
    std::string_view symbol;
};

// Compile-time mapping
constexpr std::array<SQLOperatorInfo, 11> operatorMap {{
    {SQLOperator::Equal, "="},
    {SQLOperator::LessOrEqual, "<="},
    {SQLOperator::GreaterOrEqual, ">="},
    {SQLOperator::Less, "<"},
    {SQLOperator::Greater, ">"},
    {SQLOperator::NotEqual, "!="},
    {SQLOperator::NotLess, "!<"},
    {SQLOperator::NotGreater, "!>"},
    {SQLOperator::And, "AND"},
    {SQLOperator::Or, "OR"},
    {SQLOperator::Not, "NOT"}
}};

// Convert enum to SQL string (constexpr)
constexpr std::string_view toSQL(SQLOperator op)
{
    for (auto const& item : operatorMap)
        if (item.op == op) return item.symbol;
    return {};
}

// Convert SQL string to enum (safe; non-throwing)
inline std::optional<SQLOperator> fromSQL(std::string_view str)
{
    for (auto const& item : operatorMap)
        if (item.symbol == str) return item.op;
    return std::nullopt;
}

// Check if operator is logical
constexpr bool isLogical(SQLOperator op)
{
    return op == SQLOperator::And || op == SQLOperator::Or || op == SQLOperator::Not;
}

// Stream operator for logging/debugging
inline std::ostream& operator<<(std::ostream& os, SQLOperator op)
{
    return os << toSQL(op);
}

// -----------------------------------------------------------------------------
// Backward-compatibility aliases (keep existing code using EQUAL, AND, etc.)
// -----------------------------------------------------------------------------
constexpr SQLOperator EQUAL            = SQLOperator::Equal;
constexpr SQLOperator LESS_OR_EQUAL    = SQLOperator::LessOrEqual;
constexpr SQLOperator GREATER_OR_EQUAL = SQLOperator::GreaterOrEqual;
constexpr SQLOperator LESS             = SQLOperator::Less;
constexpr SQLOperator GREATER          = SQLOperator::Greater;
constexpr SQLOperator NOT_EQUAL        = SQLOperator::NotEqual;
constexpr SQLOperator NOT_LESS         = SQLOperator::NotLess;
constexpr SQLOperator NOT_GREATER      = SQLOperator::NotGreater;
constexpr SQLOperator AND              = SQLOperator::And;
constexpr SQLOperator OR               = SQLOperator::Or;
constexpr SQLOperator NOT              = SQLOperator::Not;

} // namespace cppsqlx

#endif // CPPSQLX_SQLOPERATORS_HPP
