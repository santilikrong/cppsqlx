/*
MIT License
Copyright (c) 2025 Y-HRAH KRONG Santili
*/

#ifndef CPPSQLX_DTL_QUERY_H
#define CPPSQLX_DTL_QUERY_H

#include <cppsqlx/sqlquery.hpp>
#include <cppsqlx/filterable.h>
#include <string>
#include <memory>

namespace cppsqlx {

/**
 * @brief Base template for Data Transaction Language queries (INSERT, UPDATE, DELETE).
 * Returns int (affected rows) and supports WHERE clauses via CRTP.
 */
template <typename Derived>
class DTLQuery : public SQLQuery<int>, public filterable<Derived> {
protected:
    std::string table_;

public:
    explicit DTLQuery(const std::string& table) : table_(table) {}
    virtual ~DTLQuery() = default;

    /**
     * @brief Execute the query and return affected rows.
     */
    int run(std::shared_ptr<class Connection> connection) override {
        // Placeholder: actual DB execution logic goes here
        return 1;
    }

    const std::string& table() const { return table_; }
};

} // namespace cppsqlx

#endif // CPPSQLX_DTL_QUERY_H