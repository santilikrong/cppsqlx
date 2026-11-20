
#include "cppsqlx/sqlselectbuilder.hpp"
#include <sstream>

namespace cppsqlx
{

SQLSelectBuilder::SQLSelectBuilder() = default;

SQLSelectBuilder& SQLSelectBuilder::select(const SQLExpression& expr)
{
    selectExprs_.push_back(expr);
    return *this;
}

SQLSelectBuilder& SQLSelectBuilder::select(std::initializer_list<SQLExpression> exprs)
{
    selectExprs_.insert(selectExprs_.end(), exprs.begin(), exprs.end());
    return *this;
}

SQLSelectBuilder& SQLSelectBuilder::from(std::string_view table, std::string_view alias)
{
    fromTable_ = std::string(table);
    fromAlias_ = std::string(alias);
    return *this;
}

SQLSelectBuilder& SQLSelectBuilder::innerJoin(std::string_view table, std::string_view alias, const SQLJoinExpression& on)
{
    joins_.push_back({SQLJoinType::Inner, std::string(table), std::string(alias), on});
    return *this;
}

SQLSelectBuilder& SQLSelectBuilder::leftJoin(std::string_view table, std::string_view alias, const SQLJoinExpression& on)
{
    joins_.push_back({SQLJoinType::Left, std::string(table), std::string(alias), on});
    return *this;
}

SQLSelectBuilder& SQLSelectBuilder::rightJoin(std::string_view table, std::string_view alias, const SQLJoinExpression& on)
{
    joins_.push_back({SQLJoinType::Right, std::string(table), std::string(alias), on});
    return *this;
}

SQLSelectBuilder& SQLSelectBuilder::fullJoin(std::string_view table, std::string_view alias, const SQLJoinExpression& on)
{
    joins_.push_back({SQLJoinType::Full, std::string(table), std::string(alias), on});
    return *this;
}

SQLSelectBuilder& SQLSelectBuilder::crossJoin(std::string_view table, std::string_view alias)
{
    joins_.push_back({SQLJoinType::Cross, std::string(table), std::string(alias), std::nullopt});
    return *this;
}

SQLSelectBuilder& SQLSelectBuilder::where(const SQLExpression& predicate)
{
    whereExprs_.push_back(predicate);
    return *this;
}

SQLSelectBuilder& SQLSelectBuilder::where(const SQLJoinExpression& predicate)
{
    whereExprs_.push_back(SQLExpression("(" + predicate.toSQL() + ")"));
    return *this;
}

SQLSelectBuilder& SQLSelectBuilder::andWhere(const SQLExpression& predicate)
{
    if (whereExprs_.empty()) return where(predicate);
    auto combined = SQLExpression("(" + whereExprs_.back().toSQL() + ")").operator&&(SQLExpression("(" + predicate.toSQL() + ")"));
    whereExprs_.back() = combined;
    return *this;
}

SQLSelectBuilder& SQLSelectBuilder::orWhere(const SQLExpression& predicate)
{
    if (whereExprs_.empty()) return where(predicate);
    auto combined = SQLExpression("(" + whereExprs_.back().toSQL() + ")").operator||(SQLExpression("(" + predicate.toSQL() + ")"));
    whereExprs_.back() = combined;
    return *this;
}

SQLSelectBuilder& SQLSelectBuilder::groupBy(std::initializer_list<SQLExpression> exprs)
{
    groupByExprs_.insert(groupByExprs_.end(), exprs.begin(), exprs.end());
    return *this;
}

SQLSelectBuilder& SQLSelectBuilder::orderBy(const SQLExpression& expr, bool ascending,
                                            std::optional<std::string> nullsDirective)
{
    orderBy_.push_back(OrderByItem{expr, ascending, std::move(nullsDirective)});
    return *this;
}

SQLSelectBuilder& SQLSelectBuilder::limit(int n)
{
    limit_ = n;
    return *this;
}

SQLSelectBuilder& SQLSelectBuilder::offset(int n)
{
    offset_ = n;
    return *this;
}

std::string SQLSelectBuilder::toSQL() const
{
    std::ostringstream oss;

    // SELECT
    oss << "SELECT ";
    if (selectExprs_.empty()) {
        oss << "*";
    } else {
        for (size_t i = 0; i < selectExprs_.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << selectExprs_[i].toSQLWithAlias();
        }
    }

    // FROM
    if (!fromTable_.empty()) {
        oss << " FROM " << SQLExpression::quoteIdentifier(fromTable_);
        if (!fromAlias_.empty()) {
            oss << " AS " << SQLExpression::quoteIdentifier(fromAlias_);
        }
    }

    // JOINs
    for (const auto& j : joins_) {
        oss << " " << joinTypeToString(j.type) << " " << SQLExpression::quoteIdentifier(j.table);
        if (!j.alias.empty()) {
            oss << " AS " << SQLExpression::quoteIdentifier(j.alias);
        }
        if (j.on.has_value() && j.type != SQLJoinType::Cross) {
            oss << " ON " << j.on->toSQL();
        }
    }

    // WHERE
    if (!whereExprs_.empty()) {
        oss << " WHERE ";
        for (size_t i = 0; i < whereExprs_.size(); ++i) {
            if (i > 0) oss << " AND ";
            oss << "(" << whereExprs_[i].toSQL() << ")";
        }
    }

    // GROUP BY
    if (!groupByExprs_.empty()) {
        oss << " GROUP BY ";
        for (size_t i = 0; i < groupByExprs_.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << groupByExprs_[i].toSQL();
        }
    }

    // ORDER BY
    if (!orderBy_.empty()) {
        oss << " ORDER BY ";
        for (size_t i = 0; i < orderBy_.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << orderBy_[i].expr.toSQL() << (orderBy_[i].ascending ? " ASC" : " DESC");
            if (orderBy_[i].nullsDirective.has_value()) {
                oss << " NULLS " << *orderBy_[i].nullsDirective;
            }
        }
    }

    if (limit_.has_value())  oss << " LIMIT "  << *limit_;
    if (offset_.has_value()) oss << " OFFSET " << *offset_;

    return oss.str();
}

std::string SQLSelectBuilder::joinTypeToString(SQLJoinType type)
{
    switch (type) {
        case SQLJoinType::Inner: return "INNER JOIN";
        case SQLJoinType::Left:  return "LEFT JOIN";
        case SQLJoinType::Right: return "RIGHT JOIN";
        case SQLJoinType::Full:  return "FULL JOIN";
        case SQLJoinType::Cross: return "CROSS JOIN";
    }
    return "JOIN";
}

} // namespace cppsqlx
