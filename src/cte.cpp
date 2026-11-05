
#include "cppsqlx/cte.hpp"
#include "cppsqlx/selectquery.hpp"
#include <sstream>
#include <format>

namespace cppsqlx {

CTE::~CTE() = default; 

static inline std::string quoteIdent(std::string_view ident) {
    std::string out;
    out.reserve(ident.size() + 2);
    out.push_back('"');
    for (char c : ident) {
        if (c == '"') { out.push_back('"'); }
        out.push_back(c);
    }
    out.push_back('"');
    return out;
}


std::string CTE::toSQL() const {
    return std::format("{} AS ({})", quoteIdent(name_), query_->toSQL());
}


json CTE::toJSON() const
{
    json j;
    j["name"] = name_;
    
    // Appel récursif de toJSON() sur la requête sous-jacente
    if (query_) {
        j["query"] = query_->toJSON();
    } else {
        j["query"] = nullptr;
    }

    return json{ {"cte", j} };
}


std::expected<CTE, std::string> CTE::fromJSON(const json& j)
{
    const json& data = j.contains("cte") ? j["cte"] : j;

    try {
        std::string name = data.at("name").get<std::string>();
        
        if (!data.contains("query") || data["query"].is_null()) {
            return std::unexpected("CTE::fromJSON failed: missing internal query");
        }

        // Reconstruction récursive du SelectQuery
        auto queryRes = SelectQuery::fromJSON(data["query"]);
        if (!queryRes) {
            return std::unexpected("CTE::fromJSON -> " + queryRes.error());
        }

        return CTE(std::move(name), std::make_shared<SelectQuery>(std::move(*queryRes)));
    }
    catch (const std::exception& e) {
        return std::unexpected(std::string("CTE::fromJSON failed: ") + e.what());
    }
}


} // namespace cppsqlx
