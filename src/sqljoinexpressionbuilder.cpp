
#include "cppsqlx/sqljoinexpressionbuilder.hpp"

namespace cppsqlx
{

SQLJoinExpressionBuilder::SQLJoinExpressionBuilder() = default;

SQLJoinExpressionBuilder& SQLJoinExpressionBuilder::left(const SQLExpression& expr)
{
    left_ = expr;
    return *this;
}

SQLJoinExpressionBuilder& SQLJoinExpressionBuilder::right(const SQLExpression& expr)
{
    right_ = expr;
    return *this;
}

SQLJoinExpressionBuilder& SQLJoinExpressionBuilder::left(const SQLJoinExpression& joinExpr)
{
    left_ = SQLExpression("(" + joinExpr.toSQL() + ")");
    return *this;
}

SQLJoinExpressionBuilder& SQLJoinExpressionBuilder::right(const SQLJoinExpression& joinExpr)
{
    right_ = SQLExpression("(" + joinExpr.toSQL() + ")");
    return *this;
}

SQLJoinExpression SQLJoinExpressionBuilder::equalOp() const            { return buildComparison(SQLOperator::Equal); }
SQLJoinExpression SQLJoinExpressionBuilder::notEqualOp() const         { return buildComparison(SQLOperator::NotEqual); }
SQLJoinExpression SQLJoinExpressionBuilder::lessOp() const             { return buildComparison(SQLOperator::Less); }
SQLJoinExpression SQLJoinExpressionBuilder::greaterOp() const          { return buildComparison(SQLOperator::Greater); }
SQLJoinExpression SQLJoinExpressionBuilder::lessOrEqualOp() const      { return buildComparison(SQLOperator::LessOrEqual); }
SQLJoinExpression SQLJoinExpressionBuilder::greaterOrEqualOp() const   { return buildComparison(SQLOperator::GreaterOrEqual); }
SQLJoinExpression SQLJoinExpressionBuilder::withOp(SQLOperator op) const
{
    return buildComparison(op);
}

SQLJoinExpression SQLJoinExpressionBuilder::andJoin(const SQLJoinExpression& other) const
{
    return SQLJoinExpression::andJoin(buildComparison(SQLOperator::Equal), other);
}

SQLJoinExpression SQLJoinExpressionBuilder::orJoin(const SQLJoinExpression& other) const
{
    return SQLJoinExpression::orJoin(buildComparison(SQLOperator::Equal), other);
}

SQLJoinExpression SQLJoinExpressionBuilder::andJoin(const SQLJoinExpression& leftJoin,
                                                    const SQLJoinExpression& rightJoin)
{
    return SQLJoinExpression::andJoin(leftJoin, rightJoin);
}

SQLJoinExpression SQLJoinExpressionBuilder::orJoin(const SQLJoinExpression& leftJoin,
                                                   const SQLJoinExpression& rightJoin)
{
    return SQLJoinExpression::orJoin(leftJoin, rightJoin);
}

SQLJoinExpression SQLJoinExpressionBuilder::buildComparison(SQLOperator op) const
{
    if (isLogical(op)) {
        throw std::invalid_argument("Logical operator passed to comparison builder");
    }
    if (left_.empty() || right_.empty()) {
        throw std::invalid_argument("Both left and right SQLExpression must be set before choosing an operator");
    }
    return SQLJoinExpression(left_, right_, op);
}

} // namespace cppsqlx
