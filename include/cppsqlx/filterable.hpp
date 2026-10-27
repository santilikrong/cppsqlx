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

#ifndef CPPSQLX_FILTERABLE_H
#define CPPSQLX_FILTERABLE_H

#include <cppsqlx/sqlexpression.hpp>
#include <string>

namespace cppsqlx {

/**
 * @brief Base template for query classes that support WHERE clauses.
 * Uses CRTP (Curiously Recurring Template Pattern) to return derived type for fluent chaining.
 */
template <typename T>
class filterable {
protected:
    SQLExpression wherexpr_;

public:
    filterable() = default;

    // Cast operators for CRTP
    operator T&() { return static_cast<T&>(*this); }
    operator const T&() const { return static_cast<const T&>(*this); }

    /**
     * @brief Add WHERE clause using SQLExpression.
     * @param expr SQLExpression representing the condition.
     * @return Reference to derived type for chaining.
     */
    T& where(const SQLExpression& expr) {
        wherexpr_ = expr;
        return static_cast<T&>(*this);
    }

    /**
     * @brief Add WHERE clause using raw SQL string.
     * @param expr Raw SQL condition string.
     * @return Reference to derived type for chaining.
     */
    T& where(const std::string& expr) {
        wherexpr_ = SQLExpression(expr);
        return static_cast<T&>(*this);
    }

    /**
     * @brief Get the WHERE expression.
     * @return const reference to SQLExpression.
     */
    const SQLExpression& whereExpr() const {
        return wherexpr_;
    }
};

} // namespace cppsqlx

#endif // CPPSQLX_FILTERABLE_H