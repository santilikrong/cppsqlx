#ifndef CPPSQLX_UPDATE_QUERY_HPP
#define CPPSQLX_UPDATE_QUERY_HPP

#include <string>
#include <vector>
#include <optional>
#include <expected>
#include <utility>
#include <cppsqlx/json_concepts.hpp>
#include <cppsqlx/sqlexpression.hpp>

namespace cppsqlx
{

class UpdateQuery
{
private:
    std::optional<std::string> schema_;
    std::string table_;
    std::vector<std::pair<std::string, SQLExpression>> updates_; // Couples (Colonne, Expression/Valeur)
    std::optional<SQLExpression> where_;
    bool quoted_ = false;

public:
    UpdateQuery() = default;
    explicit UpdateQuery(std::string table) : table_(std::move(table)) {}
    ~UpdateQuery() = default;

    // ---------- Builder Fluent API ----------
    UpdateQuery& table(std::string table);
    UpdateQuery& schema(std::string schema);
    UpdateQuery& set(std::string column, const SQLExpression& expr);
    UpdateQuery& set(std::string column, const std::string& rawValue);
    UpdateQuery& where(const SQLExpression& expr);
    UpdateQuery& setQuoted(bool quoted) noexcept { quoted_ = quoted; return *this; }

    // ---------- SQL Generation ----------
    [[nodiscard]] std::string toSQL() const { return toSQL(quoted_); }
    [[nodiscard]] std::string toSQL(bool quoted) const;

    // ---------- JSON Concepts Contract ----------
    [[nodiscard]] json toJSON() const;
    static std::expected<UpdateQuery, std::string> fromJSON(const json& j);
};

} // namespace cppsqlx

#endif // CPPSQLX_UPDATE_QUERY_HPP
