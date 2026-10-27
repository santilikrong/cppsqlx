
/*
MIT License

Copyright (c) 2025 Y-HRAH KRONG Santili

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#include "cppsqlx/selectquery.hpp"
#include "cppsqlx/statement.hpp"
#include "cppsqlx/resultset.hpp"
#include "cppsqlx/join.hpp"

#include <sstream>
#include <stdexcept>

namespace cppsqlx
{

SelectQuery::SelectQuery() = default;

SelectQuery::SelectQuery(std::string tableName)
    : table_(std::move(tableName))
{
}

SelectQuery::SelectQuery(std::string schemaName, std::string tableName)
    : schema_(std::move(schemaName)), table_(std::move(tableName))
{
}

SelectQuery::~SelectQuery() = default;

std::string SelectQuery::toSQL() const
{
    std::ostringstream sql;
    sql << "SELECT ";
    if (distinct_)
    {
        sql << "DISTINCT ";
    }
    sql << (selectionSet_.empty() ? "*" : selectionSet_.toSQL());

    sql << " FROM ";
    if (schema_)
    {
        sql << *schema_ << ".";
    }
    sql << table_;
    if (tableAlias_ && !tableAlias_->empty())
    {
        sql << " AS " << *tableAlias_;
    }

    for (const auto& j : joins_)
    {
        sql << " " << j.toSQL();
    }

    if (!wherexpr_.toSQL().empty())
    {
        sql << " WHERE " << wherexpr_.toSQL();
    }

    if (!groupings_.empty())
    {
        sql << " GROUP BY ";
        for (size_t i = 0; i < groupings_.size(); ++i)
        {
            sql << groupings_[i].toSQL();
            if (i < groupings_.size() - 1)
            {
                sql << ", ";
            }
        }
    }

    if (having_)
    {
        sql << " HAVING " << having_->toSQL();
    }

    if (!orderings_.empty())
    {
        sql << " ORDER BY ";
        for (size_t i = 0; i < orderings_.size(); ++i)
        {
            sql << orderings_[i].first.toSQL() << " "
                << (orderings_[i].second == SortOrder::Asc ? "ASC" : "DESC");
            if (i < orderings_.size() - 1)
            {
                sql << ", ";
            }
        }
    }

    if (limit_)
    {
        sql << " LIMIT " << *limit_;
    }
    if (offset_)
    {
        sql << " OFFSET " << *offset_;
    }

    return sql.str();
}

Resultset SelectQuery::run(std::shared_ptr<Statement> stmt)
{
    const auto sql = toSQL();
    auto resOrErr = stmt->execute(sql);
    if (!resOrErr)
    {
        throw std::runtime_error("SelectQuery::run(Statement): " + resOrErr.error());
    }
    return std::move(resOrErr.value());
}

// Fluent API implementations
SelectQuery& SelectQuery::distinct(bool enable)
{
    distinct_ = enable;
    return *this;
}

SelectQuery& SelectQuery::select(const SQLSelectionSet& selectionSet)
{
    selectionSet_ = selectionSet;
    return *this;
}

SelectQuery& SelectQuery::addSelect(const SQLExpression& expr)
{
    selectionSet_.add(expr);
    return *this;
}

SelectQuery& SelectQuery::addSelect(const std::string& rawExpr)
{
    selectionSet_.add(rawExpr);
    return *this;
}

SelectQuery& SelectQuery::clearSelect()
{
    selectionSet_ = SQLSelectionSet{};
    return *this;
}

SelectQuery& SelectQuery::from(std::string tableName)
{
    table_ = std::move(tableName);
    return *this;
}

SelectQuery& SelectQuery::from(std::string schemaName, std::string tableName)
{
    schema_ = std::move(schemaName);
    table_ = std::move(tableName);
    return *this;
}

SelectQuery& SelectQuery::as(std::string alias)
{
    tableAlias_ = std::move(alias);
    return *this;
}

SelectQuery& SelectQuery::andWhere(const SQLExpression& predicate)
{
    if (!wherexpr_.toSQL().empty())
    {
        wherexpr_ = wherexpr_ && predicate;
    }
    else
    {
        wherexpr_ = predicate;
    }
    return *this;
}

SelectQuery& SelectQuery::orWhere(const SQLExpression& predicate)
{
    if (!wherexpr_.toSQL().empty())
    {
        wherexpr_ = wherexpr_ || predicate;
    }
    else
    {
        wherexpr_ = predicate;
    }
    return *this;
}

SelectQuery& SelectQuery::clearWhere()
{
    wherexpr_ = SQLExpression{};
    return *this;
}

const SQLExpression& SelectQuery::whereExpr() const
{
    return wherexpr_;
}

SelectQuery& SelectQuery::groupBy(const std::vector<SQLExpression>& groupings)
{
    groupings_ = groupings;
    return *this;
}

SelectQuery& SelectQuery::addGroupBy(const SQLExpression& grouping)
{
    groupings_.push_back(grouping);
    return *this;
}

SelectQuery& SelectQuery::clearGroupBy()
{
    groupings_.clear();
    return *this;
}

SelectQuery& SelectQuery::having(const SQLExpression& predicate)
{
    having_ = predicate;
    return *this;
}

SelectQuery& SelectQuery::clearHaving()
{
    having_.reset();
    return *this;
}

SelectQuery& SelectQuery::orderBy(const std::vector<std::pair<SQLExpression, SortOrder>>& orderings)
{
    orderings_ = orderings;
    return *this;
}

SelectQuery& SelectQuery::addOrderBy(const SQLExpression& expr, SortOrder order)
{
    orderings_.emplace_back(expr, order);
    return *this;
}

SelectQuery& SelectQuery::clearOrderBy()
{
    orderings_.clear();
    return *this;
}

SelectQuery& SelectQuery::limit(std::optional<std::size_t> n)
{
    limit_ = n;
    return *this;
}

SelectQuery& SelectQuery::offset(std::optional<std::size_t> n)
{
    offset_ = n;
    return *this;
}

SelectQuery& SelectQuery::join(const Dataset& rightDs, JoinType type)
{
    Join j(Dataset(*this));
    j.to(rightDs).setType(type);
    joins_.push_back(std::move(j));
    return *this;
}

SelectQuery& SelectQuery::on(const SQLExpression& condition)
{
    if (!joins_.empty())
    {
        joins_.back().on(condition);
    }
    return *this;
}

// Accessors
bool SelectQuery::isDistinct() const
{
    return distinct_;
}

const std::optional<std::string>& SelectQuery::schema() const
{
    return schema_;
}

const std::string& SelectQuery::table() const
{
    return table_;
}

const std::optional<std::string>& SelectQuery::tableAlias() const
{
    return tableAlias_;
}

const SQLSelectionSet& SelectQuery::selectionSet() const
{
    return selectionSet_;
}

const std::vector<Join>& SelectQuery::joins() const
{
    return joins_;
}

const std::vector<SQLExpression>& SelectQuery::groupings() const
{
    return groupings_;
}

const std::optional<SQLExpression>& SelectQuery::havingClause() const
{
    return having_;
}

const std::vector<std::pair<SQLExpression, SortOrder>>& SelectQuery::orderings() const
{
    return orderings_;
}

const std::optional<std::size_t>& SelectQuery::limitValue() const
{
    return limit_;
}

const std::optional<std::size_t>& SelectQuery::offsetValue() const
{
    return offset_;
}

} // namespace cppsqlx
