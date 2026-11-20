#include <cppsqlx/sqlselectionset.hpp>
#include <cppsqlx/sqlexpression.hpp>

namespace cppsqlx
{

std::expected<SQLSelectionSet, std::string> SQLSelectionSet::fromJSON(const json& j)
{
    // Détermine si le JSON reçu est enveloppé sous une clé racine ou est directement un tableau
    const json& data = j.contains("selectionset") ? j["selectionset"] : j;

    if (!data.is_array()) {
        return std::unexpected("SQLSelectionSet::fromJSON failed: expected a JSON array");
    }

    try {
        SQLSelectionSet selectionSet;

        for (const auto& itemJson : data) {
            // Reconstitution récursive de chaque SQLExpression du tableau
            auto exprRes = SQLExpression::fromJSON(itemJson);
            if (!exprRes) {
                return std::unexpected(exprRes.error()); // Propage l'erreur d'une expression invalide
            }
            selectionSet.add(std::move(*exprRes));
        }

        return selectionSet;
    }
    catch (const std::exception& e) {
        return std::unexpected(std::string("SQLSelectionSet::fromJSON failed: ") + e.what());
    }
}

} // namespace cppsqlx
