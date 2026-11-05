#ifndef CPPSQLX_CTAS_QUERY_HPP
#define CPPSQLX_CTAS_QUERY_HPP

#include <string>
#include <memory>
#include <optional>
#include <expected>
#include <cppsqlx/json_concepts.hpp>
#include <cppsqlx/selectquery.hpp>

namespace cppsqlx
{

class CTASQuery
{
private:
    std::optional<std::string> schema_;
    std::string table_;
    bool isTemporary_ = false;
    std::shared_ptr<SelectQuery> selectQuery_;
    bool quoted_ = false;

public:
    CTASQuery() = default;
    explicit CTASQuery(std::string table) : table_(std::move(table)) {}
    ~CTASQuery() = default;

    // ---------- Builder Fluent API ----------
    CTASQuery& createTable(std::string table);
    CTASQuery& schema(std::string schema);
    CTASQuery& setTemporary(bool isTemp) noexcept { isTemporary_ = isTemp; return *this; }
    CTASQuery& setQuoted(bool quoted) noexcept { quoted_ = quoted; return *this; }
    CTASQuery& select(SelectQuery query);

    // ---------- SQL Generation ----------
    [[nodiscard]] std::string toSQL() const { return toSQL(quoted_); }
    [[nodiscard]] std::string toSQL(bool quoted) const;

    // ---------- JSON Concepts Contract ----------
    [[nodiscard]] json toJSON() const;
    static std::expected<CTASQuery, std::string> fromJSON(const json& j);
};

} // namespace cppsqlx

#endif // CPPSQLX_CTAS_QUERY_HPP
