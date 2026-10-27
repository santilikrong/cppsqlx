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
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef CPPSQLX_SQL_SELECTION_SET_HPP
#define CPPSQLX_SQL_SELECTION_SET_HPP

#include <cppsqlx/sqlexpression.hpp>
#include <vector>
#include <string>
#include <sstream>

namespace cppsqlx {

/**
 * @class SQLSelectionSet
 * @brief Represents a set of SQL expressions for the SELECT clause.
 */
class SQLSelectionSet {
private:
    std::vector<SQLExpression> selections_; ///< Container for SQL expressions.

public:
    SQLSelectionSet() = default;

    // Add an SQLExpression
    void add(const SQLExpression& expr) {
        selections_.push_back(expr);
    }

    // Add a raw SQL string
    void add(const std::string& rawExpr) {
        selections_.emplace_back(rawExpr);
    }

    /**
     * @brief Generate SQL string for the selection set.
     * Uses alias-aware rendering for each expression.
     * Example: col1 AS "alias1", COUNT(*) AS "total"
     */
    /**
     * @brief Generate SQL string for the selection set.
     * Uses alias-aware rendering for each expression.
     * Example: col1 AS "alias1", COUNT(*) AS "total"
     */
    std::string toSQL(bool quoted = true) const { // 💡 Ajout du paramètre quoted
        std::ostringstream oss;
        for (size_t i = 0; i < selections_.size(); ++i) {
            // 💡 On passe le flag à l'expression (voir l'étape suivante si la signature diffère)
            oss << selections_[i].toSQLWithAlias(quoted); 
            if (i < selections_.size() - 1) {
                oss << ", ";
            }
        }
        return oss.str();
    }


    size_t size() const { return selections_.size(); }
    bool empty() const { return selections_.empty(); }

    // Provide access to underlying expressions for iteration
    const std::vector<SQLExpression>& expressions() const { return selections_; }
};

} // namespace cppsqlx

#endif // CPPSQLX_SQL_SELECTION_SET_HPP
