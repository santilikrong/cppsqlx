
#include "cppsqlx/sqljoinexpression.hpp"

namespace cppsqlx
{

SQLJoinExpression::SQLJoinExpression()
    : operator_(SQLOperator::Equal), isLogicalJoin_(false)
{
}

SQLJoinExpression::SQLJoinExpression(SQLExpression left,
                                     SQLExpression right,
                                     SQLOperator compareOp)
    : leftExpression_(std::move(left)),
      rightExpression_(std::move(right)),
      operator_(compareOp),
      isLogicalJoin_(false)
{
}

SQLJoinExpression::SQLJoinExpression(SQLJoinExpression left,
                                     SQLJoinExpression right,
                                     SQLOperator logicalOp)
    : leftExpression_(SQLExpression("(" + left.toSQL() + ")")),
      rightExpression_(SQLExpression("(" + right.toSQL() + ")")),
      operator_(logicalOp),
      isLogicalJoin_(true)
{
}

std::string SQLJoinExpression::toSQL(bool quoted) const
{
    const std::string opStr = std::string(cppsqlx::toSQL(operator_));
    if (!isLogicalJoin_) {
        return leftExpression_.toSQL(quoted) + " " + opStr + " " + rightExpression_.toSQL(quoted);
    }
    return "(" + leftExpression_.toSQL(quoted) + ") " + opStr + " (" + rightExpression_.toSQL(quoted) + ")";
}

json SQLJoinExpression::toJSON() const
{
    return json{
        {"leftexpression", leftExpression().toJSON()},
        {"operator", getStringOperator()},
        {"rightexpression", rightExpression().toJSON()}
    };
}

/**
 * @brief Convertit la représentation textuelle SQL d'un opérateur en sa valeur d'énumération SQLOperator.
 */
static SQLOperator parseOperatorString(const std::string& opStr) 
{
    // Opérateurs de comparaison
    if (opStr == "=")   return SQLOperator::Equal;
    if (opStr == "<>" || opStr == "!=")  return SQLOperator::NotEqual;
    if (opStr == "<")   return SQLOperator::Less;
    if (opStr == ">")   return SQLOperator::Greater;
    if (opStr == "<=")  return SQLOperator::LessOrEqual;
    if (opStr == ">=")  return SQLOperator::GreaterOrEqual;
    
    // Opérateurs logiques de composition
    if (opStr == "AND") return SQLOperator::And;
    if (opStr == "OR")  return SQLOperator::Or;

    // Valeur de secours par défaut ou gestion d'erreur selon les préférences de votre bibliothèque
    return SQLOperator::Equal; 
}

std::expected<SQLJoinExpression, std::string> SQLJoinExpression::fromJSON(const json& j)
{
    const json& data = j.contains("sqljoinexpression") ? j["sqljoinexpression"] : j;

    try {
        std::string opStr = data.at("operator").get<std::string>();
        SQLOperator op = parseOperatorString(opStr);

        const json& leftJson = data.at("leftexpression");
        const json& rightJson = data.at("rightexpression");

        // Détection de l'arborescence : s'agit-il d'expressions SQLJoinExpression imbriquées ?
        bool isLeftLogical = leftJson.contains("operator") || leftJson.contains("sqljoinexpression");
        bool isRightLogical = rightJson.contains("operator") || rightJson.contains("sqljoinexpression");

        if (isLeftLogical && isRightLogical) {
            auto leftRes = SQLJoinExpression::fromJSON(leftJson);
            auto rightRes = SQLJoinExpression::fromJSON(rightJson);

            if (!leftRes) return std::unexpected(leftRes.error());
            if (!rightRes) return std::unexpected(rightRes.error());

            return SQLJoinExpression(std::move(*leftRes), std::move(*rightRes), op);
        } 
        else {
            auto leftRes = SQLExpression::fromJSON(leftJson);
            auto rightRes = SQLExpression::fromJSON(rightJson);

            if (!leftRes) return std::unexpected(leftRes.error());
            if (!rightRes) return std::unexpected(rightRes.error());

            return SQLJoinExpression(std::move(*leftRes), std::move(*rightRes), op);
        }
    }
    catch (const std::exception& e) {
        return std::unexpected(std::string("SQLJoinExpression::fromJSON failed: ") + e.what());
    }
}


SQLOperator SQLJoinExpression::getOperator() const
{
    return operator_;
}

std::string SQLJoinExpression::getStringOperator() const
{
    return std::string(cppsqlx::toSQL(operator_));
}

const SQLExpression& SQLJoinExpression::leftExpression() const
{
    return leftExpression_;
}

const SQLExpression& SQLJoinExpression::rightExpression() const
{
    return rightExpression_;
}

void SQLJoinExpression::setLeftExpression(const SQLExpression &ex)
{
    leftExpression_ = ex;
}

void SQLJoinExpression::setRightExpression(const SQLExpression &ex)
{
    rightExpression_ = ex;
}

void SQLJoinExpression::setOperator(SQLOperator op)
{
    operator_ = op;
}

SQLJoinExpression SQLJoinExpression::andJoin(const SQLJoinExpression &left,
                                             const SQLJoinExpression &right)
{
    return SQLJoinExpression(left, right, SQLOperator::And);
}

SQLJoinExpression SQLJoinExpression::orJoin(const SQLJoinExpression &left,
                                            const SQLJoinExpression &right)
{
    return SQLJoinExpression(left, right, SQLOperator::Or);
}

} // namespace cppsqlx
