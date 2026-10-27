
#include "cppsqlx/sqljoinexpression.hpp"

namespace cppsqlx
{

SQLJoinExpression::SQLJoinExpression()
    : operator_(SQLOperator::Equal), isLogicalJoin_(false)
{
}

SQLJoinExpression::SQLJoinExpression(SQLExpression left,
                                     SQLExpression right,
                                     SQLOperator compareOp)
    : leftExpression_(std::move(left)),
      rightExpression_(std::move(right)),
      operator_(compareOp),
      isLogicalJoin_(false)
{
}

SQLJoinExpression::SQLJoinExpression(SQLJoinExpression left,
                                     SQLJoinExpression right,
                                     SQLOperator logicalOp)
    : leftExpression_(SQLExpression("(" + left.toSQL() + ")")),
      rightExpression_(SQLExpression("(" + right.toSQL() + ")")),
      operator_(logicalOp),
      isLogicalJoin_(true)
{
}

std::string SQLJoinExpression::toSQL(bool quoted) const
{
    const std::string opStr = std::string(cppsqlx::toSQL(operator_));
    if (!isLogicalJoin_) {
        return leftExpression_.toSQL(quoted) + " " + opStr + " " + rightExpression_.toSQL(quoted);
    }
    return "(" + leftExpression_.toSQL(quoted) + ") " + opStr + " (" + rightExpression_.toSQL(quoted) + ")";
}

SQLOperator SQLJoinExpression::getOperator() const
{
    return operator_;
}

std::string SQLJoinExpression::getStringOperator() const
{
    return std::string(cppsqlx::toSQL(operator_));
}

const SQLExpression& SQLJoinExpression::leftExpression() const
{
    return leftExpression_;
}

const SQLExpression& SQLJoinExpression::rightExpression() const
{
    return rightExpression_;
}

void SQLJoinExpression::setLeftExpression(const SQLExpression &ex)
{
    leftExpression_ = ex;
}

void SQLJoinExpression::setRightExpression(const SQLExpression &ex)
{
    rightExpression_ = ex;
}

void SQLJoinExpression::setOperator(SQLOperator op)
{
    operator_ = op;
}

SQLJoinExpression SQLJoinExpression::andJoin(const SQLJoinExpression &left,
                                             const SQLJoinExpression &right)
{
    return SQLJoinExpression(left, right, SQLOperator::And);
}

SQLJoinExpression SQLJoinExpression::orJoin(const SQLJoinExpression &left,
                                            const SQLJoinExpression &right)
{
    return SQLJoinExpression(left, right, SQLOperator::Or);
}

} // namespace cppsqlx
