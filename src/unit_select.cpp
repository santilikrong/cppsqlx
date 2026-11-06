#include <cppsqlx/unit_select.hpp>
#include <cppsqlx/selectquery.hpp>
#include <cppsqlx/insertquery.hpp>
#include <cppsqlx/ctasquery.hpp>
#include <cppsqlx/updatequery.hpp>
#include <cppsqlx/deletequery.hpp>

namespace cppsqlx
{

    std::string UnitSelect::toSQL() const
    {
        // Utilisation d'un switch classique ou d'un double-dispatch vis-à-vis du variant index
        // pour contourner tout bug potentiel de la structure overloads du compilateur Clang :
        switch (query.index())
        {
        case 0:
            return std::get<SelectQuery>(query).toSQL();
        case 1:
            return std::get<InsertQuery>(query).toSQL(quoted);
        case 2:
            return std::get<CTASQuery>(query).toSQL(quoted);
        case 3:
            return std::get<UpdateQuery>(query).toSQL(quoted);
        case 4:
            return std::get<DeleteQuery>(query).toSQL(quoted);
        case 5:
            return std::get<DropTableQuery>(query).toSQL(quoted);
        default:
            return "";
        }
    }

    json UnitSelect::toJSON() const
    {
        json queryJson;
        std::string queryType;

        std::visit(overloads{[&](const SelectQuery &q)
                             { queryJson = q.toJSON(); queryType = "SelectQuery"; },
                             [&](const InsertQuery &q)
                             { queryJson = q.toJSON(); queryType = "InsertQuery"; },
                             [&](const CTASQuery &q)
                             { queryJson = q.toJSON(); queryType = "CTASQuery"; },
                             [&](const UpdateQuery &q)
                             { queryJson = q.toJSON(); queryType = "UpdateQuery"; },
                             [&](const DeleteQuery &q)
                             { queryJson = q.toJSON(); queryType = "DeleteQuery"; },
                             [&](const DropTableQuery &q)
                             { queryJson = q.toJSON(); queryType = "DropTableQuery"; }},
                   query);

        return json{
            {"unit_select", {{"query_type", queryType}, {"query_data", queryJson}, {"target_type", targetTypeToString(targetType)}, {"name", name}, {"quoted", quoted}}}};
    }

    std::expected<UnitSelect, std::string> UnitSelect::fromJSON(const json &j)
    {
        const json &data = j.contains("unit_select") ? j["unit_select"] : j;
        try
        {
            UnitSelect us;
            us.name = data.at("name").get<std::string>();
            us.quoted = data.value("quoted", false);

            auto typeRes = stringToTargetType(data.at("target_type").get<std::string>());
            if (!typeRes)
                return std::unexpected("UnitSelect::fromJSON -> " + typeRes.error());
            us.targetType = *typeRes;

            std::string queryType = data.at("query_type").get<std::string>();
            const json &queryData = data.at("query_data");

            // 🟢 Comparaison stricte et tolérante à la casse pour chaque type du variant
            if (queryType == "SelectQuery" || queryType == "selectquery")
            {
                auto res = SelectQuery::fromJSON(queryData);
                if (!res)
                    return std::unexpected("UnitSelect::fromJSON (Select) -> " + res.error());
                us.query = std::move(*res);
            }
            else if (queryType == "InsertQuery" || queryType == "insertquery")
            {
                auto res = InsertQuery::fromJSON(queryData);
                if (!res)
                    return std::unexpected("UnitSelect::fromJSON (Insert) -> " + res.error());
                us.query = std::move(*res);
            }
            else if (queryType == "CTASQuery" || queryType == "ctasquery")
            {
                auto res = CTASQuery::fromJSON(queryData);
                if (!res)
                    return std::unexpected("UnitSelect::fromJSON (CTAS) -> " + res.error());
                us.query = std::move(*res);
            }
            else if (queryType == "UpdateQuery" || queryType == "updatequery")
            {
                auto res = UpdateQuery::fromJSON(queryData);
                if (!res)
                    return std::unexpected("UnitSelect::fromJSON (Update) -> " + res.error());
                us.query = std::move(*res);
            }
            else if (queryType == "DeleteQuery" || queryType == "deletequery")
            {
                auto res = DeleteQuery::fromJSON(queryData);
                if (!res)
                    return std::unexpected("UnitSelect::fromJSON (Delete) -> " + res.error());
                us.query = std::move(*res);
            }
            else if (queryType == "DropTableQuery" || queryType == "droptablequery")
            {
                auto res = DropTableQuery::fromJSON(queryData);
                if (!res)
                    return std::unexpected("UnitSelect::fromJSON (DropTable) -> " + res.error());
                us.query = std::move(*res);
            }
            else
            {
                return std::unexpected("UnitSelect::fromJSON -> Type de requête inconnu: " + queryType);
            }

            return us;
        }
        catch (const std::exception &e)
        {
            return std::unexpected(std::string("UnitSelect::fromJSON failed: ") + e.what());
        }
    }

} // namespace cppsqlx
