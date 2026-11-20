#ifndef CPPSQLX_INSERT_QUERY_HPP
#define CPPSQLX_INSERT_QUERY_HPP

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <expected>
#include <cppsqlx/json_concepts.hpp>
#include <cppsqlx/selectquery.hpp>

namespace cppsqlx
{

class InsertQuery
{
private:
    std::optional<std::string> schema_;
    std::string table_;
    std::vector<std::string> columns_;
    std::vector<std::vector<std::string>> values_;
    std::shared_ptr<SelectQuery> selectQuery_;
    bool quoted_ = false; // 🟢 Ajout du membre pour stocker l'état

public:
    InsertQuery() = default;
    explicit InsertQuery(std::string table) : table_(std::move(table)) {}
    ~InsertQuery() = default;

    // ---------- Builder Fluent API ----------
    InsertQuery& into(std::string table);
    InsertQuery& schema(std::string schema);
    InsertQuery& setQuoted(bool quoted) noexcept { quoted_ = quoted; return *this; } // 🟢 Setter Fluent
    
    template <typename... Args>
    InsertQuery& columns(Args&&... args) {
        (columns_.push_back(std::forward<Args>(args)), ...);
        return *this;
    }

    InsertQuery& values(std::vector<std::string> rowValues);
    InsertQuery& select(SelectQuery query);

    // ---------- SQL Generation ----------
    [[nodiscard]] std::string toSQL() const { return toSQL(quoted_); } // 🟢 Version sans paramètre
    [[nodiscard]] std::string toSQL(bool quoted) const;

    // ---------- JSON Concepts Contract ----------
    [[nodiscard]] json toJSON() const;
    static std::expected<InsertQuery, std::string> fromJSON(const json& j);
};

} // namespace cppsqlx

#endif // CPPSQLX_INSERT_QUERY_HPP
