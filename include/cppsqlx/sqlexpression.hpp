#ifndef SQLEXPRESSION_HPP
#define SQLEXPRESSION_HPP

#include <string>
#include <string_view>
#include <vector>
#include <initializer_list>
#include <utility>
#include <cstdint>

namespace cppsqlx
{

class SQLExpression
{
public:
    SQLExpression() = default;
    
    // 🟢 OPTIMISATION : Prise en charge des chaînes par déplacement (Move Semantics)
    explicit SQLExpression(std::string expr) noexcept;
    
    // 🟢 AJOUT : Permet à operator""_sqlx d'éviter toute allocation sur le tas (Heap allocation)
    explicit SQLExpression(std::string_view expr);
    
    // 🟢 AMÉLIORATION : Passage à int64_t pour éviter le troncage des grands entiers issus de operator""_sql
    explicit SQLExpression(int64_t value);
    explicit SQLExpression(int value); // Conservé pour la rétrocompatibilité si nécessaire

    // 🟢 OPTIMISATION : Passage par valeur + std::move pour le constructeur de colonne
    SQLExpression(std::string column, std::string tableAlias, bool doubleQuoted = false);

    // Règle des 5 explicite (Performances par défaut)
    ~SQLExpression() = default;
    SQLExpression(const SQLExpression&) = default;
    SQLExpression& operator=(const SQLExpression&) = default;
    SQLExpression(SQLExpression&&) noexcept = default;
    SQLExpression& operator=(SQLExpression&&) noexcept = default;

    // 🟢 OPTIMISATION : Retour de référence const pour éviter une copie de chaîne à chaque appel
    const std::string& alias() const noexcept;
    
    // 🟢 OPTIMISATION : Passage par string_view pour l'alias (évite les allocations)
    SQLExpression as(std::string_view alias) const;

    bool empty() const noexcept;
    std::string toSQL(bool quoted = true) const; 
    std::string toSQLWithAlias(bool quoted = true) const;

    // Opérateurs mathématiques et logiques
    SQLExpression operator+(const SQLExpression& rhs) const;
    SQLExpression operator-(const SQLExpression& rhs) const;
    SQLExpression operator*(const SQLExpression& rhs) const;
    SQLExpression operator/(const SQLExpression& rhs) const;
    SQLExpression operator&&(const SQLExpression& rhs) const;
    SQLExpression operator||(const SQLExpression& rhs) const;

    // Comparaisons SQL
    SQLExpression equal(const SQLExpression& rhs) const;
    SQLExpression notEqual(const SQLExpression& rhs) const;
    SQLExpression greater(const SQLExpression& rhs) const;
    SQLExpression less(const SQLExpression& rhs) const;
    SQLExpression greaterOrEqual(const SQLExpression& rhs) const;
    SQLExpression lessOrEqual(const SQLExpression& rhs) const;

    // 🟢 OPTIMISATION : Utilisation de std::string_view pour les patterns
    SQLExpression like(std::string_view pattern) const;
    SQLExpression notLike(std::string_view pattern) const;

    // Clauses IN et BETWEEN
    SQLExpression in(const std::vector<std::string>& values) const;
    SQLExpression in(const std::vector<int>& values) const;
    SQLExpression in(const std::vector<SQLExpression>& values) const;
    SQLExpression between(const SQLExpression& low, const SQLExpression& high) const;

    // Fonctions d'agrégation et de chaînes
    SQLExpression avg() const;
    SQLExpression sum() const;
    SQLExpression count() const;
    SQLExpression max() const;
    SQLExpression min() const;
    SQLExpression upper() const;
    SQLExpression lower() const;

    // 🟢 OPTIMISATION : Utilisation de std::string_view pour le fallback COALESCE
    SQLExpression coalesce(std::string_view fallback) const;
    static SQLExpression coalesce(std::initializer_list<SQLExpression> args);

    SQLExpression round() const;
    SQLExpression round(int decimals) const;
    SQLExpression power(int p) const;

    SQLExpression nullif(const SQLExpression& other) const;

    static SQLExpression caseExpr(
        const std::vector<std::pair<SQLExpression, SQLExpression>>& whenThenPairs,
        const SQLExpression& elseExpr);

    SQLExpression isNull() const;
    SQLExpression isNotNull() const;

    // 🟢 OPTIMISATION : Utilisation de std::string_view ou chaînes optimisées pour le quoting
    static std::string quoteIdentifier(std::string_view ident);
    static std::string quoteString(std::string_view s);
    static std::string quoteString(std::string s); // Surcharge par valeur pour le déplacement direct

private:
    std::string expression_;
    std::string alias_;

    // 🟢 OPTIMISATION : Passage par string_view pour le nom de fonction / opérateur interne
    SQLExpression wrapFunction(std::string_view func) const;
    SQLExpression wrapBinary(const SQLExpression& rhs, std::string_view op) const;
};

} // namespace cppsqlx

#endif // SQLEXPRESSION_HPP
