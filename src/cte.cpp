
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

} // namespace cppsqlx
