#include <cppsqlx/droptablequery.hpp>
#include <sstream>

namespace cppsqlx
{

std::string DropTableQuery::toSQL([[maybe_unused]] bool quoted) const
{
    if (table_.empty()) {
        return "";
    }

    std::ostringstream sql;
    sql << "DROP TABLE ";
    if (ifExists_) {
        sql << "IF EXISTS ";
    }

    if (schema_ && !schema_->empty()) {
        sql << *schema_ << ".";
    }
    sql << table_ << ";";

    return sql.str();
}

json DropTableQuery::toJSON() const
{
    return json{
        {"droptablequery", {
            {"table", table_},
            {"schema", schema_.value_or("")},
            {"if_exists", ifExists_}
        }}
    };
}

std::expected<DropTableQuery, std::string> DropTableQuery::fromJSON(const json& j)
{
    const json& data = j.contains("droptablequery") ? j["droptablequery"] : j;
    try {
        DropTableQuery q(data.at("table").get<std::string>());
        q.ifExists_ = data.value("if_exists", true);
        
        std::string sch = data.value("schema", "");
        if (!sch.empty()) {
            q.schema_ = sch;
        }
        return q;
    }
    catch (const std::exception& e) {
        return std::unexpected(std::string("DropTableQuery::fromJSON failed: ") + e.what());
    }
}

} // namespace cppsqlx
