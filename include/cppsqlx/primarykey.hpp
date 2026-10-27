
/*
 * MIT License
 * Copyright (c) 2025 cppsqlx
 *
 * Permission is hereby granted, free of charge...
 * (full MIT text same as other headers)
 */

#ifndef CPPSQLX_PRIMARYKEY_HPP
#define CPPSQLX_PRIMARYKEY_HPP

#include <string>
#include <vector>
#include <sstream>
#include <expected>
#include <algorithm>
#include <cctype>

namespace cppsqlx {

/**
 * @class PrimaryKey
 * @brief Represents a table-level PRIMARY KEY constraint, with ordered columns.
 */
class PrimaryKey {
public:
    /**
     * @brief Factory method to create a PrimaryKey.
     * @param name Constraint name (e.g., "pk_users").
     * @param table Table name the PK belongs to.
     * @return std::expected<PrimaryKey, std::string>
     */
    static std::expected<PrimaryKey, std::string> create(std::string name, std::string table) 
    {
        if (!detail::isIdentifierValid(name))  
        {
            return std::unexpected("PrimaryKey: invalid name");
        }

        if (!detail::isIdentifierValid(table)) 
        {
            return std::unexpected("PrimaryKey: invalid table name");
        }

        return PrimaryKey(std::move(name), std::move(table));
    }

    /// @brief Add a column to the PK (keeps order).
    [[nodiscard]] std::expected<void, std::string> addColumn(const std::string& col) {
        if (!detail::isIdentifierValid(col)) return std::unexpected("PrimaryKey: invalid column name");
        columns_.push_back(col);
        return {};
    }

    [[nodiscard]] const std::string& name() const noexcept 
    { 
        return name_; 
    }

    [[nodiscard]] const std::string& tableName() const noexcept 
    { 
        return tablename_; 
    }

    [[nodiscard]] const std::vector<std::string>& columns() const noexcept 
    { 
        return columns_; 
    }

    [[nodiscard]] bool isDefined() const noexcept 
    { 
        return !columns_.empty(); 
    }

    [[nodiscard]] bool isComposite() const noexcept 
    { 
        return columns_.size() > 1; 
    }

    /**
     * @brief Generate SQL fragment for the PK constraint.
     * @return std::expected<std::string, std::string>
     */
    [[nodiscard]] std::expected<std::string, std::string> asConstraintSQL() const 
    {
        if (columns_.empty()) 
        {
            return std::unexpected("PrimaryKey: no columns defined");
        }

        std::ostringstream sql;
        sql << "CONSTRAINT " << name_ << " PRIMARY KEY (";
        for (std::size_t i = 0; i < columns_.size(); ++i) 
        {
            sql << columns_[i];
            if (i + 1 < columns_.size()) 
            {
                sql << ", ";
            }
        }
        sql << ")";
        return sql.str();
    }

private:
    PrimaryKey(std::string name, std::string table) : name_(std::move(name)), tablename_(std::move(table)) 
    {}

    std::string              name_;
    std::string              tablename_;
    std::vector<std::string> columns_;
};

} // namespace cppsqlx

#endif // CPPSQLX_PRIMARYKEY_HPP
