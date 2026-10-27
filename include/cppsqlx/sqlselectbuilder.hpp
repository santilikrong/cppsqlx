
#ifndef CPPSQLX_SQLSELECTBUILDER_HPP
#define CPPSQLX_SQLSELECTBUILDER_HPP

#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include "cppsqlx/sqlexpression.hpp"
#include "cppsqlx/sqljoinexpression.hpp"

namespace cppsqlx
{

enum class SQLJoinType { Inner, Left, Right, Full, Cross };

struct OrderByItem
{
    SQLExpression expr;
    bool ascending = true;
    std::optional<std::string> nullsDirective;
};

struct JoinSpec
{
    SQLJoinType type;
    std::string table;
    std::string alias;
    std::optional<SQLJoinExpression> on;
};

class SQLSelectBuilder
{
public:
    SQLSelectBuilder();

    SQLSelectBuilder& select(const SQLExpression& expr);
    SQLSelectBuilder& select(std::initializer_list<SQLExpression> exprs);

    SQLSelectBuilder& from(std::string_view table, std::string_view alias = {});

    SQLSelectBuilder& innerJoin(std::string_view table, std::string_view alias, const SQLJoinExpression& on);
    SQLSelectBuilder& leftJoin(std::string_view table, std::string_view alias, const SQLJoinExpression& on);
    SQLSelectBuilder& rightJoin(std::string_view table, std::string_view alias, const SQLJoinExpression& on);
    SQLSelectBuilder& fullJoin(std::string_view table, std::string_view alias, const SQLJoinExpression& on);
    SQLSelectBuilder& crossJoin(std::string_view table, std::string_view alias = {});

    SQLSelectBuilder& where(const SQLExpression& predicate);
    SQLSelectBuilder& where(const SQLJoinExpression& predicate);
    SQLSelectBuilder& andWhere(const SQLExpression& predicate);
    SQLSelectBuilder& orWhere(const SQLExpression& predicate);

    SQLSelectBuilder& groupBy(std::initializer_list<SQLExpression> exprs);
    SQLSelectBuilder& orderBy(const SQLExpression& expr, bool ascending = true,
                              std::optional<std::string> nullsDirective = std::nullopt);

    SQLSelectBuilder& limit(int n);
    SQLSelectBuilder& offset(int n);

    std::string toSQL() const;

private:
    static std::string joinTypeToString(SQLJoinType type);

    std::vector<SQLExpression> selectExprs_;
    std::string fromTable_;
    std::string fromAlias_;
    std::vector<JoinSpec> joins_;
    std::vector<SQLExpression> whereExprs_;
    std::vector<SQLExpression> groupByExprs_;
    std::vector<OrderByItem> orderBy_;
    std::optional<int> limit_;
    std::optional<int> offset_;
};

} // namespace cppsqlx

#endif