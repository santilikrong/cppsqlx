#include <cppsqlx/deletequery.hpp>
#include <sstream>

namespace cppsqlx
{

std::string DeleteQuery::toSQL([[maybe_unused]] bool quoted) const
{
    if (table_.empty()) {
        return "";
    }

    std::ostringstream sql;
    sql << "DELETE FROM ";

    if (schema_ && !schema_->empty()) {
        sql << *schema_ << ".";
    }
    sql << table_;

    // 🟢 CORRECTION 1 : Appel de la méthode interne .toSQL() de SQLExpression
    if (whereCondition_) {
        sql << " WHERE " << whereCondition_->toSQL();
    }
    
    sql << ";";
    return sql.str();
}

json DeleteQuery::toJSON() const
{
    // 🟢 CORRECTION 2 : Idem pour la sérialisation JSON
    return json{
        {"deletequery", {
            {"table", table_},
            {"schema", schema_.value_or("")},
            {"where", whereCondition_ ? whereCondition_->toSQL() : ""}
        }}
    };
}

std::expected<DeleteQuery, std::string> DeleteQuery::fromJSON(const json& j)
{
    const json& data = j.contains("deletequery") ? j["deletequery"] : j;
    try {
        DeleteQuery q(data.at("table").get<std::string>());
        
        std::string sch = data.value("schema", "");
        if (!sch.empty()) {
            q.schema_ = sch;
        }

        std::string wh = data.value("where", "");
        if (!wh.empty()) {
            q.whereCondition_ = SQLExpression(wh);
        }

        return q;
    }
    catch (const std::exception& e) {
        return std::unexpected(std::string("DeleteQuery::fromJSON failed: ") + e.what());
    }
}

} // namespace cppsqlx
