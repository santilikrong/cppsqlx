#include <cppsqlx/etltask.hpp>
#include <cppsqlx/joinable_factory.hpp> // Utile pour la reconstruction si Dataset passe par une factory polymorphe

namespace cppsqlx
{

json ETLTask::toJSON() const
{
    json j;
    j["name"] = name_;
    
    // Sérialisation de la requête SelectQuery
    if (sqlselect_) {
        j["query"] = sqlselect_->toJSON();
    } else {
        j["query"] = nullptr;
    }

    // Sérialisation du Dataset cible
    if (targetds_) {
        j["target"] = targetds_->toJSON();
    } else {
        j["target"] = nullptr;
    }

    // Sérialisation de la map en tableau de connexions (pour supporter l'objet comme clé JSON)
    json jsonConnections = json::array();
    for (const auto& [datasetPtr, connectionName] : connections_) {
        if (datasetPtr) {
            json connItem;
            connItem["dataset"] = datasetPtr->toJSON();
            connItem["lname"] = connectionName;
            jsonConnections.push_back(connItem);
        }
    }
    j["connections"] = jsonConnections;

    return json{ {"task", j} };
}

std::expected<ETLTask, std::string> ETLTask::fromJSON(const json& j)
{
    const json& data = j.contains("task") ? j["task"] : j;

    try {
        ETLTask task;
        task.name_ = data.at("name").get<std::string>();

        // Reconstruction de SelectQuery
        if (data.contains("query") && !data["query"].is_null()) {
            auto queryRes = SelectQuery::fromJSON(data["query"]);
            if (!queryRes) return std::unexpected("ETLTask::fromJSON -> " + queryRes.error());
            task.sqlselect_ = std::make_shared<SelectQuery>(std::move(*queryRes));
        }

        // Reconstruction du Dataset Target
        if (data.contains("target") && !data["target"].is_null()) {
            auto dsRes = Dataset::fromJSON(data["target"]);
            if (!dsRes) return std::unexpected("ETLTask::fromJSON -> " + dsRes.error());
            task.targetds_ = std::make_shared<Dataset>(std::move(*dsRes));
        }

        // Reconstruction de la map de connexions
        if (data.contains("connections") && data["connections"].is_array()) {
            for (const auto& connJson : data["connections"]) {
                std::string lname = connJson.at("lname").get<std::string>();
                
                auto dsRes = Dataset::fromJSON(connJson.at("dataset"));
                if (!dsRes) return std::unexpected("ETLTask::fromJSON (connections) -> " + dsRes.error());
                
                auto dsPtr = std::make_shared<Dataset>(std::move(*dsRes));
                task.connections_[dsPtr] = std::move(lname);
            }
        }

        return task;
    }
    catch (const std::exception& e) {
        return std::unexpected(std::string("ETLTask::fromJSON failed: ") + e.what());
    }
}

} // namespace cppsqlx
