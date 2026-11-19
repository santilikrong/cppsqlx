
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
#include <cppsqlx/joinable_factory.hpp>

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

    // Le helper interne récursif qui collecte les CTEs à plat
    std::string SelectQuery::toSQLInternal(std::vector<CollectedCTE> &collectedCtes) const
    {
        // 1. Collecter les CTEs locaux définis sur cette instance précise
        for (const auto &cte : ctes_)
        {
            auto it = std::find_if(collectedCtes.begin(), collectedCtes.end(),
                                   [&cte](const CollectedCTE &c) { return c.name == cte.name(); });

            if (it == collectedCtes.end())
            {
                std::string subQuerySql = cte.query().toSQLInternal(collectedCtes);
                collectedCtes.push_back({cte.name(), subQuerySql});
            }
        }

        // 2. Collecter récursivement les CTEs cachés dans les clauses de JOIN (y compris les sous-requêtes UNION)
        for (const auto &j : joins_)
        {
            if (j.target)
            {
                if (auto subquery = std::dynamic_pointer_cast<SelectQuery>(j.target))
                {
                    std::string subSql = subquery->toSQLInternal(collectedCtes);
                    if (subquery->hasSetOperations())
                    {
                        std::string cteName = subquery->alias_.value_or(subquery->joinidentifier());
                        auto it = std::find_if(collectedCtes.begin(), collectedCtes.end(),
                                               [&cteName](const CollectedCTE &c) { return c.name == cteName; });
                        if (it == collectedCtes.end())
                        {
                            collectedCtes.push_back({cteName, subSql});
                        }
                    }
                }
            }
        }


        // 3. Générer le corps de la requête SELECT courante (sans son propre WITH local)
        std::ostringstream sql;

        auto formatIdent = [this](const std::string &ident) -> std::string
        {
            return quoted_ ? SQLExpression::quoteIdentifier(ident) : ident;
        };

        sql << "SELECT " << (selectionSet_.empty() ? "*" : selectionSet_.toSQL(quoted_));

        if (!table_.empty() || from_)
        {
            if (selectionSet_.empty())
            {
                throw std::runtime_error("No FROM source specified nor selectionset");
            }

            sql << " FROM ";
            if (from_)
            {
                if (auto subFrom = std::dynamic_pointer_cast<SelectQuery>(from_))
                {
                    std::string subSql = subFrom->toSQLInternal(collectedCtes);
                    if (subFrom->hasSetOperations())
                    {
                        std::string cteName = subFrom->alias_.value_or(subFrom->joinidentifier());
                        auto it = std::find_if(collectedCtes.begin(), collectedCtes.end(),
                                               [&cteName](const CollectedCTE &c) { return c.name == cteName; });
                        if (it == collectedCtes.end())
                        {
                            collectedCtes.push_back({cteName, subSql});
                        }
                        sql << formatIdent(cteName);
                    }
                    else
                    {
                        sql << from_->joinidentifier();
                    }
                }
                else
                {
                    sql << from_->joinidentifier();
                }

                if (alias_ && !alias_->empty())
                {
                    sql << " AS " << formatIdent(*alias_);
                }
            }
            else
            {
                if (schema_ && !schema_->empty())
                {
                    sql << formatIdent(*schema_) << ".";
                }
                sql << formatIdent(table_);
                if (alias_ && !alias_->empty())
                {
                    sql << " AS " << formatIdent(*alias_);
                }
            }

            // Clause JOINs
            for (const auto &j : joins_)
            {
                sql << " " << j.joinType << " ";

                if (j.target)
                {
                    sql << j.target->joinidentifier();
                    if (j.alias && !j.alias->empty())
                    {
                        sql << " AS " << formatIdent(*j.alias);
                    }
                }
                else if (j.alias)
                {
                    sql << formatIdent(*j.alias);
                }

                if (j.on)
                {
                    sql << " ON " << j.on->toSQL(quoted_);
                }
            }

            if (where_)
                sql << " WHERE " << where_->toSQL();

            if (!groupBy_.empty())
            {
                sql << " GROUP BY ";
                for (size_t i = 0; i < groupBy_.size(); ++i)
                {
                    sql << groupBy_[i].toSQL();
                    if (i + 1 < groupBy_.size())
                        sql << ", ";
                }
            }

            if (having_)
                sql << " HAVING " << having_->toSQL();

            if (!orderBy_.empty())
            {
                sql << " ORDER BY ";
                for (size_t i = 0; i < orderBy_.size(); ++i)
                {
                    sql << orderBy_[i].expr.toSQL()
                        << (orderBy_[i].dir == SortDirection::ASC ? " ASC" : " DESC");
                    if (i + 1 < orderBy_.size())
                        sql << ", ";
                }
            }

            if (limit_)
                sql << " LIMIT " << *limit_;
            if (offset_)
                sql << " OFFSET " << *offset_;
        }

        // 4. Traitement des opérations ensemblistes (UNION ALL, UNION, INTERSECT, EXCEPT)
        if (!setOperations_.empty())
        {
            std::ostringstream compound;

            bool wrapFirst = (!orderBy_.empty() || limit_.has_value() || offset_.has_value());
            if (wrapFirst)
                compound << "(\n" << sql.str() << "\n)";
            else
                compound << sql.str();

            for (const auto &branch : setOperations_)
            {
                if (!branch.query)
                    continue;

                // Remonte également les CTEs définies dans les branches suivantes vers collectedCtes
                std::string branchSql = branch.query->toSQLInternal(collectedCtes);

                compound << "\n" << toString(branch.op) << "\n";

                bool wrapBranch = (!branch.query->orderBy_.empty() ||
                                   branch.query->limit_.has_value() ||
                                   branch.query->offset_.has_value() ||
                                   branch.query->hasSetOperations());
                if (wrapBranch)
                    compound << "(\n" << branchSql << "\n)";
                else
                    compound << branchSql;
            }

            if (!unionOrderBy_.empty())
            {
                compound << "\nORDER BY ";
                for (size_t i = 0; i < unionOrderBy_.size(); ++i)
                {
                    compound << unionOrderBy_[i].expr.toSQL()
                             << (unionOrderBy_[i].dir == SortDirection::ASC ? " ASC" : " DESC");
                    if (i + 1 < unionOrderBy_.size())
                        compound << ", ";
                }
            }

            if (unionLimit_.has_value())
                compound << "\nLIMIT " << *unionLimit_;
            if (unionOffset_.has_value())
                compound << "\nOFFSET " << *unionOffset_;

            return compound.str();
        }

        return sql.str();
    }

    // 🟢 CORRECTION : La méthode publique devient le point d'entrée unique qui injecte le WITH global
    std::string SelectQuery::toSQL() const
    {
        std::vector<CollectedCTE> collectedCtes;
        
        // 1. Générer le corps principal et collecter tous les CTEs à plat
        std::string mainBody = toSQLInternal(collectedCtes);
        
        std::ostringstream finalSql;
        auto formatIdent = [this](const std::string &ident) -> std::string {
            return quoted_ ? SQLExpression::quoteIdentifier(ident) : ident;
        };

        // 2. Émettre la clause WITH unique si des CTEs ont été collectés
        if (!collectedCtes.empty())
        {
            finalSql << "WITH ";
            for (size_t i = 0; i < collectedCtes.size(); ++i)
            {
                finalSql << formatIdent(collectedCtes[i].name) << " AS (" << collectedCtes[i].sql << ")";
                if (i + 1 < collectedCtes.size())
                {
                    finalSql << ", ";
                }
            }
            finalSql << " ";
        }

        // 3. Concaténer la requête principale
        finalSql << mainBody;
        return finalSql.str();
    }


    json SelectQuery::toJSON() const
    {

        json j;

        // 1. Informations de la Table principale et du Schéma
        if (schema_.has_value())
        {
            j["schema"] = schema_.value();
        }

        if (!table_.empty())
        {
            j["table"] = table_;
        }
        else if (from_)
        {
            j["table"] = from_->joinidentifier();
        }
        else
        {
            j["table"] = "";
        }

        if (alias_.has_value())
        {
            j["alias"] = alias_.value();
        }


        // 2. Clause SELECT (utilise le toJSON de SQLSelectionSet)
        j["select"] = selectionSet_.toJSON();

        // 3. Clause WHERE (si présente)
        if (where_.has_value())
        {
            j["where"] = where_.value().toJSON();
        }

        // 4. Clause JOINS
        if (!joins_.empty())
        {
            json jsonJoins = json::array();
            for (const auto &item : joins_)
            {
                json itemJson;
                itemJson["type"] = item.joinType;
                if (item.alias.has_value())
                {
                    itemJson["alias"] = item.alias.value();
                }
                if (item.on.has_value())
                {
                    itemJson["on"] = item.on.value().toJSON();
                }
                // Note: Si target possède un moyen d'être sérialisé, vous pouvez l'ajouter ici
                itemJson["target_identifier"] = item.target ? item.target->joinidentifier() : "";
                jsonJoins.push_back(itemJson);
            }
            j["joins"] = jsonJoins;
        }

        // 5. Clause GROUP BY
        if (!groupBy_.empty())
        {
            json jsonGroupBy = json::array();
            for (const auto &expr : groupBy_)
            {
                jsonGroupBy.push_back(expr.toJSON());
            }
            j["groupby"] = jsonGroupBy;
        }

        // 6. Clause HAVING
        if (having_.has_value())
        {
            j["having"] = having_.value().toJSON();
        }

        // 7. Clause ORDER BY
        if (!orderBy_.empty())
        {
            json jsonOrderBy = json::array();
            for (const auto &order : orderBy_)
            {
                json orderJson;
                orderJson["expression"] = order.expr.toJSON();
                orderJson["direction"] = (order.dir == SortDirection::ASC) ? "ASC" : "DESC";
                jsonOrderBy.push_back(orderJson);
            }
            j["orderby"] = jsonOrderBy;
        }

        // 8. LIMIT & OFFSET
        if (limit_.has_value())
        {
            j["limit"] = limit_.value();
        }
        if (offset_.has_value())
        {
            j["offset"] = offset_.value();
        }

        // 9. CTEs (Common Table Expressions)
        if (!ctes_.empty())
        {
            json jsonCtes = json::array();
            for (const auto &cte : ctes_)
            {
                jsonCtes.push_back(cte.toJSON());
            }
            j["ctes"] = jsonCtes;
        }

        // 10. Opérations ensemblistes (UNION ALL, UNION, INTERSECT, EXCEPT)
        if (!setOperations_.empty())
        {
            json jsonSetOps = json::array();
            for (const auto &branch : setOperations_)
            {
                if (!branch.query)
                    continue;
                json opJson;
                opJson["operator"] = toString(branch.op);
                opJson["query"] = branch.query->toJSON();
                jsonSetOps.push_back(std::move(opJson));
            }
            j["set_operations"] = std::move(jsonSetOps);
        }

        // 11. Clauses globales post-UNION (ORDER BY / LIMIT / OFFSET)
        if (!unionOrderBy_.empty())
        {
            json jsonUnionOrderBy = json::array();
            for (const auto &order : unionOrderBy_)
            {
                json orderJson;
                orderJson["expression"] = order.expr.toJSON();
                orderJson["direction"] = (order.dir == SortDirection::ASC) ? "ASC" : "DESC";
                jsonUnionOrderBy.push_back(std::move(orderJson));
            }
            j["union_orderby"] = std::move(jsonUnionOrderBy);
        }

        if (unionLimit_.has_value())
        {
            j["union_limit"] = unionLimit_.value();
        }
        if (unionOffset_.has_value())
        {
            j["union_offset"] = unionOffset_.value();
        }


        // Enrobage dans un objet racine "selectquery" pour la cohérence
        return json{{"selectquery", j}};
    }

    std::expected<SelectQuery, std::string> SelectQuery::fromJSON(const json &j)
    {
        // Supporte le format avec ou sans clé racine "selectquery"
        const json &data = j.contains("selectquery") ? j["selectquery"] : j;

        try
        {
            std::string table = data.at("table").get<std::string>();
            SelectQuery query;

            // 1. Schéma et Table principale
            if (data.contains("schema") && !data["schema"].get<std::string>().empty())
            {
                query.schema_ = data["schema"].get<std::string>();
            }
            query.table_ = table;

            if (data.contains("alias") && !data["alias"].get<std::string>().empty())
            {
                query.alias_ = data["alias"].get<std::string>();
            }

            // 2. Clause SELECT (utilise SQLSelectionSet::fromJSON)
            if (data.contains("select"))
            {
                auto selRes = SQLSelectionSet::fromJSON(data["select"]);
                if (!selRes)
                    return std::unexpected("SelectQuery::fromJSON -> " + selRes.error());
                query.selectionSet_ = std::move(*selRes);
            }

            // 3. Clause WHERE
            if (data.contains("where") && !data["where"].is_null())
            {
                auto whereRes = SQLExpression::fromJSON(data["where"]);
                if (!whereRes)
                    return std::unexpected("SelectQuery::fromJSON -> " + whereRes.error());
                query.where_ = std::move(*whereRes);
            }

            // 4. Clause JOINS
            if (data.contains("joins") && data["joins"].is_array())
            {
                for (const auto &joinJson : data["joins"])
                {
                    JoinItem item;
                    item.joinType = joinJson.at("type").get<std::string>();

                    if (joinJson.contains("alias") && !joinJson["alias"].is_null())
                    {
                        item.alias = joinJson["alias"].get<std::string>();
                    }

                    if (joinJson.contains("on") && !joinJson["on"].is_null())
                    {
                        auto onRes = SQLExpression::fromJSON(joinJson["on"]);
                        if (!onRes)
                            return std::unexpected("SelectQuery::fromJSON (join ON) -> " + onRes.error());
                        item.on = std::move(*onRes);
                    }

                    // 💡 Reconstruction polymorphe de la cible du JOIN via la Factory
                    // Reconstruction polymorphe de la cible du JOIN via la Factory ou target_identifier
                    if (joinJson.contains("target") && !joinJson["target"].is_null())
                    {
                        auto targetRes = JoinableFactory::fromJSON(joinJson["target"]);
                        if (!targetRes)
                            return std::unexpected("SelectQuery::fromJSON (join target) -> " + targetRes.error());
                        item.target = std::move(*targetRes);
                    }
                    else if (joinJson.contains("target_identifier") && !joinJson["target_identifier"].get<std::string>().empty())
                    {
                        std::string targetIdent = joinJson["target_identifier"].get<std::string>();
                        std::string schemaName = "";
                        std::string tableName = targetIdent;

                        if (auto dotPos = targetIdent.find('.'); dotPos != std::string::npos)
                        {
                            schemaName = targetIdent.substr(0, dotPos);
                            tableName = targetIdent.substr(dotPos + 1);
                        }

                        Dataset ds(tableName);
                        if (!schemaName.empty())
                        {
                            ds.setSchema(schemaName);
                        }
                        item.target = std::make_shared<Dataset>(std::move(ds));
                    }


                    query.joins_.push_back(std::move(item));
                }
            }

            // 5. Clause GROUP BY
            if (data.contains("groupby") && data["groupby"].is_array())
            {
                for (const auto &gbJson : data["groupby"])
                {
                    auto gbRes = SQLExpression::fromJSON(gbJson);
                    if (!gbRes)
                        return std::unexpected("SelectQuery::fromJSON (groupby) -> " + gbRes.error());
                    query.groupBy_.push_back(std::move(*gbRes));
                }
            }

            // 6. Clause HAVING
            if (data.contains("having") && !data["having"].is_null())
            {
                auto havingRes = SQLExpression::fromJSON(data["having"]);
                if (!havingRes)
                    return std::unexpected("SelectQuery::fromJSON (having) -> " + havingRes.error());
                query.having_ = std::move(*havingRes);
            }

            // 7. Clause ORDER BY
            if (data.contains("orderby") && data["orderby"].is_array())
            {
                for (const auto &obJson : data["orderby"])
                {
                    auto exprRes = SQLExpression::fromJSON(obJson.at("expression"));
                    if (!exprRes)
                        return std::unexpected("SelectQuery::fromJSON (orderby expr) -> " + exprRes.error());

                    std::string dirStr = obJson.at("direction").get<std::string>();
                    SortDirection dir = (dirStr == "DESC") ? SortDirection::DESC : SortDirection::ASC;

                    query.orderBy_.emplace_back(std::move(*exprRes), dir);
                }
            }

            // 8. LIMIT & OFFSET
            if (data.contains("limit") && !data["limit"].is_null())
            {
                query.limit_ = data["limit"].get<size_t>();
            }
            if (data.contains("offset") && !data["offset"].is_null())
            {
                query.offset_ = data["offset"].get<size_t>();
            }

            // 9. CTEs (Common Table Expressions)
            if (data.contains("ctes") && data["ctes"].is_array())
            {
                for (const auto &cteJson : data["ctes"])
                {
                    auto cteRes = CTE::fromJSON(cteJson);
                    if (!cteRes)
                        return std::unexpected("SelectQuery::fromJSON (cte) -> " + cteRes.error());
                    query.ctes_.push_back(std::move(*cteRes));
                }
            }

            // 10. Opérations ensemblistes (UNION ALL, UNION, INTERSECT, EXCEPT)
            if (data.contains("set_operations") && data["set_operations"].is_array())
            {
                for (const auto &opJson : data["set_operations"])
                {
                    if (!opJson.contains("query"))
                        continue;

                    auto branchRes = SelectQuery::fromJSON(opJson["query"]);
                    if (!branchRes)
                        return std::unexpected("SelectQuery::fromJSON (set_operations) -> " + branchRes.error());

                    std::string opStr = opJson.value("operator", "UNION ALL");
                    SetOperator op = SetOperator::UNION_ALL;
                    if (opStr == "UNION")
                        op = SetOperator::UNION;
                    else if (opStr == "INTERSECT")
                        op = SetOperator::INTERSECT;
                    else if (opStr == "EXCEPT")
                        op = SetOperator::EXCEPT;

                    query.setOperations_.push_back({op, std::make_shared<SelectQuery>(std::move(*branchRes))});
                }
            }

            // 11. Clauses globales post-UNION (ORDER BY / LIMIT / OFFSET)
            if (data.contains("union_orderby") && data["union_orderby"].is_array())
            {
                for (const auto &obJson : data["union_orderby"])
                {
                    auto exprRes = SQLExpression::fromJSON(obJson.at("expression"));
                    if (!exprRes)
                        return std::unexpected("SelectQuery::fromJSON (union_orderby expr) -> " + exprRes.error());

                    std::string dirStr = obJson.at("direction").get<std::string>();
                    SortDirection dir = (dirStr == "DESC") ? SortDirection::DESC : SortDirection::ASC;

                    query.unionOrderBy_.emplace_back(std::move(*exprRes), dir);
                }
            }

            if (data.contains("union_limit") && !data["union_limit"].is_null())
            {
                query.unionLimit_ = data["union_limit"].get<size_t>();
            }
            if (data.contains("union_offset") && !data["union_offset"].is_null())
            {
                query.unionOffset_ = data["union_offset"].get<size_t>();
            }


            return query;
        }
        catch (const std::exception &e)
        {
            return std::unexpected(std::string("SelectQuery::fromJSON failed: ") + e.what());
        }
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

    SelectQuery &SelectQuery::unionAll(const SelectQuery &other)
    {
        setOperations_.push_back({SetOperator::UNION_ALL, std::make_shared<SelectQuery>(other)});
        return *this;
    }

    SelectQuery &SelectQuery::unionAll(std::shared_ptr<SelectQuery> other)
    {
        if (other)
            setOperations_.push_back({SetOperator::UNION_ALL, std::move(other)});
        return *this;
    }

    SelectQuery &SelectQuery::unionDistinct(const SelectQuery &other)
    {
        setOperations_.push_back({SetOperator::UNION, std::make_shared<SelectQuery>(other)});
        return *this;
    }

    SelectQuery &SelectQuery::unionDistinct(std::shared_ptr<SelectQuery> other)
    {
        if (other)
            setOperations_.push_back({SetOperator::UNION, std::move(other)});
        return *this;
    }

    SelectQuery &SelectQuery::intersect(const SelectQuery &other)
    {
        setOperations_.push_back({SetOperator::INTERSECT, std::make_shared<SelectQuery>(other)});
        return *this;
    }

    SelectQuery &SelectQuery::except(const SelectQuery &other)
    {
        setOperations_.push_back({SetOperator::EXCEPT, std::make_shared<SelectQuery>(other)});
        return *this;
    }

    SelectQuery &SelectQuery::unionOrderBy(const Order &order)
    {
        unionOrderBy_.push_back(order);
        return *this;
    }

    SelectQuery &SelectQuery::unionLimit(size_t n)
    {
        unionLimit_ = n;
        return *this;
    }

    SelectQuery &SelectQuery::unionOffset(size_t n)
    {
        unionOffset_ = n;
        return *this;
    }

    std::expected<void, std::string> SelectQuery::validate() const
    {
        if (setOperations_.empty())
            return {};

        size_t expectedCols = selectionSet_.size();
        for (size_t i = 0; i < setOperations_.size(); ++i)
        {
            const auto &branch = setOperations_[i];
            if (!branch.query)
                return std::unexpected("Branche " + toString(branch.op) + " #" + std::to_string(i + 1) + " nulle.");

            auto subVal = branch.query->validate();
            if (!subVal)
                return subVal;

            if (expectedCols > 0 && branch.query->selectionSet_.size() > 0 &&
                branch.query->selectionSet_.size() != expectedCols)
            {
                return std::unexpected(
                    "Incompatibilité de colonnes dans " + toString(branch.op) +
                    " : la requête principale projette " + std::to_string(expectedCols) +
                    " colonne(s), mais la branche #" + std::to_string(i + 1) +
                    " en projette " + std::to_string(branch.query->selectionSet_.size()) + ".");
            }
        }
        return {};
    }


} // namespace cppsqlx
