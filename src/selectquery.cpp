
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
#include "cppsqlx/cte.hpp"

#include <sstream>
#include <stdexcept>

namespace cppsqlx
{

    static inline std::string quoteIdent(const std::string &ident)
    {
        std::string out;
        out.reserve(ident.size() + 2);
        out.push_back('"');

        for (char c : ident)
        {
            if (c == '"')
            {
                out.push_back('"');
            }
            out.push_back(c);
        }

        out.push_back('"');
        return out;
    }

    static inline std::string qualify(const std::optional<std::string> &schema,
                                      const std::string &table)
    {
        return (schema && !schema->empty())
                   ? quoteIdent(*schema) + "." + quoteIdent(table)
                   : quoteIdent(table);
    }

    // SELECT
    SelectQuery &SelectQuery::select(const SQLExpression &expr)
    {
        selectionSet_.add(expr);
        return *this;
    }

    SelectQuery &SelectQuery::select(const std::string &rawExpr)
    {
        selectionSet_.add(rawExpr);
        return *this;
    }

    SelectQuery &SelectQuery::select(const SQLSelectionSet &selectionSet)
    {
        for (const auto &expr : selectionSet.expressions())
        {
            selectionSet_.add(expr);
        }
        return *this;
    }

    // GROUP BY
    SelectQuery &SelectQuery::groupBy(const SQLExpression &expr)
    {
        groupBy_.push_back(expr);
        return *this;
    }

    SelectQuery &SelectQuery::groupBy(const SQLSelectionSet &selectionSet)
    {
        for (const auto &expr : selectionSet.expressions())
        {
            groupBy_.push_back(expr);
        }
        return *this;
    }

    // HAVING
    SelectQuery &SelectQuery::having(const SQLExpression &expr)
    {
        having_ = expr;
        return *this;
    }

    // ORDER BY
    SelectQuery &SelectQuery::orderBy(const SQLExpression &expr)
    {
        addOrderBy(expr);
        return *this;
    }

    SelectQuery &SelectQuery::orderBy(const Order &order)
    {
        addOrderBy(order);
        return *this;
    }

    // FROM
    SelectQuery &SelectQuery::from(const Dataset &ds)
    {
        schema_ = ds.schema();
        table_ = ds.name();
        alias_ = ds.alias();
        from_.reset();
        return *this;
    }

    SelectQuery &SelectQuery::from(std::shared_ptr<Joinable> ds, std::string alias)
    {
        if (!ds)
        {
            throw std::invalid_argument("Joinable source is null");
        }

        if (auto subquery = std::dynamic_pointer_cast<SelectQuery>(ds))
        {
            ctes_.emplace_back(alias, subquery);
            table_ = alias;
            from_.reset();
        }
        else
        {
            from_ = ds;
            alias_ = alias;
        }

        return *this;
    }

    SelectQuery &SelectQuery::from(std::string alias)
    {
        auto it = std::find_if(ctes_.begin(), ctes_.end(), [alias](const CTE &cte)
                               { return cte.name() == alias; });
        if (ctes_.end() != it)
            table_ = alias;

        return *this;
    }

    SelectQuery &SelectQuery::join(std::string joinType,
                                   std::shared_ptr<Joinable> ds,
                                   std::optional<std::string> alias,
                                   std::optional<SQLExpression> on)
    {
        if (!ds)
            throw std::invalid_argument("Joinable target is null");
        if (joinType.empty())
            throw std::invalid_argument("joinType must not be empty");

        if (auto subquery = std::dynamic_pointer_cast<SelectQuery>(ds))
        {
            if (alias && !alias->empty())
            {
                ctes_.emplace_back(*alias, subquery);
                // Pour une sous-requête, on enregistre le JOIN avec l'alias,
                // mais on passe nullptr pour 'target' dans joins_ pour indiquer à toSQL
                // qu'il faut utiliser le nom du CTE et non réécrir le SELECT.
                joins_.push_back({joinType, nullptr, alias, on});
            }
            else
            {
                throw std::invalid_argument("A subquery in a JOIN clause must have an alias");
            }
        }
        else
        {
            // Cas d'une table normale (Dataset) : on stocke le ds et son alias normalement
            joins_.push_back({joinType, ds, alias, on});
        }

        return *this;
    }

    SelectQuery &SelectQuery::join(std::string joinType,
                                   std::shared_ptr<Joinable> ds,
                                   std::optional<std::string> alias,
                                   const SQLJoinExpression &onCondition)
    {
        return join(joinType, ds, alias, SQLExpression(onCondition.toSQL()));
    }

    SelectQuery &SelectQuery::as(std::string_view alias)
    {
        alias_ = std::string(alias);
        return *this;
    }

    SelectQuery &SelectQuery::with(const CTE &cte)
    {
        ctes_.push_back(cte);
        return *this;
    }

    SelectQuery &SelectQuery::with(std::string name, std::shared_ptr<SelectQuery> query)
    {
        ctes_.emplace_back(std::move(name), std::move(query));
        return *this;
    }

    SelectQuery &SelectQuery::where(const SQLExpression &expr)
    {
        where_ = expr;
        return *this;
    }

    SelectQuery &SelectQuery::limit(size_t n)
    {
        limit_ = n;
        return *this;
    }

    SelectQuery &SelectQuery::offset(size_t n)
    {
        offset_ = n;
        return *this;
    }

    std::string SelectQuery::joinidentifier() const
    {
        return "(" + toSQL() + ")";
    }

    std::string SelectQuery::toSQL() const
    {
        std::ostringstream sql;

        // Fonction d'aide locale pour appliquer ou non les guillemets
        auto formatIdent = [this](const std::string &ident) -> std::string
        {
            return quoted_ ? SQLExpression::quoteIdentifier(ident) : ident;
        };

        // WITH
        if (!ctes_.empty())
        {
            sql << "WITH ";
            for (size_t i = 0; i < ctes_.size(); ++i)
            {
                // Utilisation du flag pour le nom du CTE
                sql << formatIdent(ctes_[i].name())
                    << " AS (" << ctes_[i].query().toSQL() << ")";
                if (i + 1 < ctes_.size())
                {
                    sql << ", ";
                }
            }
            sql << " ";
        }

        // SELECT
        sql << "SELECT " << (selectionSet_.empty() ? "*" : selectionSet_.toSQL(quoted_));

        if (!table_.empty() || from_)
        {
            if (selectionSet_.empty())
            {
                throw std::runtime_error("No FROM source specified nor selectionset");
            }

            // FROM
            sql << " FROM ";
            if (from_)
            {
                sql << from_->joinidentifier();
                if (alias_ && !alias_->empty())
                {
                    // Utilisation du flag pour l'alias du FROM
                    sql << " AS " << formatIdent(*alias_);
                }
            }
            else
            {
                // Utilisation du flag pour le nom de la table
                sql << formatIdent(table_);
            }

            // JOINs
            for (const auto &j : joins_)
            {
                sql << " " << j.joinType << " ";

                if (j.target)
                {
                    sql << j.target->joinidentifier();
                    if (j.alias && !j.alias->empty())
                    {
                        // Utilisation du flag pour l'alias du JOIN physique
                        sql << " AS " << formatIdent(*j.alias);
                    }
                }
                else if (j.alias)
                {
                    // Utilisation du flag pour le nom du CTE joint
                    sql << formatIdent(*j.alias);
                }

                if (j.on)
                {
                    sql << " ON " << j.on->toSQL(quoted_);
                }
            }

            if (where_)
            {
                sql << " WHERE " << where_->toSQL();
            }

            if (!groupBy_.empty())
            {
                sql << " GROUP BY ";
                for (size_t i = 0; i < groupBy_.size(); ++i)
                {
                    sql << groupBy_[i].toSQL();
                    if (i + 1 < groupBy_.size())
                    {
                        sql << ", ";
                    }
                }
            }

            if (having_)
            {
                sql << " HAVING " << having_->toSQL();
            }

            if (!orderBy_.empty())
            {
                sql << " ORDER BY ";
                for (size_t i = 0; i < orderBy_.size(); ++i)
                {
                    sql << orderBy_[i].expr.toSQL()
                        << (orderBy_[i].dir == SortDirection::ASC ? " ASC" : " DESC");
                    if (i + 1 < orderBy_.size())
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
        }

        return sql.str();
    }

    Resultset SelectQuery::run(std::shared_ptr<Connection> /*connection*/)
    {
        throw std::runtime_error("Not implemented for demo");
    }

    // Helpers
    void SelectQuery::addSelection(const SQLExpression &expr)
    {
        selectionSet_.add(expr);
    }

    void SelectQuery::addSelection(const std::string &rawExpr)
    {
        selectionSet_.add(rawExpr);
    }

    void SelectQuery::addGroupBy(const SQLExpression &expr)
    {
        groupBy_.push_back(expr);
    }

    void SelectQuery::addGroupBy(const std::string &rawExpr)
    {
        groupBy_.push_back(SQLExpression(rawExpr));
    }

    void SelectQuery::addOrderBy(const SQLExpression &expr)
    {
        orderBy_.push_back(Order(expr, SortDirection::ASC));
    }

    void SelectQuery::addOrderBy(const std::string &rawExpr)
    {
        orderBy_.push_back(Order(SQLExpression(rawExpr), SortDirection::ASC));
    }

    void SelectQuery::addOrderBy(const Order &order)
    {
        orderBy_.push_back(order);
    }

} // namespace cppsqlx
