#include <cppsqlx/logicalconnection.hpp>

namespace cppsqlx
{

json LogicalConnection::toJSON() const
{
    json jsonPairs = json::array();
    for (auto c : connections_) {
        // Extraction du tuple, appels à refresh() hérités de votre logique
        ConnectionContext ctx = std::get<0>(c);
        PhysicalConnection pconn = std::get<1>(c);
        ctx.refresh();
        pconn.refresh();

        json pairJson;
        pairJson["context"] = ctx.toJSON();
        pairJson["physicalconnection"] = pconn.toJSON();
        jsonPairs.push_back(pairJson);
    }

    return json{
        {"logicalconnection", {
            {"name", name_},
            {"connections", jsonPairs}
        }}
    };
}

std::expected<LogicalConnection, std::string> LogicalConnection::fromJSON(const json& j)
{
    const json& data = j.contains("logicalconnection") ? j["logicalconnection"] : j;
    try {
        LogicalConnection lc;
        lc.name_ = data.at("name").get<std::string>();

        if (data.contains("connections") && data["connections"].is_array()) {
            for (const auto& pairJson : data["connections"]) {
                auto ctxRes = ConnectionContext::fromJSON(pairJson.at("context"));
                auto pcRes = PhysicalConnection::fromJSON(pairJson.at("physicalconnection"));

                if (!ctxRes) return std::unexpected("LogicalConnection::fromJSON -> " + ctxRes.error());
                if (!pcRes) return std::unexpected("LogicalConnection::fromJSON -> " + pcRes.error());

                lc.connections_.emplace_back(std::move(*ctxRes), std::move(*pcRes));
            }
        }
        return lc;
    }
    catch (const std::exception& e) {
        return std::unexpected(std::string("LogicalConnection::fromJSON failed: ") + e.what());
    }
}

} // namespace cppsqlx
