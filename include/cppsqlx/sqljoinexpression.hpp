
#ifndef CPPSQLX_SQLJOINEXPRESSION_HPP
#define CPPSQLX_SQLJOINEXPRESSION_HPP

#include <string>
#include "cppsqlx/sqlexpression.hpp"
#include "cppsqlx/sqloperators.hpp"

namespace cppsqlx
{

class SQLJoinExpression
{
public:
    SQLJoinExpression();
    explicit SQLJoinExpression(SQLExpression left,
                               SQLExpression right,
                               SQLOperator compareOp);
    explicit SQLJoinExpression(SQLJoinExpression left,
                               SQLJoinExpression right,
                               SQLOperator logicalOp);
    ~SQLJoinExpression() = default;

    std::string toSQL(bool quoted = false) const;

    SQLOperator getOperator() const;
    std::string getStringOperator() const;

    const SQLExpression& leftExpression() const;
    const SQLExpression& rightExpression() const;

    void setLeftExpression(const SQLExpression &ex);
    void setRightExpression(const SQLExpression &ex);
    void setOperator(SQLOperator op);

    static SQLJoinExpression andJoin(const SQLJoinExpression &left,
                                     const SQLJoinExpression &right);
    static SQLJoinExpression orJoin(const SQLJoinExpression &left,
                                    const SQLJoinExpression &right);

private:
    SQLExpression leftExpression_;
    SQLExpression rightExpression_;
    SQLOperator operator_;
    bool isLogicalJoin_;
    
};

} // namespace cppsqlx

#endif // CPPSQLX_SQLJOINEXPRESSION_HPP
