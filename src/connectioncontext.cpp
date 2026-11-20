#include <cppsqlx/connectioncontext.hpp>

namespace cppsqlx
{

json ConnectionContext::toJSON() const
{
    return json{
        {"context", {
            {"name", name_},
            {"description", description_}
        }}
    };
}

std::expected<ConnectionContext, std::string> ConnectionContext::fromJSON(const json& j)
{
    const json& data = j.contains("context") ? j["context"] : j;
    try {
        ConnectionContext cc;
        cc.name_ = data.at("name").get<std::string>();
        cc.description_ = data.value("description", "");
        return cc;
    }
    catch (const std::exception& e) {
        return std::unexpected(std::string("ConnectionContext::fromJSON failed: ") + e.what());
    }
}

} // namespace cppsqlx
