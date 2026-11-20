
#include <iostream>
#include <memory>
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
        auto ds=std::make_shared<Dataset>("customers");
        auto query=SelectQuery("orders")
        .select("customer_id"_col, "SUM(amount)"_sqlx.as("total_amount"))
        .join("INNER JOIN", ds, "c",
            JEX().
            andJoin
            (
                JEX().left("orders.customer_id"_sqlx).right("c.id"_sqlx).equalOp()
                ,
                JEX().left("orders.status"_sqlx).right("active"_sql).equalOp()
            )  
        )
        .groupBy("customer_id"_col)
        .orderBy(desc("total_amount"), asc("customer_id"))
        .having("amount"_col.sum().greater("1000"_sqlx));

        // Generate SQL
        std::string sql = query.toSQL();
        std::cout << "Generated SQL:\n" << sql << std::endl;
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
    }

    return 0;
}
