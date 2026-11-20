/*
MIT License
Copyright (c) 2025 Y-HRAH KRONG Santili
*/

#ifndef CPPSQLX_DTL_QUERY_H
#define CPPSQLX_DTL_QUERY_H

#include <cppsqlx/sqlquery.hpp>
#include <cppsqlx/filterable.h>
#include <string>
#include <utility>

namespace cppsqlx {

// Déclaration anticipée propre de la classe Connection
class Connection;

/**
 * @brief Base template for Data Transaction Language queries (INSERT, UPDATE, DELETE).
 * Returns int (affected rows) and supports WHERE clauses via CRTP.
 */
template <typename Derived>
class DTLQuery : public SQLQuery<int>, public filterable<Derived> {
protected:
    std::string table_;

public:
    /**
     * @brief Constructeur optimisé par sémantique de mouvement.
     */
    explicit DTLQuery(std::string table) noexcept : table_(std::move(table)) {}
    
    // Destructeur virtuel par défaut
    ~DTLQuery() override = default;

    // Règle des 5 : Assurer un comportement sain lors des copies/déplacements en template
    DTLQuery(const DTLQuery&) = default;
    DTLQuery& operator=(const DTLQuery&) = default;
    DTLQuery(DTLQuery&&) noexcept = default;
    DTLQuery& operator=(DTLQuery&&) noexcept = default;

    /**
     * @brief Execute the query and return affected rows.
     * 🟢 CORRECTION : Utilisation d'une référence/pointeur nu Connection au lieu du shared_ptr obsolète.
     */
    int run(Connection& connection) override {
        // Logique d'exécution fictive (à surcharger ou implémenter via libpq)
        // Exemple :
        // auto stmt = connection.createStatement();
        // ...
        return 1;
    }

    /**
     * @brief Accesseur optimisé.
     */
    const std::string& table() const noexcept { return table_; }
};

} // namespace cppsqlx

#endif // CPPSQLX_DTL_QUERY_H
