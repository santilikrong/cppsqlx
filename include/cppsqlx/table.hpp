
/*
 * MIT License
 * Copyright (c) 2025 cppsqlx
 *
 * Permission is hereby granted, free of charge...
 * (full MIT text same as other headers)
 */

#ifndef CPPSQLX_TABLE_HPP
#define CPPSQLX_TABLE_HPP

#include "cppsqlx/column.hpp"
#include "cppsqlx/primarykey.hpp"
#include "cppsqlx/foreignkey.hpp"

#include <string>
#include <vector>
#include <optional>
#include <expected>

namespace cppsqlx
{

class Table
{
public:
    static std::expected<Table, std::string> create(std::string schema, std::string name);

    // Accessors
    [[nodiscard]] const std::string& schema() const noexcept;
    [[nodiscard]] const std::string& name() const noexcept;
    [[nodiscard]] const std::optional<std::string>& alias() const noexcept;
    [[nodiscard]] const std::vector<Column>& columns() const noexcept;
    [[nodiscard]] const std::optional<PrimaryKey>& primaryKey() const noexcept;
    [[nodiscard]] const std::vector<ForeignKey>& foreignKeys() const noexcept;

    // Mutators
    Table& as(std::string alias);
    [[nodiscard]] std::expected<void, std::string> addColumn(const Column& col);
    [[nodiscard]] std::expected<void, std::string> setPrimaryKey(const PrimaryKey& pk);
    [[nodiscard]] std::expected<void, std::string> addForeignKey(const ForeignKey& fk);

    // SQL Generation
    [[nodiscard]] std::expected<std::string, std::string> generateCreateSQL() const;
    std::string toSQL() const;

private:
    Table(std::string schema, std::string name);

    std::string                schema_;
    std::string                name_;
    std::optional<std::string> alias_;
    std::vector<Column>        columns_;
    std::optional<PrimaryKey>  primarykey_;
    std::vector<ForeignKey>    foreignkeys_;
};

} // namespace cppsqlx

#endif // CPPSQLX_TABLE_HPP
