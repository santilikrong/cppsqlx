
/*
 * MIT License
 * Copyright (c) 2025 cppsqlx
 *
 * Permission is hereby granted, free of charge...
 * (full MIT text same as other headers)
 */

#ifndef CPPSQLX_FOREIGNKEY_HPP
#define CPPSQLX_FOREIGNKEY_HPP

#include <string>
#include <vector>
#include <sstream>
#include <expected>
#include <algorithm>
#include <cctype>

namespace cppsqlx {

/**
 * @file foreignkey.hpp
 * @brief Definition of the cppsqlx::ForeignKey class.
 */


/**
 * @class ForeignKey
 * @brief Represents a table-level FOREIGN KEY constraint (supports composite keys).
 */
class ForeignKey 
{
public:
    /**
     * @brief Factory method to create a ForeignKey.
     * @param name Constraint name (e.g., "fk_orders_user_id").
     * @param referencedTable Target table name (e.g., "users").
     * @return std::expected<ForeignKey, std::string>
     */
    static std::expected<ForeignKey, std::string> create(std::string name, std::string referencedTable) 
    {
        if (!detail::isIdentifierValid(name))            
        {
            return std::unexpected("ForeignKey: invalid name");
        };

        if (!detail::isIdentifierValid(referencedTable)) 
        {
            return std::unexpected("ForeignKey: invalid referenced table name");
        };

        return ForeignKey(std::move(name), std::move(referencedTable));
    }

    /// @brief Add a local column.
    [[nodiscard]] std::expected<void, std::string> addColumn(const std::string& col) 
    {
        if (!detail::isIdentifierValid(col)) 
        {
            return std::unexpected("ForeignKey: invalid local column name");
        };
        columns_.push_back(col);
        return {};
    }

    /// @brief Add a referenced column.
    [[nodiscard]] std::expected<void, std::string> addRefColumn(const std::string& col) 
    {
        if (!detail::isIdentifierValid(col)) 
        {
            return std::unexpected("ForeignKey: invalid referenced column name");
        };
        refColumns_.push_back(col);
        return {};
    }

    [[nodiscard]] const std::string& name() const noexcept 
    { 
        return name_; 
    }

    [[nodiscard]] const std::string& referencedTable() const noexcept 
    { 
        return refTable_; 
    }

    [[nodiscard]] const std::vector<std::string>& columns() const noexcept 
    { 
        return columns_; 
    }

    [[nodiscard]] const std::vector<std::string>& referencedColumns() const noexcept 
    { 
        return refColumns_; 
    }

    /**
     * @brief Generate SQL fragment for the FK constraint.
     * @return std::expected<std::string, std::string>
     */
    [[nodiscard]] std::expected<std::string, std::string> asSQL() const 
    {
        if (columns_.empty())    
        {
            return std::unexpected("ForeignKey: local columns must be defined");
        };

        if (refColumns_.empty()) return std::unexpected("ForeignKey: referenced columns must be defined");
        if (columns_.size() != refColumns_.size())
            return std::unexpected("ForeignKey: local and referenced columns count mismatch");

        std::ostringstream sql;
        sql << "CONSTRAINT " << name_ << " FOREIGN KEY (";
        for (std::size_t i = 0; i < columns_.size(); ++i) {
            sql << columns_[i];
            if (i + 1 < columns_.size()) sql << ", ";
        }
        sql << ") REFERENCES " << refTable_ << " (";
        for (std::size_t i = 0; i < refColumns_.size(); ++i) 
        {
            sql << refColumns_[i];
            if (i + 1 < refColumns_.size()) 
            {
                sql << ", ";
            }
        }
        sql << ")";
        return sql.str();
    }

private:
    ForeignKey(std::string name, std::string referencedTable)
        : name_(std::move(name)), refTable_(std::move(referencedTable)) {}

    std::string              name_;
    std::string              refTable_;
    std::vector<std::string> columns_;
    std::vector<std::string> refColumns_;
};

} // namespace cppsqlx

#endif // CPPSQLX_FOREIGNKEY_HPP
