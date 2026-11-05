#ifndef CPPSQLX_DELETE_QUERY_HPP
#define CPPSQLX_DELETE_QUERY_HPP

#include <string>
#include <optional>
#include <expected>
#include <nlohmann/json.hpp>
#include <cppsqlx/sqlexpression.hpp>
#include <cppsqlx/json_concepts.hpp>

namespace cppsqlx
{

class DeleteQuery
{
private:
    std::optional<std::string> schema_;
    std::string table_;
    std::optional<SQLExpression> where_;
    bool quoted_ = false;

public:
    DeleteQuery() = default;
    explicit DeleteQuery(std::string table) : table_(std::move(table)) {}
    ~DeleteQuery() = default;

    // ---------- Builder Fluent API ----------
    DeleteQuery& from(std::string table);
    DeleteQuery& schema(std::string schema);
    DeleteQuery& where(const SQLExpression& expr);
    DeleteQuery& setQuoted(bool quoted) noexcept { quoted_ = quoted; return *this; }

    // ---------- SQL Generation ----------
    [[nodiscard]] std::string toSQL() const { return toSQL(quoted_); }
    [[nodiscard]] std::string toSQL(bool quoted) const;

    // ---------- JSON Concepts Contract ----------
    [[nodiscard]] nlohmann::json toJSON() const;
    static std::expected<DeleteQuery, std::string> fromJSON(const nlohmann::json& j);
};

} // namespace cppsqlx

#endif // CPPSQLX_DELETE_QUERY_HPP
