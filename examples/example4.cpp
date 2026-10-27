/*
MIT License

Copyright (c) 2025 Y-HRAH KRONG Santili

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <iostream>
#include "cppsqlx/logger.hpp"
#include <boost/program_options.hpp>
#include "cppsqlx/connection.hpp"
#include "cppsqlx/datasetfactory.hpp"
#include "cppsqlx/sqlselectbuilder.hpp"
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

namespace po = boost::program_options;

using namespace cppsqlx;
using namespace cppsqlx::literals;

int main(int argc, char *argv[])
{
    cppsqlx::Logger::instance().init();
    auto logger = cppsqlx::Logger::instance().getLogger("cppsqlx.example1");

    std::string dbname, user, password, host, query;
    int port;
    po::options_description desc("Allowed options");
    desc.add_options()("help,h", "Show help message")("dbname,d", po::value<std::string>(&dbname)->default_value("dizdb"), "Database name")("user,u", po::value<std::string>(&user)->required(), "Database user")("host,H", po::value<std::string>(&host)->default_value("localhost"), "Database host")("port,P", po::value<int>(&port)->default_value(5432), "Database port");

    po::variables_map vm;
    try
    {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help"))
        {
            std::cout << desc << "\n";
            return 0;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error parsing options: " << e.what() << "\n";
        return 1;
    }

    LOG4CXX_INFO(logger, "Starting example application");
    // Build connection string
    std::string connStr = "dbname=" + dbname +
                          " user=" + user +
                          " host=" + host +
                          " port=" + std::to_string(port) +
                          " sslmode=disable";

    cppsqlx::Connection conn(connStr);
    if (auto result = conn.connect(); !result)
    {
        LOG4CXX_INFO(logger, "Failed to connect: " + result.error());
        return 1;
    }

    if (conn.isConnected())
    {
        LOG4CXX_INFO(logger, "Connected to PostgreSQL successfully");
        auto ds1 = cppsqlx::DatasetFactory().createFromDatabase(conn, "t1");
        auto ds2 = cppsqlx::DatasetFactory().createFromDatabase(conn, "t2");

        // std::cout << ds1->toString() << "\n";

        auto q1 = cppsqlx::SelectQuery()
                      .select("c11"_col.as("c11"), "c12"_col.as("c12"), "c13"_col.as("c13"))
                      .from(ds1.value());

        auto q2 = cppsqlx::SelectQuery()
                      .select("c21"_col.as("c21"), "c22"_col.as("c22"))
                      .from(ds2.value());

        auto query = SelectQuery()
                         .with(CTE("q1", std::make_shared<SelectQuery>(q1)))
                         .select("q1.c12"_col.as("c1"), "q1.c13"_col.as("c3"), "q2.c22"_col.as("c2"))
                         .from("q1")
                         .join("INNER JOIN", std::make_shared<SelectQuery>(q2), "q2",
                               JEX().left("c11"_col).right("c21"_col).equalOp());

        std::cout << query.toSQL() << "\n";

        auto stmt = conn.createStatement();
        auto res = stmt->execute(query.toSQL());
        int numcols = res->columns();
        std::cout << "Number of cols : " << res->columns() << std::endl;
        std::cout << "Number of rows : " << res->rows() << std::endl;

        //res->print();
        std::cout << res.value() << std::endl ;

        /*for (auto row : *res)
        {
            for (int i = 0; i < numcols; i++)
            {
                auto value = row[i];
                std::cout << *value << ";";
            };
            std::cout << std::endl;
        }*/


        if (auto res = conn.close(); !res)
        {
            LOG4CXX_WARN(logger, "Error while closing connection: " + res.error());
        }
        LOG4CXX_INFO(logger, "Disconnected from PostgreSQL");
    }
    else
    {
        LOG4CXX_ERROR(logger, "Failed to connect to PostgreSQL");
    }

    LOG4CXX_INFO(logger, "Example application finished");

    return 0;
}
