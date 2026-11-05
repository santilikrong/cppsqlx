#include <cppsqlx/physicalconnection.hpp>

namespace cppsqlx
{

json PhysicalConnection::toJSON() const
{
    return json{
        {"physicalconnection", {
            {"name", name_},
            {"dbname", dbname_},
            {"schema", schema_},
            {"host", dbhost_},
            {"port", dbport_},
            {"user", dbuser_},
            {"password", dbpassword_}
        }}
    };
}

std::expected<PhysicalConnection, std::string> PhysicalConnection::fromJSON(const json& j)
{
    const json& data = j.contains("physicalconnection") ? j["physicalconnection"] : j;
    try {
        PhysicalConnection pc;
        pc.name_ = data.at("name").get<std::string>();
        pc.dbname_ = data.at("dbname").get<std::string>();
        pc.schema_ = data.value("schema", "");
        pc.dbhost_ = data.at("host").get<std::string>();
        pc.dbport_ = data.at("port").get<int>();
        pc.dbuser_ = data.at("user").get<std::string>();
        pc.dbpassword_ = data.at("password").get<std::string>();
        return pc;
    }
    catch (const std::exception& e) {
        return std::unexpected(std::string("PhysicalConnection::fromJSON failed: ") + e.what());
    }
}

} // namespace cppsqlx
