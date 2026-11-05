#ifndef CPPSQLX_JOINABLE_FACTORY_HPP
#define CPPSQLX_JOINABLE_FACTORY_HPP

#include <memory>
#include <expected>
#include <string>
#include <nlohmann/json.hpp>
#include <cppsqlx/joinable.hpp>
#include <cppsqlx/dataset.hpp>
#include <cppsqlx/selectquery.hpp>


namespace cppsqlx
{

    class JoinableFactory
    {
    public:
        static std::expected<std::shared_ptr<Joinable>, std::string> fromJSON(const json &j)
        {
            // Recherche du bloc de données (gère si enveloppé sous "dataset" ou "selectquery")
            json data;
            if (j.contains("dataset"))
                data = j["dataset"];
            else if (j.contains("selectquery"))
                data = j["selectquery"];
            else
                data = j;

            try
            {
                // Lecture du discriminateur de type
                std::string type = data.at("object_type").get<std::string>();

                if (type == "Dataset")
                {
                    auto dsRes = Dataset::fromJSON(data);
                    if (!dsRes)
                        return std::unexpected(dsRes.error());
                    return std::make_shared<Dataset>(std::move(*dsRes));
                }
                else if (type == "SelectQuery")
                {
                    // Dès que SelectQuery::fromJSON sera écrit :
                    auto sqRes = SelectQuery::fromJSON(data);
                    if (!sqRes)
                        return std::unexpected(sqRes.error());
                    return std::make_shared<SelectQuery>(std::move(*sqRes));
                    return std::unexpected("SelectQuery::fromJSON non encore implémenté");
                }

                return std::unexpected("JoinableFactory: type inconnu '" + type + "'");
            }
            catch (const std::exception &e)
            {
                return std::unexpected(std::string("JoinableFactory failed: ") + e.what());
            }
        }
    };

} // namespace cppsqlx

#endif // CPPSQLX_JOINABLE_FACTORY_HPP
