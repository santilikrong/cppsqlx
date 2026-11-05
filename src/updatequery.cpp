#include <cppsqlx/updatequery.hpp>
#include <sstream>
#include <stdexcept>

namespace cppsqlx
{

UpdateQuery& UpdateQuery::table(std::string table)
{
    table_ = std::move(table);
    return *this;
}

UpdateQuery& UpdateQuery::schema(std::string schema)
{
    schema_ = std::move(schema);
    return *this;
}

UpdateQuery& UpdateQuery::set(std::string column, const SQLExpression& expr)
{
    updates_.emplace_back(std::move(column), expr);
    return *this;
}

UpdateQuery& UpdateQuery::set(std::string column, const std::string& rawValue)
{
    updates_.emplace_back(std::move(column), SQLExpression(rawValue));
    return *this;
}

UpdateQuery& UpdateQuery::where(const SQLExpression& expr)
{
    where_ = expr;
    return *this;
}

std::string UpdateQuery::toSQL(bool quoted) const
{
    if (table_.empty()) {
        throw std::runtime_error("UpdateQuery::toSQL() -> Target table name is not specified");
    }
    if (updates_.empty()) {
        throw std::runtime_error("UpdateQuery::toSQL() -> No SET assignments specified");
    }

    std::ostringstream sql;
    auto formatIdent = [quoted](const std::string& ident) {
        return quoted ? SQLExpression::quoteIdentifier(ident) : ident;
    };

    sql << "UPDATE ";
    if (schema_ && !schema_->empty()) {
        sql << formatIdent(*schema_) << ".";
    }
    sql << formatIdent(table_) << " SET ";

    for (size_t i = 0; i < updates_.size(); ++i) {
        sql << formatIdent(updates_[i].first) << " = " << updates_[i].second.toSQL(quoted);
        if (i + 1 < updates_.size()) {
            sql << ", ";
        }
    }

    if (where_) {
        sql << " WHERE " << where_->toSQL(quoted);
    }

    return sql.str();
}

nlohmann::json UpdateQuery::toJSON() const
{
    nlohmann::json j;
    if (schema_) j["schema"] = *schema_;
    j["table"] = table_;
    j["quoted"] = quoted_;

    nlohmann::json jsonUpdates = nlohmann::json::array();
    for (const auto& [col, expr] : updates_) {
        nlohmann::json item;
        item["column"] = col;
        item["expression"] = expr.toJSON();
        jsonUpdates.push_back(item);
    }
    j["updates"] = jsonUpdates;

    if (where_) {
        j["where"] = where_->toJSON();
    } else {
        j["where"] = nullptr;
    }

    return nlohmann::json{ {"updatequery", j} };
}

std::expected<UpdateQuery, std::string> UpdateQuery::fromJSON(const nlohmann::json& j)
{
    const nlohmann::json& data = j.contains("updatequery") ? j["updatequery"] : j;
    try {
        UpdateQuery query;
        query.table_ = data.at("table").get<std::string>();
        query.quoted_ = data.value("quoted", false);

        if (data.contains("schema") && !data["schema"].is_null()) {
            query.schema_ = data["schema"].get<std::string>();
        }

        if (data.contains("updates") && data["updates"].is_array()) {
            for (const auto& itemJson : data["updates"]) {
                std::string col = itemJson.at("column").get<std::string>();
                auto exprRes = SQLExpression::fromJSON(itemJson.at("expression"));
                if (!exprRes) return std::unexpected("UpdateQuery::fromJSON (updates) -> " + exprRes.error());
                query.updates_.emplace_back(std::move(col), std::move(*exprRes));
            }
        }

        if (data.contains("where") && !data["where"].is_null()) {
            auto whereRes = SQLExpression::fromJSON(data["where"]);
            if (!whereRes) return std::unexpected("UpdateQuery::fromJSON (where) -> " + whereRes.error());
            query.where_ = std::move(*whereRes);
        }

        return query;
    }
    catch (const std::exception& e) {
        return std::unexpected(std::string("UpdateQuery::fromJSON failed: ") + e.what());
    }
}

} // namespace cppsqlx
