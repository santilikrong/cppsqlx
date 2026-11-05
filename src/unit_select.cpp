#include <cppsqlx/unit_select.hpp>

namespace cppsqlx
{

    std::string UnitSelect::toSQL() const
    {
        return std::visit(overloads{[this](const SelectQuery &q)
                                    { return q.toSQL(); },
                                    [this](const auto &q)
                                    { return q.toSQL(quoted); },
                                    [this](const DeleteQuery &q)
                                    { return q.toSQL(quoted); }},
                          query);
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
                             { queryJson = q.toJSON(); queryType = "DeleteQuery"; }},
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

            if (queryType == "SelectQuery")
            {
                auto res = SelectQuery::fromJSON(queryData);
                if (!res)
                    return std::unexpected("UnitSelect::fromJSON (Select) -> " + res.error());
                us.query = std::move(*res);
            }
            else if (queryType == "InsertQuery")
            {
                auto res = InsertQuery::fromJSON(queryData);
                if (!res)
                    return std::unexpected("UnitSelect::fromJSON (Insert) -> " + res.error());
                us.query = std::move(*res);
            }
            else if (queryType == "CTASQuery")
            {
                auto res = CTASQuery::fromJSON(queryData);
                if (!res)
                    return std::unexpected("UnitSelect::fromJSON (CTAS) -> " + res.error());
                us.query = std::move(*res);
            }
            else if (queryType == "UpdateQuery")
            {
                auto res = UpdateQuery::fromJSON(queryData);
                if (!res)
                    return std::unexpected("UnitSelect::fromJSON (Update) -> " + res.error());
                us.query = std::move(*res);
            }
            else if (queryType == "DeleteQuery")
            {
                auto res = DeleteQuery::fromJSON(queryData);
                if (!res)
                    return std::unexpected("UnitSelect::fromJSON (Delete) -> " + res.error());
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
