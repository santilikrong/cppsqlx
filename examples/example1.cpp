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
#include "cppsqlx/statement.hpp" // 🟢 AJOUT CRUCIAL : Rend le type Statement complet pour l'appel de execute()

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
    cppsqlx::Logger::instance().init();
    auto logger = cppsqlx::Logger::instance().getLogger("cppsqlx.example1");

    std::string dbname, user, password, host, query;
    int port;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Show help message")
        ("dbname,d", po::value<std::string>(&dbname)->default_value("dizdb"), "Database name")
        ("user,u", po::value<std::string>(&user)->default_value("myuser"), "Database user")
        ("host,H", po::value<std::string>(&host)->default_value("localhost"), "Database host")
        ("port,P", po::value<int>(&port)->default_value(5432), "Database port")
        ("query,q", po::value<std::string>(&query)->required(), "SQL query to stream");

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
        auto stmt = conn.createStatement();
        
        // 🟢 Correction : Traiter le type de retour std::expected
        auto resExpected = stmt->execute(query);
        if (!resExpected.has_value()) 
        {
            LOG4CXX_ERROR(logger, "Query execution failed: " + resExpected.error());
            return 1;
        }

        // Récupération de l'objet Resultset réel par référence
        auto& res = resExpected.value();
        
        int numcols = res.columns(); // Remplacement de res->columns() par res.columns()
        std::cout << "Number of cols : " << res.columns() << std::endl;
        std::cout << "Number of rows : " << res.rows() << std::endl;

        // 🟢 Correction du parcours de Resultset (suppression de l'étoile *res)
        for (auto row : res)
        {
            for (int i = 0; i < numcols; i++)
            {
                auto value = row[i];
                // Vérification si la valeur optionnelle contient une chaîne avant affichage
                if (value.has_value()) {
                    std::cout << value.value() << ";";
                } else {
                    std::cout << "NULL;";
                }
            };
            std::cout << std::endl;
        }

        // Fermeture propre de la connexion
        if (auto closeRes = conn.close(); !closeRes)
        {
            LOG4CXX_WARN(logger, "Error while closing connection: " + closeRes.error());
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

