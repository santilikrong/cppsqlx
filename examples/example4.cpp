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
#include <fstream> 
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
#include "cppsqlx/json_concepts.hpp"

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
        auto dsemp = cppsqlx::DatasetFactory().createFromDatabase(conn, "t_employes");
        auto dsag = cppsqlx::DatasetFactory().createFromDatabase(conn, "t_agences");
        auto dsreg = cppsqlx::DatasetFactory().createFromDatabase(conn, "t_regions");

        if (dsreg) 
        {
            std::cout << "--- Représentation JSON de dsreg (Metadata & Columns) ---" << std::endl;
            std::cout << cppsqlx::io::dump(dsreg.value(),4) << std::endl;
            std::cout << "--------------------------------------------------------" << std::endl;
        }
        else 
        {
            LOG4CXX_ERROR(logger, "Impossible de générer le JSON : dsreg est invalide -> " + dsreg.error());
        }

        auto q_ag = cppsqlx::SelectQuery()
                      .select("id"_col.as("id"), "nom"_col.as("nom"), "id_reg"_col.as("id_reg"))
                      .from(dsag.value());

auto q_emp = cppsqlx::SelectQuery()
              .with(CTE("ag", std::make_shared<SelectQuery>(q_ag)))
              .select("emp.id"_col.as("id"), "emp.matricule"_col.as("matricule"), "ag.nom"_col.as("agence"), "ag.id_reg"_col.as("id_reg"), "emp.id_ag"_col.as("id_ag"))
              .from(dsemp.value().as("emp"))
              .join("INNER JOIN", std::make_shared<SelectQuery>(q_ag), "ag", // 💡 Utilise q_ag
                       JEX().left("emp.id_ag"_col).right("ag.id"_col).equalOp());



        auto q_reg = cppsqlx::SelectQuery()
                      .select("id"_col.as("id"), "nom"_col.as("nom"))
                      .from(dsreg.value());

        auto query = SelectQuery()
                         .with(CTE("emp", std::make_shared<SelectQuery>(q_emp)))
                         .select("emp.matricule"_col.as("matricule"), "emp.agence"_col.as("agence"), "reg.nom"_col.as("region"))
                         .from(dsreg.value().as("reg"))
                         .join("INNER JOIN", std::make_shared<SelectQuery>(q_emp), "emp",
                               JEX().left("emp.id_reg"_col).right("reg.id"_col).equalOp())
                               ; 



        std::cout << "--- Représentation JSON de query ---" << std::endl;
        std::string serializedQuery = cppsqlx::io::dump(query, 4);
        std::cout << serializedQuery << std::endl;
        std::cout << "------------------------------------" << std::endl;

        // 🟢 SAUVEGARDE DU JSON DANS UN FICCHIER
        std::string filename = "query_output.json";
        std::ofstream jsonFile(filename);
        if (jsonFile.is_open())
        {
            jsonFile << serializedQuery;
            jsonFile.close();
            LOG4CXX_INFO(logger, "Représentation JSON sauvegardée avec succès dans " + filename);
        }
        else
        {
            LOG4CXX_ERROR(logger, "Impossible d'ouvrir le fichier " + filename + " pour écriture");
        }

        // 🟢 RECONSTITUTION DE L'OBJET DEPUIS LE JSON (Désérialisation)
        LOG4CXX_INFO(logger, "Tentative de reconstruction de la requête depuis sa chaîne JSON...");
        
        // On parse la chaîne de caractères brute en objet nlohmann::json
        json parsedJson = json::parse(serializedQuery);
        
        // Appel de la méthode usine statique protégée par std::expected
        auto query_loaded_res = cppsqlx::SelectQuery::fromJSON(parsedJson);

        if (query_loaded_res)
        {
            // Instanciation de query_loaded par déplacement
            cppsqlx::SelectQuery query_loaded = std::move(*query_loaded_res);
            LOG4CXX_INFO(logger, "Objet query_loaded instancié avec succès !");

            std::cout << "--- Requête SQL issue de l'objet reconstruit (query_loaded) ---" << std::endl;
            std::cout << query_loaded.toSQL() << std::endl;
            std::cout << "----------------------------------------------------------------" << std::endl;
        }
        else
        {
            // En cas d'erreur de structure JSON, on log l'erreur explicite sans planter
            LOG4CXX_ERROR(logger, "Échec de la désérialisation de query_loaded : " + query_loaded_res.error());
        }

        std::cout << query.toSQL() << "\n";

        auto stmt = conn.createStatement();
        auto res = stmt->execute(query.toSQL());
        int numcols = res->columns();
        std::cout << "Number of cols : " << res->columns() << std::endl;
        std::cout << "Number of rows : " << res->rows() << std::endl;

        std::cout << res.value() << std::endl ;


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
