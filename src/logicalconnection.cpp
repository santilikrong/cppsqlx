#include <cppsqlx/logicalconnection.hpp>

namespace cppsqlx
{

json LogicalConnection::toJSON() const
{
    json jsonEnvs = json::object();
    for (const auto& [context, physicalConfig] : environments_) {
        // En JSON, la clé textuelle devient naturellement la clé de l'objet
        jsonEnvs[context] = physicalConfig.toJSON();
    }

    return json{
        {"logicalconnection", {
            {"name", name_},
            {"environments", jsonEnvs}
        }}
    };
}

/*std::expected<LogicalConnection, std::string> LogicalConnection::fromJSON(const json& j)
{
    const json& data = j.contains("logicalconnection") ? j["logicalconnection"] : j;
    try {
        LogicalConnection lc;
        lc.name_ = data.at("name").get<std::string>();

        if (data.contains("environments") && data["environments"].is_object()) {
            for (auto& [contextKey, physicalJson] : data["environments"].items()) {
                auto pcRes = PhysicalConnection::fromJSON(physicalJson);
                if (!pcRes) {
                    return std::unexpected("LogicalConnection::fromJSON (" + contextKey + ") -> " + pcRes.error());
                }
                lc.environments_.insert_or_assign(contextKey, std::move(*pcRes));
            }
        }
        return lc;
    }
    catch (const std::exception& e) {
        return std::unexpected(std::string("LogicalConnection::fromJSON failed: ") + e.what());
    }
}*/

std::expected<LogicalConnection, std::string> LogicalConnection::fromJSON(const json& j)
{
    // 1. Déballage de l'enveloppe principale de la connexion logique
    const json& data = j.contains("logicalconnection") ? j["logicalconnection"] : j;
    
    try {
        LogicalConnection logConn(""); // Ajuster selon le constructeur (ex: prend le nom ou vide)
        
        // 2. Extraction du dictionnaire d'environnements (DEV, TEST)
        if (data.contains("environments") && data["environments"].is_object()) {
            for (const auto& [envName, envJson] : data["environments"].items()) {
                
                // 🟢 UNWRAPPING CRUCIAL : Extraction de l'enveloppe "physicalconnection"
                const json& physData = envJson.contains("physicalconnection") ? envJson["physicalconnection"] : envJson;
                
                auto physRes = PhysicalConnection::fromJSON(physData);
                if (physRes) {
                    logConn.registerEnvironment(envName, *physRes);
                }
            }
        }
        return logConn;
    }
    catch (const std::exception& e) {
        return std::unexpected(std::string("LogicalConnection::fromJSON failed: ") + e.what());
    }
}


} // namespace cppsqlx
