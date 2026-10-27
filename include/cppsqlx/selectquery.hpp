
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
#include <string_view>
#include <vector>
#include <optional>
#include <memory>
#include <concepts>

#include "cppsqlx/sqlquery.hpp"
#include "cppsqlx/sqlexpression.hpp"
#include "cppsqlx/sqlselectionset.hpp"
#include "cppsqlx/join.hpp"
#include "cppsqlx/cte.hpp"
#include "cppsqlx/resultset.hpp"
#include "cppsqlx/connection.hpp"
#include "cppsqlx/statement.hpp"
#include "cppsqlx/joinable.hpp"
#include "cppsqlx/dataset.hpp"
#include "cppsqlx/sqljoinexpression.hpp"

namespace cppsqlx
{

    template <typename T>
    concept Selectable =
        std::same_as<T, cppsqlx::SQLExpression> ||
        std::same_as<T, std::string> ||
        std::integral<T>;

    enum class SortDirection
    {
        ASC,
        DESC
    };

    struct Order
    {
        SQLExpression expr;
        SortDirection dir;

        Order(SQLExpression e, SortDirection d)
            : expr(std::move(e)), dir(d)
        {
        }
    };

    // Convenience helpers for ORDER BY
    inline Order asc(const std::string &column)
    {
        return Order(SQLExpression(column), SortDirection::ASC);
    }

    inline Order desc(const std::string &column)
    {
        return Order(SQLExpression(column), SortDirection::DESC);
    }

    inline Order asc(const SQLExpression &expr)
    {
        return Order(expr, SortDirection::ASC);
    }

    inline Order desc(const SQLExpression &expr)
    {
        return Order(expr, SortDirection::DESC);
    }

    class SelectQuery : public SQLQuery<Resultset>, public Joinable
    {
    public:
        SelectQuery() noexcept = default;

        explicit SelectQuery(std::string_view tablename)
        {
            std::string s(tablename);
            auto dotPos = s.find('.');
            if (dotPos != std::string::npos)
            {
                schema_ = s.substr(0, dotPos);
                table_ = s.substr(dotPos + 1);
            }
        }

        SelectQuery(std::string_view schemaname, std::string_view tablename)
            : schema_(std::string(schemaname)), table_(tablename)
        {
        }

        // SelectQuery() : quoted_(true) {} // Initialisation par défaut

        // Interface fluide pour activer/désactiver les guillemets
        SelectQuery &quoted(bool enable)
        {
            quoted_ = enable;
            return *this;
        }

        // Getter optionnel si besoin à l'extérieur
        bool isQuoted() const { return quoted_; }

        // SELECT
        SelectQuery &select(const SQLExpression &expr);
        SelectQuery &select(const std::string &rawExpr);
        SelectQuery &select(const SQLSelectionSet &selectionSet);

        template <Selectable First, Selectable... Rest>
        SelectQuery &select(First &&first, Rest &&...rest)
        {
            addSelection(std::forward<First>(first));
            (addSelection(std::forward<Rest>(rest)), ...);
            return *this;
        }

        // GROUP BY
        SelectQuery &groupBy(const SQLExpression &expr);
        SelectQuery &groupBy(const SQLSelectionSet &selectionSet);

        template <Selectable First, Selectable... Rest>
        SelectQuery &groupBy(First &&first, Rest &&...rest)
        {
            addGroupBy(std::forward<First>(first));
            (addGroupBy(std::forward<Rest>(rest)), ...);
            return *this;
        }

        // HAVING
        SelectQuery &having(const SQLExpression &expr);

        // ORDER BY
        SelectQuery &orderBy(const SQLExpression &expr);

        template <Selectable First, Selectable... Rest>
        SelectQuery &orderBy(First &&first, Rest &&...rest)
        {
            addOrderBy(std::forward<First>(first));
            (addOrderBy(std::forward<Rest>(rest)), ...);
            return *this;
        }

        SelectQuery &orderBy(const Order &order);

        template <typename... Orders>
        SelectQuery &orderBy(const Order &first, const Orders &...rest)
        {
            addOrderBy(first);
            (addOrderBy(rest), ...);
            return *this;
        }

        // FROM / JOIN
        SelectQuery &from(const Dataset &ds);
        SelectQuery &from(std::shared_ptr<Joinable> ds, std::string alias);
        SelectQuery &from(std::string alias);

        SelectQuery &join(std::string joinType,
                          std::shared_ptr<Joinable> ds,
                          std::optional<std::string> alias = std::nullopt,
                          std::optional<SQLExpression> on = std::nullopt);

        // Overload for SQLJoinExpression
        SelectQuery &join(std::string joinType,
                          std::shared_ptr<Joinable> ds,
                          std::optional<std::string> alias,
                          const SQLJoinExpression &onCondition);

        SelectQuery &as(std::string_view alias);

        // CTE
        SelectQuery &with(const CTE &cte);
        SelectQuery &with(std::string name, std::shared_ptr<SelectQuery> query);

        // Other clauses
        SelectQuery &where(const SQLExpression &expr);
        SelectQuery &limit(size_t n);
        SelectQuery &offset(size_t n);

        [[nodiscard]] std::string toSQL() const override;
        Resultset run(std::shared_ptr<Connection> connection) override;
        [[nodiscard]] std::string joinidentifier() const override;

    private:
        std::optional<std::string> schema_;
        std::string table_;
        std::optional<std::string> alias_;
        std::shared_ptr<Joinable> from_{};

        SQLSelectionSet selectionSet_;
        bool quoted_ = false;

        struct JoinItem
        {
            std::string joinType;
            std::shared_ptr<Joinable> target;
            std::optional<std::string> alias;
            std::optional<SQLExpression> on;
        };

        std::vector<JoinItem> joins_;

        std::vector<CTE> ctes_;
        std::optional<SQLExpression> where_;
        std::vector<SQLExpression> groupBy_;
        std::optional<SQLExpression> having_;
        std::vector<Order> orderBy_;
        std::optional<size_t> limit_;
        std::optional<size_t> offset_;

        // Helpers for SELECT
        void addSelection(const SQLExpression &expr);
        void addSelection(const std::string &rawExpr);

        template <std::integral T>
        void addSelection(T value)
        {
            selectionSet_.add(SQLExpression(static_cast<int>(value)));
        }

        // Helpers for GROUP BY
        void addGroupBy(const SQLExpression &expr);
        void addGroupBy(const std::string &rawExpr);

        template <std::integral T>
        void addGroupBy(T value)
        {
            groupBy_.push_back(SQLExpression(static_cast<int>(value)));
        }

        // Helpers for ORDER BY
        void addOrderBy(const SQLExpression &expr);
        void addOrderBy(const std::string &rawExpr);

        template <std::integral T>
        void addOrderBy(T value)
        {
            orderBy_.push_back(Order(SQLExpression(static_cast<int>(value)), SortDirection::ASC));
        }

        void addOrderBy(const Order &order);
    };

} // namespace cppsqlx

#endif // CPPSQLX_SELECT_QUERY_HPP
