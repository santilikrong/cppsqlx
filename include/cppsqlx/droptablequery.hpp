#ifndef CPPSQLX_DROP_TABLE_QUERY_HPP
#define CPPSQLX_DROP_TABLE_QUERY_HPP

#include <string>
#include <optional>
#include <expected>
#include <cppsqlx/json_concepts.hpp>

namespace cppsqlx
{

class DropTableQuery
{
private:
    std::optional<std::string> schema_;
    std::string table_;
    bool ifExists_ = true; // 🟢 Sécurisé par défaut

public:
    DropTableQuery() = default;
    explicit DropTableQuery(std::string table) : table_(std::move(table)) {}
    ~DropTableQuery() = default;

    // ---------- Fluent Builder API ----------
    DropTableQuery& schema(std::string schemaName) { schema_ = std::move(schemaName); return *this; }
    DropTableQuery& setIfExists(bool enable) noexcept { ifExists_ = enable; return *this; }

    // ---------- SQL Generation ----------
    [[nodiscard]] std::string toSQL(bool quoted = false) const;

    // ---------- JSON Concepts Contract ----------
    [[nodiscard]] json toJSON() const;
    static std::expected<DropTableQuery, std::string> fromJSON(const json& j);
};

} // namespace cppsqlx

#endif // CPPSQLX_DROP_TABLE_QUERY_HPP
