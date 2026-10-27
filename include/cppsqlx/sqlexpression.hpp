
#ifndef SQLEXPRESSION_HPP
#define SQLEXPRESSION_HPP

#include <string>
#include <vector>
#include <initializer_list>
#include <utility>

namespace cppsqlx
{

class SQLExpression
{
public:
    SQLExpression();
    explicit SQLExpression(std::string expr);
    explicit SQLExpression(int value);

    SQLExpression(std::string column, std::string tableAlias, bool doubleQuoted = false);

    std::string alias() const;
    SQLExpression as(const std::string& alias) const;

    bool empty() const noexcept;
    std::string toSQL(bool quoted = true) const; // 💡 Ajout du paramètre ici aussi
    std::string toSQLWithAlias(bool quoted = true) const;

    SQLExpression operator+(const SQLExpression& rhs) const;
    SQLExpression operator-(const SQLExpression& rhs) const;
    SQLExpression operator*(const SQLExpression& rhs) const;
    SQLExpression operator/(const SQLExpression& rhs) const;
    SQLExpression operator&&(const SQLExpression& rhs) const;
    SQLExpression operator||(const SQLExpression& rhs) const;

    SQLExpression equal(const SQLExpression& rhs) const;
    SQLExpression notEqual(const SQLExpression& rhs) const;
    SQLExpression greater(const SQLExpression& rhs) const;
    SQLExpression less(const SQLExpression& rhs) const;
    SQLExpression greaterOrEqual(const SQLExpression& rhs) const;
    SQLExpression lessOrEqual(const SQLExpression& rhs) const;

    SQLExpression like(const std::string& pattern) const;
    SQLExpression notLike(const std::string& pattern) const;

    SQLExpression in(const std::vector<std::string>& values) const;
    SQLExpression in(const std::vector<int>& values) const;
    SQLExpression in(const std::vector<SQLExpression>& values) const;

    SQLExpression between(const SQLExpression& low, const SQLExpression& high) const;

    SQLExpression avg() const;
    SQLExpression sum() const;
    SQLExpression count() const;
    SQLExpression max() const;
    SQLExpression min() const;
    SQLExpression upper() const;
    SQLExpression lower() const;

    SQLExpression coalesce(const std::string& fallback) const;
    static SQLExpression coalesce(std::initializer_list<SQLExpression> args);

    SQLExpression round() const;
    SQLExpression round(int decimals) const;
    SQLExpression power(int p) const;

    SQLExpression nullif(const SQLExpression& other) const;

    static SQLExpression caseExpr(
        const std::vector<std::pair<SQLExpression, SQLExpression>>& whenThenPairs,
        const SQLExpression& elseExpr);

    SQLExpression isNull() const;
    SQLExpression isNotNull() const;

    static std::string quoteIdentifier(const std::string& ident);
    static std::string quoteString(const std::string& s);

private:
    std::string expression_;
    std::string alias_;

    SQLExpression wrapFunction(const std::string& func) const;
    SQLExpression wrapBinary(const SQLExpression& rhs, const std::string& op) const;
};

} // namespace cppsqlx

#endif // SQLEXPRESSION_HPP
