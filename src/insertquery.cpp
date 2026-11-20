#include <cppsqlx/insertquery.hpp>
#include <cppsqlx/sqlexpression.hpp>
#include <sstream>
#include <stdexcept>

namespace cppsqlx
{

InsertQuery& InsertQuery::into(std::string table)
{
    table_ = std::move(table);
    return *this;
}

InsertQuery& InsertQuery::schema(std::string schema)
{
    schema_ = std::move(schema);
    return *this;
}

InsertQuery& InsertQuery::values(std::vector<std::string> rowValues)
{
    values_.push_back(std::move(rowValues));
    selectQuery_.reset(); 
    return *this;
}

InsertQuery& InsertQuery::select(SelectQuery query)
{
    selectQuery_ = std::make_shared<SelectQuery>(std::move(query));
    values_.clear(); 
    return *this;
}

std::string InsertQuery::toSQL(bool quoted) const
{
    if (table_.empty()) {
        throw std::runtime_error("InsertQuery::toSQL() -> Target table is not specified");
    }

    std::ostringstream sql;
    auto formatIdent = [quoted](const std::string& ident) {
        return quoted ? SQLExpression::quoteIdentifier(ident) : ident;
    };

    sql << "INSERT INTO ";
    if (schema_ && !schema_->empty()) {
        sql << formatIdent(*schema_) << ".";
    }
    sql << formatIdent(table_);

    if (!columns_.empty()) {
        sql << " (";
        for (size_t i = 0; i < columns_.size(); ++i) {
            sql << formatIdent(columns_[i]);
            if (i + 1 < columns_.size()) sql << ", ";
        }
        sql << ")";
    }

    if (selectQuery_) {
        // 🟢 Transmet le flag quoted à la sous-requête SelectQuery
        sql << " " << selectQuery_->toSQL(); 
    }
    else if (!values_.empty()) {
        sql << " VALUES ";
        for (size_t r = 0; r < values_.size(); ++r) {
            sql << "(";
            for (size_t c = 0; c < values_[r].size(); ++c) {
                sql << values_[r][c]; 
                if (c + 1 < values_[r].size()) sql << ", ";
            }
            sql << ")";
            if (r + 1 < values_.size()) sql << ", ";
        }
    }

    return sql.str();
}

json InsertQuery::toJSON() const
{
    json j;
    if (schema_) j["schema"] = *schema_;
    j["table"] = table_;
    j["columns"] = columns_;
    j["quoted"] = quoted_; // 🟢 Sauvegarde du flag dans le JSON

    if (selectQuery_) {
        j["select_query"] = selectQuery_->toJSON();
    } else {
        j["values"] = values_;
    }

    return json{ {"insertquery", j} };
}

std::expected<InsertQuery, std::string> InsertQuery::fromJSON(const json& j)
{
    const json& data = j.contains("insertquery") ? j["insertquery"] : j;
    try {
        InsertQuery query;
        query.table_ = data.at("table").get<std::string>();
        query.quoted_ = data.value("quoted", false); // 🟢 Restitution du flag
        
        if (data.contains("schema") && !data["schema"].is_null()) {
            query.schema_ = data["schema"].get<std::string>();
        }

        if (data.contains("columns") && data["columns"].is_array()) {
            query.columns_ = data["columns"].get<std::vector<std::string>>();
        }

        if (data.contains("select_query") && !data["select_query"].is_null()) {
            auto selectRes = SelectQuery::fromJSON(data["select_query"]);
            if (!selectRes) return std::unexpected("InsertQuery::fromJSON -> " + selectRes.error());
            query.selectQuery_ = std::make_shared<SelectQuery>(std::move(*selectRes));
        } 
        else if (data.contains("values") && data["values"].is_array()) {
            query.values_ = data["values"].get<std::vector<std::vector<std::string>>>();
        }

        return query;
    }
    catch (const std::exception& e) {
        return std::unexpected(std::string("InsertQuery::fromJSON failed: ") + e.what());
    }
}

} // namespace cppsqlx
