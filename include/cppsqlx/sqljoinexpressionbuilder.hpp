
#ifndef CPPSQLX_SQLJOINEXPRESSIONBUILDER_HPP
#define CPPSQLX_SQLJOINEXPRESSIONBUILDER_HPP

#include "cppsqlx/sqljoinexpression.hpp"
#include "cppsqlx/sqlexpression.hpp"
#include "cppsqlx/sqloperators.hpp"
#include <stdexcept>

namespace cppsqlx
{

class SQLJoinExpressionBuilder
{
public:
    SQLJoinExpressionBuilder();

    // Accept SQLExpression
    SQLJoinExpressionBuilder& left(const SQLExpression& expr);
    SQLJoinExpressionBuilder& right(const SQLExpression& expr);

    // Accept SQLJoinExpression (wrap as SQLExpression with parentheses)
    SQLJoinExpressionBuilder& left(const SQLJoinExpression& joinExpr);
    SQLJoinExpressionBuilder& right(const SQLJoinExpression& joinExpr);

    // Comparison operators
    SQLJoinExpression equalOp() const;
    SQLJoinExpression notEqualOp() const;
    SQLJoinExpression lessOp() const;
    SQLJoinExpression greaterOp() const;
    SQLJoinExpression lessOrEqualOp() const;
    SQLJoinExpression greaterOrEqualOp() const;
    SQLJoinExpression withOp(SQLOperator op) const;

    // Logical composition
    SQLJoinExpression andJoin(const SQLJoinExpression& other) const;
    SQLJoinExpression orJoin(const SQLJoinExpression& other) const;

    static SQLJoinExpression andJoin(const SQLJoinExpression& leftJoin,
                                     const SQLJoinExpression& rightJoin);
    static SQLJoinExpression orJoin(const SQLJoinExpression& leftJoin,
                                    const SQLJoinExpression& rightJoin);

private:
    SQLExpression left_;
    SQLExpression right_;

    SQLJoinExpression buildComparison(SQLOperator op) const;
};

// Factory function
inline SQLJoinExpressionBuilder JEX()
{
    return SQLJoinExpressionBuilder();
}

} // namespace cppsqlx

#endif // CPPSQLX_SQLJOINEXPRESSIONBUILDER_HPP
