#include "cppsqlx/ddlquery.hpp"

namespace cppsqlx {

DDLQuery::DDLQuery(const std::string& ddl) : ddl_(ddl) {}

std::string DDLQuery::toString() const {
    return ddl_;
}

void DDLQuery::run(std::shared_ptr<Connection> connection) {
    // Execute DDL query (CREATE, DROP, ALTER)
}

} // namespace cppsqlx