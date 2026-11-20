#ifndef CPPSQLX_DELETE_QUERY_HPP
#define CPPSQLX_DELETE_QUERY_HPP

#include <string>
#include <optional>
#include <expected>
#include <cppsqlx/json_concepts.hpp>
#include <cppsqlx/sqlexpression.hpp>

namespace cppsqlx
{

class DeleteQuery
{
private:
    std::optional<std::string> schema_;
    std::string table_;
    std::optional<SQLExpression> whereCondition_;

public:
    DeleteQuery() = default;
    explicit DeleteQuery(std::string table) : table_(std::move(table)) {}
    ~DeleteQuery() = default;

    // ---------- Fluent Builder API ----------
    DeleteQuery& schema(std::string schemaName) { schema_ = std::move(schemaName); return *this; }
    
    DeleteQuery& where(SQLExpression condition) { 
        whereCondition_ = std::move(condition); 
        return *this; 
    }

    // ---------- SQL Generation ----------
    [[nodiscard]] std::string toSQL(bool quoted = false) const;

    // ---------- JSON Concepts Contract ----------
    [[nodiscard]] json toJSON() const;
    static std::expected<DeleteQuery, std::string> fromJSON(const json& j);
};

} // namespace cppsqlx

#endif // CPPSQLX_DELETE_QUERY_HPP
