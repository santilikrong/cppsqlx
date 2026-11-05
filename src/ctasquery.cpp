#include <cppsqlx/ctasquery.hpp>
#include <cppsqlx/sqlexpression.hpp>
#include <sstream>
#include <stdexcept>

namespace cppsqlx
{

CTASQuery& CTASQuery::createTable(std::string table)
{
    table_ = std::move(table);
    return *this;
}

CTASQuery& CTASQuery::schema(std::string schema)
{
    schema_ = std::move(schema);
    return *this;
}

CTASQuery& CTASQuery::select(SelectQuery query)
{
    selectQuery_ = std::make_shared<SelectQuery>(std::move(query));
    return *this;
}

std::string CTASQuery::toSQL(bool quoted) const
{
    if (table_.empty()) {
        throw std::runtime_error("CTASQuery::toSQL() -> Target table name is not specified");
    }
    if (!selectQuery_) {
        throw std::runtime_error("CTASQuery::toSQL() -> Underlying SelectQuery is missing");
    }

    std::ostringstream sql;
    auto formatIdent = [quoted](const std::string& ident) {
        return quoted ? SQLExpression::quoteIdentifier(ident) : ident;
    };

    sql << "CREATE ";
    if (isTemporary_) {
        sql << "TEMPORARY ";
    }
    sql << "TABLE ";

    if (schema_ && !schema_->empty()) {
        sql << formatIdent(*schema_) << ".";
    }
    sql << formatIdent(table_) << " AS ";

    // Propagation du flag 'quoted' à la sous-requête interne
    sql << selectQuery_->toSQL();

    return sql.str();
}

json CTASQuery::toJSON() const
{
    json j;
    if (schema_) j["schema"] = *schema_;
    j["table"] = table_;
    j["is_temporary"] = isTemporary_;
    j["quoted"] = quoted_;

    if (selectQuery_) {
        j["select_query"] = selectQuery_->toJSON();
    } else {
        j["select_query"] = nullptr;
    }

    return json{ {"ctasquery", j} };
}

std::expected<CTASQuery, std::string> CTASQuery::fromJSON(const json& j)
{
    const json& data = j.contains("ctasquery") ? j["ctasquery"] : j;
    try {
        CTASQuery query;
        query.table_ = data.at("table").get<std::string>();
        query.isTemporary_ = data.value("is_temporary", false);
        query.quoted_ = data.value("quoted", false);
        
        if (data.contains("schema") && !data["schema"].is_null()) {
            query.schema_ = data["schema"].get<std::string>();
        }

        if (data.contains("select_query") && !data["select_query"].is_null()) {
            auto selectRes = SelectQuery::fromJSON(data["select_query"]);
            if (!selectRes) return std::unexpected("CTASQuery::fromJSON -> " + selectRes.error());
            query.selectQuery_ = std::make_shared<SelectQuery>(std::move(*selectRes));
        }

        return query;
    }
    catch (const std::exception& e) {
        return std::unexpected(std::string("CTASQuery::fromJSON failed: ") + e.what());
    }
}

} // namespace cppsqlx
