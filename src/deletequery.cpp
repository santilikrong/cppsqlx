#include <cppsqlx/deletequery.hpp>
#include <sstream>
#include <stdexcept>

namespace cppsqlx
{

DeleteQuery& DeleteQuery::from(std::string table)
{
    table_ = std::move(table);
    return *this;
}

DeleteQuery& DeleteQuery::schema(std::string schema)
{
    schema_ = std::move(schema);
    return *this;
}

DeleteQuery& DeleteQuery::where(const SQLExpression& expr)
{
    where_ = expr;
    return *this;
}

std::string DeleteQuery::toSQL(bool quoted) const
{
    if (table_.empty()) {
        throw std::runtime_error("DeleteQuery::toSQL() -> Target table name is not specified");
    }

    std::ostringstream sql;
    auto formatIdent = [quoted](const std::string& ident) {
        return quoted ? SQLExpression::quoteIdentifier(ident) : ident;
    };

    sql << "DELETE FROM ";
    if (schema_ && !schema_->empty()) {
        sql << formatIdent(*schema_) << ".";
    }
    sql << formatIdent(table_);

    if (where_) {
        sql << " WHERE " << where_->toSQL(quoted);
    }

    return sql.str();
}

nlohmann::json DeleteQuery::toJSON() const
{
    nlohmann::json j;
    if (schema_) j["schema"] = *schema_;
    j["table"] = table_;
    j["quoted"] = quoted_;

    if (where_) {
        j["where"] = where_->toJSON();
    } else {
        j["where"] = nullptr;
    }

    return nlohmann::json{ {"deletequery", j} };
}

std::expected<DeleteQuery, std::string> DeleteQuery::fromJSON(const nlohmann::json& j)
{
    const nlohmann::json& data = j.contains("deletequery") ? j["deletequery"] : j;
    try {
        DeleteQuery query;
        query.table_ = data.at("table").get<std::string>();
        query.quoted_ = data.value("quoted", false);

        if (data.contains("schema") && !data["schema"].is_null()) {
            query.schema_ = data["schema"].get<std::string>();
        }

        if (data.contains("where") && !data["where"].is_null()) {
            auto whereRes = SQLExpression::fromJSON(data["where"]);
            if (!whereRes) return std::unexpected("DeleteQuery::fromJSON (where) -> " + whereRes.error());
            query.where_ = std::move(*whereRes);
        }

        return query;
    }
    catch (const std::exception& e) {
        return std::unexpected(std::string("DeleteQuery::fromJSON failed: ") + e.what());
    }
}

} // namespace cppsqlx
