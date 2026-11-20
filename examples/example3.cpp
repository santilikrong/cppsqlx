#include <iostream>
#include <memory>
#include <utility>
#include "cppsqlx/selectquery.hpp"
#include "cppsqlx/sqljoinexpression.hpp"
#include "cppsqlx/sqloperators.hpp"
#include "cppsqlx/sqlliterals.hpp" 
#include "cppsqlx/sqlexpression.hpp"
#include "cppsqlx/dataset.hpp"
#include "cppsqlx/sqljoinexpressionbuilder.hpp"
#include "cppsqlx/sqlselectbuilder.hpp"

using namespace cppsqlx;
using namespace cppsqlx::literals;

int main()
{
    try
    {
        // 1. Définition des sous-requêtes (API fluide tirant parti des littéraux optimisés)
        auto q1 = SelectQuery()
                    .select("c11"_sql.as("c11"))
                    .select("c12"_sql.as("c12"))
                    .from("t1");

        auto q2 = SelectQuery()
                    .select("12"_sql.as("c21"))
                    .select("World"_sql.as("c22"));

        // 2. Construction de la requête principale avec CTE et jointure
        // Note : Les méthodes .as() utilisent désormais std::string_view sans allocation sur le tas.
        auto query = SelectQuery()
                     .with(CTE("q1", std::make_shared<SelectQuery>(std::move(q1))))
                     .select("q1.c12"_col.as("c1"), "q2.c22"_col.as("c2"))
                     .from("q1")
                     .join("INNER JOIN", std::make_shared<SelectQuery>(std::move(q2)), "q2", 
                           JEX().left("c11"_col).right("c21"_col).equalOp());

        // 3. Génération et affichage du code SQL
        std::string sql = query.toSQL();
        std::cout << "Generated SQL:\n" << sql << std::endl;
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
