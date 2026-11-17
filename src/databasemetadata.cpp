#include "cppsqlx/statement.hpp"
#include "cppsqlx/resultset.hpp"
#include <sstream>
#include <cppsqlx/databasemetadata.hpp>

namespace cppsqlx
{

std::expected<std::vector<std::string>, std::string> DatabaseMetadata::getSchemas() const
{
    if (!connection_ || !connection_->isConnected()) {
        return std::unexpected("DatabaseMetadata error: Active database connection required.");
    }

    auto stmt = connection_->createStatement();
    if (!stmt) return std::unexpected("DatabaseMetadata error: Failed to create statement context.");

    std::string sql = "SELECT schema_name FROM information_schema.schemata "
                      "WHERE schema_name NOT LIKE 'pg_%' AND schema_name <> 'information_schema' "
                      "ORDER BY schema_name;";

    auto res = stmt->execute(sql);
    if (!res) return std::unexpected(res.error());

    std::vector<std::string> schemas;
    for (int i = 0; i < res->rows(); ++i) {
        schemas.push_back(res->fetch(i, 0).value_or("")); 
    }

    return schemas;
}

std::expected<std::vector<std::string>, std::string> DatabaseMetadata::getTables(const std::string& schema) const
{
    if (!connection_ || !connection_->isConnected()) {
        return std::unexpected("DatabaseMetadata error: Active database connection required.");
    }

    auto stmt = connection_->createStatement();
    if (!stmt) return std::unexpected("DatabaseMetadata error: Failed to create statement context.");

    std::ostringstream sql;
    sql << "SELECT table_name FROM information_schema.tables "
        << "WHERE table_schema = '" << schema << "' AND table_type = 'BASE TABLE' "
        << "ORDER BY table_name;";

    auto res = stmt->execute(sql.str());
    if (!res) return std::unexpected(res.error());

    std::vector<std::string> tables;
    for (int i = 0; i < res->rows(); ++i) {
        tables.push_back(res->fetch(i, 0).value_or(""));
    }
    return tables;
}

std::expected<std::vector<std::string>, std::string> DatabaseMetadata::getViews(const std::string& schema) const
{
    if (!connection_ || !connection_->isConnected()) {
        return std::unexpected("DatabaseMetadata error: Active database connection required.");
    }

    auto stmt = connection_->createStatement();
    if (!stmt) return std::unexpected("DatabaseMetadata error: Failed to create statement context.");

    std::ostringstream sql;
    sql << "SELECT table_name FROM information_schema.tables "
        << "WHERE table_schema = '" << schema << "' AND table_type = 'VIEW' "
        << "ORDER BY table_name;";

    auto res = stmt->execute(sql.str());
    if (!res) return std::unexpected(res.error());

    std::vector<std::string> views;
    for (int i = 0; i < res->rows(); ++i) {
        views.push_back(res->fetch(i, 0).value_or(""));
    }
    return views;
}

std::expected<std::vector<ColumnMetadata>, std::string> DatabaseMetadata::getTableStructure(const std::string& tableName, const std::string& schema) const
{
    if (!connection_ || !connection_->isConnected()) {
        return std::unexpected("DatabaseMetadata error: Active database connection required.");
    }

    auto stmt = connection_->createStatement();
    if (!stmt) return std::unexpected("DatabaseMetadata error: Failed to create statement context.");

    std::ostringstream sql;
    sql << "SELECT c.column_name, "
        << "  CASE "
        << "    WHEN c.character_maximum_length IS NOT NULL THEN c.data_type || '(' || c.character_maximum_length || ')' "
        << "    WHEN c.numeric_precision IS NOT NULL AND c.numeric_scale IS NOT NULL AND c.data_type <> 'integer' THEN c.data_type || '(' || c.numeric_precision || ',' || c.numeric_scale || ')' "
        << "    ELSE c.data_type "
        << "  END as formatted_type, " // 🟢 CONCATÉNATION DE LA TAILLE (ex: varchar(50))
        << "  c.is_nullable, "
        << "  COALESCE((SELECT 'YES' FROM information_schema.table_constraints tc "
        << "    JOIN information_schema.key_column_usage kcu ON tc.constraint_name = kcu.constraint_name "
        << "    WHERE tc.constraint_type = 'PRIMARY KEY' AND tc.table_schema = c.table_schema "
        << "    AND tc.table_name = c.table_name AND kcu.column_name = c.column_name LIMIT 1), 'NO') as is_pk "
        << "FROM information_schema.columns c "
        << "WHERE c.table_schema = '" << schema << "' AND c.table_name = '" << tableName << "' "
        << "ORDER BY c.ordinal_position;";

    auto res = stmt->execute(sql.str());
    if (!res) return std::unexpected(res.error());

    std::vector<ColumnMetadata> columns;
    for (int i = 0; i < res->rows(); ++i) {
        ColumnMetadata col;
        col.name = res->fetch(i, 0).value_or("");
        col.dataType = res->fetch(i, 1).value_or(""); // 🟢 Contient maintenant le type formaté avec la taille
        col.isNullable = (res->fetch(i, 2).value_or("") == "YES");
        col.isPrimaryKey = (res->fetch(i, 3).value_or("") == "YES");
        columns.push_back(std::move(col));
    }

    return columns;
}

std::expected<std::vector<ColumnMetadata>, std::string> DatabaseMetadata::getQueryStructure(const std::string& selectSql) const
{
    if (!connection_ || !connection_->isConnected())
    {
        return std::unexpected("Connexion PostgreSQL inactive pour l'introspection de la requête.");
    }

    // Exécution avec LIMIT 0 : PostgreSQL planifie la requête sans lire aucune ligne
    std::string probeSql = "SELECT * FROM (" + selectSql + ") AS _dizui_introspect LIMIT 0";
    PGresult* res = PQexec(connection_->get(), probeSql.c_str());

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        std::string err = PQerrorMessage(connection_->get());
        PQclear(res);
        return std::unexpected("Erreur SQL lors de l'introspection : " + err);
    }

    int numFields = PQnfields(res);
    std::vector<ColumnMetadata> columns;
    columns.reserve(numFields);

    for (int i = 0; i < numFields; ++i)
    {
        ColumnMetadata col;
        col.name = PQfname(res, i);
        col.isNullable = true;
        col.isPrimaryKey = false;

        Oid typeOid = PQftype(res, i);
        int typeMod = PQfmod(res, i);

        // Résolution du nom lisible du type PostgreSQL (ex: "character varying", "numeric", "integer")
        std::string typeQuery = "SELECT pg_catalog.format_type(" + std::to_string(typeOid) + ", " + std::to_string(typeMod) + ")";
        PGresult* typeRes = PQexec(connection_->get(), typeQuery.c_str());
        if (PQresultStatus(typeRes) == PGRES_TUPLES_OK && PQntuples(typeRes) > 0)
        {
            col.dataType = PQgetvalue(typeRes, 0, 0);
        }
        else
        {
            col.dataType = "unknown";
        }
        PQclear(typeRes);

        columns.push_back(std::move(col));
    }

    PQclear(res);
    return columns;
}


} // namespace cppsqlx
