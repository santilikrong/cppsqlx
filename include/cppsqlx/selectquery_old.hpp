
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

#ifndef CPPSQLX_SELECT_QUERY_HPP
#define CPPSQLX_SELECT_QUERY_HPP

#include <string>
#include <memory>
#include <optional>
#include <vector>
#include <utility>
#include <concepts>

#include "cppsqlx/sqlquery.hpp"
#include "cppsqlx/joinable.hpp"
#include "cppsqlx/statement.hpp"
#include "cppsqlx/resultset.hpp"
#include "cppsqlx/filterable.hpp"
#include "cppsqlx/sqlexpression.hpp"
#include "cppsqlx/sqlselectionset.hpp"
#include "cppsqlx/dataset.hpp"
#include "cppsqlx/join.hpp"

namespace cppsqlx
{

enum class SortOrder
{
    Asc,
    Desc
};

// Concepts for variadic builders
template<typename T>
concept SelectArg = std::same_as<T, SQLExpression> ||
                    std::same_as<T, std::string> ||
                    std::integral<T>;

template<typename T>
concept GroupByArg = std::same_as<T, SQLExpression> ||
                     std::same_as<T, std::string>;

template<typename T>
concept OrderByArg = std::same_as<T, SQLExpression> ||
                     std::same_as<T, std::string>;

template<typename T>
concept HavingArg = std::same_as<T, SQLExpression> ||
                    std::same_as<T, std::string>;

class SelectQuery : public SQLQuery<Resultset>,
                    public Joinable,
                    public filterable<SelectQuery>
{
public:
    SelectQuery();
    explicit SelectQuery(std::string tableName);
    SelectQuery(std::string schemaName, std::string tableName);
    ~SelectQuery() override;

    SelectQuery(const SelectQuery&) = default;
    SelectQuery(SelectQuery&&) noexcept = default;
    SelectQuery& operator=(const SelectQuery&) = default;
    SelectQuery& operator=(SelectQuery&&) noexcept = default;

    // Core
    std::string toSQL() const;
    Resultset run(std::shared_ptr<Statement> stmt);

    // Fluent API
    SelectQuery& distinct(bool enable = true);

    SelectQuery& select(const SQLSelectionSet& selectionSet);
    SelectQuery& addSelect(const SQLExpression& expr);
    SelectQuery& addSelect(const std::string& rawExpr);
    SelectQuery& clearSelect();

    template<SelectArg First, SelectArg... Rest>
    SelectQuery& select(First first, Rest... rest)
    {
        if constexpr (std::same_as<First, SQLExpression>)
        {
            selectionSet_.add(first);
        }
        else if constexpr (std::same_as<First, std::string>)
        {
            selectionSet_.add(SQLExpression(first));
        }
        else if constexpr (std::integral<First>)
        {
            selectionSet_.add(SQLExpression(static_cast<int>(first)));
        }
        return select(rest...);
    }

    SelectQuery& select()
    {
        return *this;
    }

    SelectQuery& from(std::string tableName);
    SelectQuery& from(std::string schemaName, std::string tableName);
    SelectQuery& as(std::string alias);

    // WHERE
    SelectQuery& andWhere(const SQLExpression& predicate);
    SelectQuery& orWhere(const SQLExpression& predicate);
    SelectQuery& clearWhere();
    const SQLExpression& whereExpr() const;

    // GROUP BY
    SelectQuery& groupBy(const std::vector<SQLExpression>& groupings);
    SelectQuery& addGroupBy(const SQLExpression& grouping);
    SelectQuery& clearGroupBy();

    template<GroupByArg First, GroupByArg... Rest>
    SelectQuery& groupBy(First first, Rest... rest)
    {
        if constexpr (std::same_as<First, SQLExpression>)
        {
            groupings_.push_back(first);
        }
        else if constexpr (std::same_as<First, std::string>)
        {
            groupings_.push_back(SQLExpression(first));
        }
        return groupBy(rest...);
    }

    SelectQuery& groupBy()
    {
        return *this;
    }

    // HAVING
    SelectQuery& having(const SQLExpression& predicate);
    SelectQuery& clearHaving();

    template<HavingArg First, HavingArg... Rest>
    SelectQuery& having(First first, Rest... rest)
    {
        if constexpr (std::same_as<First, SQLExpression>)
        {
            having_ = having_ ? (*having_ && first) : first;
        }
        else if constexpr (std::same_as<First, std::string>)
        {
            SQLExpression expr(first);
            having_ = having_ ? (*having_ && expr) : expr;
        }
        return having(rest...);
    }

    SelectQuery& having()
    {
        return *this;
    }

    // ORDER BY
    SelectQuery& orderBy(const std::vector<std::pair<SQLExpression, SortOrder>>& orderings);
    SelectQuery& addOrderBy(const SQLExpression& expr, SortOrder order = SortOrder::Asc);
    SelectQuery& clearOrderBy();

    template<OrderByArg First, typename... Rest>
    SelectQuery& orderBy(First first, SortOrder order = SortOrder::Asc, Rest... rest)
    {
        if constexpr (std::same_as<First, SQLExpression>)
        {
            orderings_.emplace_back(first, order);
        }
        else if constexpr (std::same_as<First, std::string>)
        {
            orderings_.emplace_back(SQLExpression(first), order);
        }
        return orderBy(rest...);
    }

    SelectQuery& orderBy()
    {
        return *this;
    }

    // LIMIT / OFFSET
    SelectQuery& limit(std::optional<std::size_t> n);
    SelectQuery& offset(std::optional<std::size_t> n);

    // JOIN API
    SelectQuery& join(const Dataset& rightDs, JoinType type = JoinType::INNER);
    SelectQuery& on(const SQLExpression& condition);

    // Accessors
    bool isDistinct() const;
    const std::optional<std::string>& schema() const;
    const std::string& table() const;
    const std::optional<std::string>& tableAlias() const;

    const SQLSelectionSet& selectionSet() const;
    const std::vector<Join>& joins() const;
    const std::vector<SQLExpression>& groupings() const;
    const std::optional<SQLExpression>& havingClause() const;
    const std::vector<std::pair<SQLExpression, SortOrder>>& orderings() const;
    const std::optional<std::size_t>& limitValue() const;
    const std::optional<std::size_t>& offsetValue() const;

private:
    bool distinct_ = false;
    std::optional<std::string> schema_;
    std::string table_;
    std::optional<std::string> tableAlias_;

    SQLSelectionSet selectionSet_;
    SQLExpression wherexpr_;
    std::vector<SQLExpression> groupings_;
    std::optional<SQLExpression> having_;
    std::vector<std::pair<SQLExpression, SortOrder>> orderings_;
    std::optional<std::size_t> limit_;
    std::optional<std::size_t> offset_;
    std::vector<Join> joins_;
};

} // namespace cppsqlx

#endif // CPPSQLX_SELECT_QUERY_HPP
