#ifndef CPPSQLX_UNIT_SELECT_HPP
#define CPPSQLX_UNIT_SELECT_HPP

#include <string>
#include <variant>
#include <expected>
#include <cppsqlx/selectquery.hpp>
#include <cppsqlx/insertquery.hpp>
#include <cppsqlx/ctasquery.hpp>
#include <cppsqlx/updatequery.hpp>
#include <cppsqlx/deletequery.hpp>
#include <cppsqlx/json_concepts.hpp>
#include <cppsqlx/droptablequery.hpp>

namespace cppsqlx
{

    enum class TargetType
    {
        SubQuery,
        TemporaryTable,
        View,
        Table
    };

    // Utilitaire de surcharge pour std::visit (Pattern classique C++20/C++23)
    template <class... Ts>
    struct overloads : Ts...
    {
        using Ts::operator()...;
    };

    // Alias pour le variant regroupant toutes les requêtes possibles dans une étape
    using QueryVariant = std::variant<SelectQuery, InsertQuery, CTASQuery, UpdateQuery, DeleteQuery, DropTableQuery>;

    inline std::string targetTypeToString(TargetType type)
    {
        switch (type)
        {
        case TargetType::SubQuery:
            return "sub_query";
        case TargetType::TemporaryTable:
            return "temporary_table";
        case TargetType::View:
            return "view";
        case TargetType::Table:
            return "table";
        }
        return "table";
    }

    inline std::expected<TargetType, std::string> stringToTargetType(const std::string &str)
    {
        if (str == "sub_query")
            return TargetType::SubQuery;
        if (str == "temporary_table")
            return TargetType::TemporaryTable;
        if (str == "view")
            return TargetType::View;
        if (str == "table")
            return TargetType::Table;
        return std::unexpected("TargetType inconnu : " + str);
    }

    struct UnitSelect
    {
        QueryVariant query; // 🟢 Remplacement par le std::variant statique
        TargetType targetType = TargetType::Table;
        std::string name;

        // Métadonnées ou flag d'activation pour la gestion globale
        bool quoted = false;

        /**
         * @brief Génère le SQL correspondant à la requête active dans le variant.
         */
        [[nodiscard]] std::string toSQL() const;

        // ---------- JSON Concepts Contract ----------
        [[nodiscard]] json toJSON() const;
        static std::expected<UnitSelect, std::string> fromJSON(const json &j);
    };

} // namespace cppsqlx

#endif // CPPSQLX_UNIT_SELECT_HPP
