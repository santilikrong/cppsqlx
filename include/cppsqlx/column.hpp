
/*
 * MIT License
 * Copyright (c) 2025 cppsqlx
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * MIT License
 * Copyright (c) 2025 cppsqlx
 */

/*
 * MIT License
 * Copyright (c) 2025 cppsqlx
 */

#ifndef CPPSQLX_COLUMN_HPP
#define CPPSQLX_COLUMN_HPP

#include <string>
#include <sstream>
#include <expected>
#include <algorithm>
#include <cctype>

namespace cppsqlx {

/**
 * @file column.hpp
 * @brief Definition of the cppsqlx::Column class.
 */

/// @brief Internal helpers for validation and case-insensitive checks.
namespace detail {

    /// @brief Validate identifier: non-empty and only letters, digits, or underscore.
    inline bool isIdentifierValid(const std::string& id) noexcept 
    {
        auto islet = [] (char c) {  return std::isalnum(c) != 0 || c == '_'; };
        return !id.empty() &&
               std::all_of(id.begin(), id.end(), islet);
    }

    /// @brief Convert string to lowercase.
    inline std::string toLower(std::string s) 
    {
        std::transform(s.begin(), s.end(), s.begin(), [](char c){ return static_cast<char>(std::tolower(c)); } );
        return s;
    }

    /// @brief Case-insensitive substring check.
    inline bool containsCaseInsensitive(const std::string& haystack, const std::string& needle) 
    {
        auto h = toLower(haystack);
        auto n = toLower(needle);
        return h.find(n) != std::string::npos;
    }
}

/**
 * @class Column
 * @brief Represents a PostgreSQL-compatible column with extended metadata and explicit error handling.
 */
class Column {
    friend class DatasetFactory;
public:
    /**
     * @brief Factory method to create a Column with validation.
     */
    static std::expected<Column, std::string> create(
        std::string name, std::string sqlType,
        bool isNullable = true, bool unique = false,
        std::string defaultValue = "",
        int size = 0, int precision = 0, int scale = 0, int position = -1)
    {
        if (!detail::isIdentifierValid(name)) {
            return std::unexpected("Column: invalid name");
        }
        if (sqlType.empty()) {
            return std::unexpected("Column: type cannot be empty");
        }
        if (size < 0 || precision < 0 || scale < 0) {
            return std::unexpected("Column: size/precision/scale cannot be negative");
        }
        return Column(std::move(name), std::move(sqlType), isNullable, unique,
                      std::move(defaultValue), size, precision, scale, position);
    }

    // ---------- Accessors ----------
    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    [[nodiscard]] const std::string& type() const noexcept { return sqlType_; }
    [[nodiscard]] bool isNullable() const noexcept { return isnullable_; }
    [[nodiscard]] bool isUnique() const noexcept { return unique_; }
    [[nodiscard]] const std::string& defaultValue() const noexcept { return defaultValue_; }
    [[nodiscard]] int size() const noexcept { return size_; }
    [[nodiscard]] int precision() const noexcept { return precision_; }
    [[nodiscard]] int scale() const noexcept { return scale_; }
    [[nodiscard]] int position() const noexcept { return position_; }

    /**
     * @brief Generate SQL fragment for column definition (exception-free).
     */
    [[nodiscard]] std::expected<std::string, std::string> asSQL() const {
        if (!detail::isIdentifierValid(name_)) {
            return std::unexpected("Column: invalid name");
        }

        std::ostringstream sql;
        sql << name_ << " " << sqlType_;

        const bool hasParenInType = sqlType_.find('(') != std::string::npos;

        // Append size (for CHAR/VARCHAR) if not already present in type string.
        if (!hasParenInType && size_ > 0 &&
            (detail::containsCaseInsensitive(sqlType_, "char") ||
             detail::containsCaseInsensitive(sqlType_, "varchar")))
        {
            sql << "(" << size_ << ")";
        }

        // Append precision/scale (for DECIMAL/NUMERIC) if not already present.
        if (!hasParenInType &&
            (detail::containsCaseInsensitive(sqlType_, "decimal") ||
             detail::containsCaseInsensitive(sqlType_, "numeric")))
        {
            if (precision_ > 0) {
                sql << "(" << precision_;
                if (scale_ >= 0) sql << "," << scale_;
                sql << ")";
            }
        }

        if (!isnullable_) sql << " NOT NULL";
        if (unique_)      sql << " UNIQUE";
        if (!defaultValue_.empty()) sql << " DEFAULT " << defaultValue_;

        return sql.str();
    }

private:
    Column(std::string name, std::string sqlType, bool isNullable, bool unique,
           std::string defaultValue, int size, int precision, int scale, int position)
        : name_(std::move(name)), sqlType_(std::move(sqlType)), isnullable_(isNullable),
          unique_(unique), defaultValue_(std::move(defaultValue)),
          size_(size), precision_(precision), scale_(scale), position_(position) {}

    std::string name_;
    std::string sqlType_;
    bool        isnullable_;
    bool        unique_;
    std::string defaultValue_;
    int         size_;
    int         precision_;
    int         scale_;
    int         position_;
};

} // namespace cppsqlx

#endif // CPPSQLX_COLUMN_HPP
