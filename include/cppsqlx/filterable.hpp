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
#include <string_view>
#include <utility>

namespace cppsqlx {

/**
 * @brief Base template for query classes that support WHERE clauses.
 * Uses CRTP (Curiously Recurring Template Pattern) to return derived type for fluent chaining.
 */
template <typename T>
class filterable {
protected:
    SQLExpression wherexpr_;

    // Helpers internes (sécurisés et explicites) pour le CRTP
    T& derived() noexcept { return static_cast<T&>(*this); }
    const T& derived() const noexcept { return static_cast<const T&>(*this); }

public:
    filterable() = default;
    
    // Destructeur polymorphique non-virtuel protégé ou public selon l'usage, 
    // default est suffisant ici car filterable ne détient pas de polymorphisme dynamique.
    ~filterable() = default;

    /**
     * @brief Add WHERE clause using SQLExpression (Lvalue version - Copy).
     */
    T& where(const SQLExpression& expr) {
        wherexpr_ = expr;
        return derived();
    }

    /**
     * @brief Add WHERE clause using SQLExpression (Rvalue version - Move).
     * 🟢 AJOUT : Évite les copies coûteuses d'expressions complexes temporaires.
     */
    T& where(SQLExpression&& expr) noexcept(std::is_nothrow_move_assignable_v<SQLExpression>) {
        wherexpr_ = std::move(expr);
        return derived();
    }

    /**
     * @brief Add WHERE clause using a string view (Modern C++ replacement for raw SQL).
     * 🟢 AMÉLIORATION : std::string_view évite les allocations de std::string lors du passage de littéraux.
     */
    T& where(std::string_view expr) {
        wherexpr_ = SQLExpression(expr);
        return derived();
    }

    /**
     * @brief Add WHERE clause using raw SQL string (Rvalue version).
     * 🟢 AMÉLIORATION : Si l'utilisateur passe un std::string temporaire (ex: issu d'un sstream),
     * on le déplace efficacement.
     */
    T& where(std::string expr) noexcept(std::is_nothrow_constructible_v<SQLExpression, std::string&&>) {
        wherexpr_ = SQLExpression(std::move(expr));
        return derived();
    }

    /**
     * @brief Get the WHERE expression.
     * @return const reference to SQLExpression.
     */
    const SQLExpression& whereExpr() const noexcept {
        return wherexpr_;
    }
};

} // namespace cppsqlx

#endif // CPPSQLX_FILTERABLE_H
