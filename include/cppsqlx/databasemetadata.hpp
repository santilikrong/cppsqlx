#ifndef CPPSQLX_DATABASE_METADATA_HPP
#define CPPSQLX_DATABASE_METADATA_HPP

#include <string>
#include <vector>
#include <expected>
#include <memory>
#include <cppsqlx/connection.hpp>

namespace cppsqlx
{

struct ColumnMetadata {
    std::string name;
    std::string dataType;
    bool isNullable;
    bool isPrimaryKey;
};

struct ObjectMetadata {
    std::string name;
    std::string type; // "BASE TABLE" ou "VIEW"
};

class DatabaseMetadata
{
private:
    std::shared_ptr<Connection> connection_;

public:
    explicit DatabaseMetadata(std::shared_ptr<Connection> conn) : connection_(std::move(conn)) {}
    ~DatabaseMetadata() = default;

    [[nodiscard]] std::expected<std::vector<std::string>, std::string> getSchemas() const;
    [[nodiscard]] std::expected<std::vector<std::string>, std::string> getTables(const std::string& schema = "public") const;
    [[nodiscard]] std::expected<std::vector<std::string>, std::string> getViews(const std::string& schema = "public") const;
    [[nodiscard]] std::expected<std::vector<ColumnMetadata>, std::string> getTableStructure(const std::string& tableName, const std::string& schema = "public") const;
};

} // namespace cppsqlx

#endif // CPPSQLX_DATABASE_METADATA_HPP
