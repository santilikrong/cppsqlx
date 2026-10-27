/*
MIT License

Copyright (c) 2025 Y-HRAH KRONG Santili

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LI, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "cppsqlx/table.hpp"
#include "cppsqlx/detail.hpp" // Assuming isIdentifierValid is here
#include <sstream>
#include <algorithm>

namespace cppsqlx
{

std::expected<Table, std::string> Table::create(std::string schema, std::string name)
{
    if (schema.empty()) schema = "public";

    if (!detail::isIdentifierValid(schema))
        return std::unexpected("Table: invalid schema name");

    if (!detail::isIdentifierValid(name))
        return std::unexpected("Table: invalid table name");

    return Table(std::move(schema), std::move(name));
}

// Accessors
const std::string& Table::schema() const noexcept { return schema_; }
const std::string& Table::name() const noexcept { return name_; }
const std::optional<std::string>& Table::alias() const noexcept { return alias_; }
const std::vector<Column>& Table::columns() const noexcept { return columns_; }
const std::optional<PrimaryKey>& Table::primaryKey() const noexcept { return primarykey_; }
const std::vector<ForeignKey>& Table::foreignKeys() const noexcept { return foreignkeys_; }

// Mutators
Table& Table::as(std::string alias)
{
    alias_ = std::move(alias);
    return *this;
}

std::expected<void, std::string> Table::addColumn(const Column& col)
{
    auto exists = std::any_of(columns_.begin(), columns_.end(),
        [col](const Column c){ return c.name() == col.name(); });

    if (exists)
        return std::unexpected("Table: duplicate column name '" + col.name() + "'");

    columns_.push_back(col);
    return {};
}

std::expected<void, std::string> Table::setPrimaryKey(const PrimaryKey& pk)
{
    if (pk.tableName() != name_)
        return std::unexpected("Table: PrimaryKey table name does not match this table");

    for (const auto& colName : pk.columns())
    {
        auto found = std::any_of(columns_.begin(), columns_.end(),
            [colName](const Column c){ return c.name() == colName; });

        if (!found)
            return std::unexpected("Table: PK column '" + colName + "' does not exist in table");
    }

    primarykey_ = pk;
    return {};
}

std::expected<void, std::string> Table::addForeignKey(const ForeignKey& fk)
{
    if (fk.columns().empty() || fk.referencedColumns().empty())
        return std::unexpected("Table: FK must have local and referenced columns");

    if (fk.columns().size() != fk.referencedColumns().size())
        return std::unexpected("Table: FK local and referenced columns count mismatch");

    for (const auto& colName : fk.columns())
    {
        auto found = std::any_of(columns_.begin(), columns_.end(),
            [colName](const Column c){ return c.name() == colName; });

        if (!found)
            return std::unexpected("Table: FK local column '" + colName + "' does not exist in table");
    }

    foreignkeys_.push_back(fk);
    return {};
}

// SQL Generation
std::expected<std::string, std::string> Table::generateCreateSQL() const
{
    if (!detail::isIdentifierValid(schema_))
        return std::unexpected("Table: invalid schema name");

    if (!detail::isIdentifierValid(name_))
        return std::unexpected("Table: invalid table name");

    if (columns_.empty())
        return std::unexpected("Table: no columns defined");

    std::ostringstream sql;
    sql << "CREATE TABLE " << schema_ << "." << name_ << " (\n";

    for (std::size_t i = 0; i < columns_.size(); ++i)
    {
        auto colSql = columns_[i].asSQL();
        if (!colSql)
            return std::unexpected("Table: column SQL generation failed for '" +
                                   columns_[i].name() + "': " + colSql.error());

        sql << "  " << *colSql;
        if (i + 1 < columns_.size() || primarykey_.has_value() || !foreignkeys_.empty())
            sql << ",";
        sql << "\n";
    }

    if (primarykey_.has_value())
    {
        auto pkSql = primarykey_->asConstraintSQL();
        if (!pkSql)
            return std::unexpected("Table: primary key SQL generation failed: " + pkSql.error());

        sql << "  " << *pkSql;
        if (!foreignkeys_.empty())
            sql << ",";
        sql << "\n";
    }

    for (std::size_t i = 0; i < foreignkeys_.size(); ++i)
    {
        auto fkSql = foreignkeys_[i].asSQL();
        if (!fkSql)
            return std::unexpected("Table: foreign key SQL generation failed: " + fkSql.error());

        sql << "  " << *fkSql;
        if (i + 1 < foreignkeys_.size())
            sql << ",";
        sql << "\n";
    }

    sql << ");";
    return sql.str();
}

std::string Table::toSQL() const
{
    std::string sql = schema_ + "." + name_;
    if (alias_ && !alias_->empty())
        sql += " AS " + *alias_;
    return sql;
}

// Private constructor
Table::Table(std::string schema, std::string name)
    : schema_(std::move(schema)), name_(std::move(name)) {}

} // namespace cppsqlx
