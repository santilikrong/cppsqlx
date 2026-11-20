#include <iostream>
#include <fstream>
#include <memory>
#include <boost/program_options.hpp>

#include "cppsqlx/logger.hpp"
#include "cppsqlx/connection.hpp"
#include "cppsqlx/datasetfactory.hpp"
#include "cppsqlx/dataintegrationproject.hpp"
#include "cppsqlx/json_concepts.hpp"
#include "cppsqlx/selectquery.hpp"
#include "cppsqlx/insertquery.hpp"
#include "cppsqlx/updatequery.hpp"
#include "cppsqlx/deletequery.hpp"
#include "cppsqlx/ctasquery.hpp"
#include "cppsqlx/droptablequery.hpp"
#include "cppsqlx/sqljoinexpression.hpp"
#include "cppsqlx/sqloperators.hpp"
#include "cppsqlx/sqlliterals.hpp"
#include "cppsqlx/sqlexpression.hpp"
#include "cppsqlx/dataset.hpp"
#include "cppsqlx/sqljoinexpressionbuilder.hpp"
#include "cppsqlx/sqlselectbuilder.hpp"
#include <cppsqlx/physicalconnection.hpp>
#include <cppsqlx/logicalconnection.hpp>

namespace po = boost::program_options;
using namespace cppsqlx;
using namespace cppsqlx::literals;

int main(int argc, char *argv[])
{
    cppsqlx::Logger::instance().init();
    auto logger = cppsqlx::Logger::instance().getLogger("cppsqlx.example_project");

    std::string dbname, user, password, host;
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

    LOG4CXX_INFO(logger, "Starting DataIntegrationProject multiple jobs example application");

    std::string connStr = "dbname=" + dbname + " user=" + user + " host=" + host + " port=" + std::to_string(port) + " sslmode=disable";
    cppsqlx::Connection conn(connStr);

    if (auto result = conn.connect(); !result)
    {
        LOG4CXX_INFO(logger, "Failed to connect: " + result.error());
        return 1;
    }

    if (conn.isConnected())
    {
        LOG4CXX_INFO(logger, "Connected to PostgreSQL successfully. Building Project Metadata...");

        // 1. Instanciation racine du Projet
        DataIntegrationProject project("Plateforme_ETL_Finance", "/home/santili/cppsqlx/projects/finance");

        // 2. Déclaration de l'infrastructure Réseau (Physique & Logique)
        PhysicalConnection physDev;
        physDev.name_ = "pg_dev_cluster";
        physDev.dbname_ = "d_financedb";
        physDev.dbhost_ = "localhost";
        physDev.dbport_ = 5432;
        physDev.dbuser_ = "santili";
        project.addPhysicalConnection(std::move(physDev));

        PhysicalConnection physTest;
        physTest.name_ = "pg_test_cluster";
        physTest.dbname_ = "t_financedb";
        physTest.dbhost_ = "localhost";
        physTest.dbport_ = 5432;
        physTest.dbuser_ = "santili";
        project.addPhysicalConnection(std::move(physTest));

        LogicalConnection logFinance("Finance_DB_Gateway");
        logFinance.registerEnvironment("DEV", project.physicalConnections().at("pg_dev_cluster"));
        logFinance.registerEnvironment("TEST", project.physicalConnections().at("pg_test_cluster"));
        project.addLogicalConnection(std::move(logFinance));

        // 3. Récupération et Enregistrement des Datasets (Metadata de tables)
        auto dsemp = cppsqlx::DatasetFactory().createFromDatabase(conn, "t_employes");
        auto dsag = cppsqlx::DatasetFactory().createFromDatabase(conn, "t_agences");
        auto dsreg = cppsqlx::DatasetFactory().createFromDatabase(conn, "t_regions");

        if (dsemp)
            project.addDataset(std::move(dsemp.value()));
        if (dsag)
            project.addDataset(std::move(dsag.value()));
        if (dsreg)
            project.addDataset(std::move(dsreg.value()));

        // 4. Construction des requêtes isolées (CRUD Variant)
        auto q_ag = cppsqlx::SelectQuery()
                        .select("id"_col.as("id"), "nom"_col.as("nom"), "id_reg"_col.as("id_reg"))
                        .from(project.datasets().at("t_agences"));

        Dataset empDataset = project.datasets().at("t_employes");
        empDataset.as("emp");

        auto q_emp = cppsqlx::SelectQuery()
                         .with(CTE("ag", std::make_shared<SelectQuery>(q_ag)))
                         .select("emp.id"_col.as("id"), "emp.matricule"_col.as("matricule"), "ag.nom"_col.as("agence"), "ag.id_reg"_col.as("id_reg"), "emp.id_ag"_col.as("id_ag"))
                         .from(empDataset)
                         .join("INNER JOIN", std::make_shared<SelectQuery>(q_ag), "ag",
                               JEX().left("emp.id_ag"_col).right("ag.id"_col).equalOp());

        project.addQuery("GetAgences", q_ag);
        project.addQuery("GetEmployesWithAgence", q_emp);

        auto q_ctas = cppsqlx::CTASQuery("t_entrepot")
                          .setTemporary(false)
                          .select(q_ag);
        project.addQuery("PopulateWarehouse", q_ctas);

        // ============================================================================
        // 5. Modélisation de la Tâche de Nettoyage (Nouveau !)
        // ============================================================================
        Task taskClean("Clean_Infrastructure_Task");

        UnitSelect dropStep;
        dropStep.name = "Suppression_Entrepot_Existant";
        dropStep.query = cppsqlx::DropTableQuery("t_entrepot").setIfExists(true);
        dropStep.targetType = TargetType::Table;
        taskClean.addStep(1, std::move(dropStep));

        project.addTask(std::move(taskClean));

        // ============================================================================
        // Tâche principale de Transformation (Modifiée, plus de Drop ici)
        // ============================================================================
        Task taskTransformation("Transform_and_Clean_Task");

        UnitSelect step1;
        step1.name = "Etape_Selection_Initiale";
        step1.query = q_emp;
        step1.targetType = TargetType::SubQuery;
        taskTransformation.addStep(1, std::move(step1));

        UnitSelect step2;
        step2.name = "Etape_Nettoyage_Update";
        step2.query = cppsqlx::UpdateQuery("t_employes")
                          .set("statut", SQLExpression(std::string_view("'traite'")))
                          .where("id"_col.greater(SQLExpression(0)));
        step2.targetType = TargetType::Table;
        taskTransformation.addStep(2, std::move(step2));

        UnitSelect step3;
        step3.name = "Etape_Creation_Entrepot";
        step3.query = q_ctas;
        step3.targetType = TargetType::Table;
        taskTransformation.addStep(3, std::move(step3));

        project.addTask(std::move(taskTransformation));

        // ============================================================================
        // 6. Construction des Jobs d'Orchestration
        // ============================================================================
        // Job 1 : Nettoyage (Pas de dépendances amont)
        Job cleanJob("ETL_Clean_Job");
        cleanJob.setX(100);
        cleanJob.setY(100);
        auto cleanTaskPtr = std::make_shared<Task>(project.tasks().at("Clean_Infrastructure_Task"));
        cleanJob.addTask(1, cleanTaskPtr);
        project.addJob(std::move(cleanJob));

        // Job 2 : Sync & Transformation (Dépend du succès du Job 1)
        Job syncJob("ETL_Sync_Job");
        syncJob.setX(100);
        syncJob.setY(300);
        // 🟢 AJOUT DE LA CONDITION : syncJob attend le succès complet de cleanJob
        syncJob.addCondition(Condition("ETL_Clean_Job", JobStatus::Success));

        auto transTaskPtr = std::make_shared<Task>(project.tasks().at("Transform_and_Clean_Task"));
        syncJob.addTask(1, transTaskPtr);
        project.addJob(std::move(syncJob));

        // ============================================================================
        // 7. Consolidation finale dans le Workflow Orchestrateur
        // ============================================================================
        Workflow mainWorkflow("Workflow_Principal_Finance");

        // On attache les deux Jobs au graphe du Workflow
        mainWorkflow.addJob(std::make_shared<Job>(project.jobs().at("ETL_Clean_Job")));
        mainWorkflow.addJob(std::make_shared<Job>(project.jobs().at("ETL_Sync_Job")));
        project.addWorkflow(std::move(mainWorkflow));

        // ============================================================================
        // SÉRIALISATION GLOBALE DU PROJET
        // ============================================================================
        std::cout << "--- Sérialisation complète du Projet DataIntegrationProject ---" << std::endl;
        std::string serializedProject = cppsqlx::io::dump(project, 4);
        auto res1 = project.saveToDisk();

        auto filename = "/home/santili/cppsqlx/projects/finance/project.json";

        // ============================================================================
        // CHARGEMENT DIRECT ET RECONSTITUTION COMPLÈTE DEPUIS LE DISQUE
        // ============================================================================
        LOG4CXX_INFO(logger, std::string("Tentative de chargement et reconstruction inverse du Projet depuis le fichier : ") + filename);
        auto project_loaded_res = cppsqlx::DataIntegrationProject::loadFromDisk(filename);
        if (project_loaded_res)
        {
            DataIntegrationProject project_loaded = std::move(*project_loaded_res);
            LOG4CXX_INFO(logger, "DataIntegrationProject chargé depuis le disque avec succès !");

            // 🟢 CORRECTION : Ajout des types explicites <Task> entre chevrons
            auto cleanTaskRef = std::make_shared<Task>(project_loaded.tasks().at("Clean_Infrastructure_Task"));
            auto transTaskRef = std::make_shared<Task>(project_loaded.tasks().at("Transform_and_Clean_Task"));

            Job rCleanJob("ETL_Clean_Job");
            rCleanJob.addTask(1, cleanTaskRef);

            Job rSyncJob("ETL_Sync_Job");
            rSyncJob.addCondition(Condition("ETL_Clean_Job", JobStatus::Success));
            rSyncJob.addTask(1, transTaskRef);

            Workflow realWorkflow("Workflow_Principal_Finance");
            
            // 🟢 CORRECTION : Ajout des types explicites <Job> entre chevrons
            realWorkflow.addJob(std::make_shared<Job>(std::move(rCleanJob)));
            realWorkflow.addJob(std::make_shared<Job>(std::move(rSyncJob)));

            const auto& gateway = project_loaded.logicalConnections().at("Finance_DB_Gateway");

            // Exécution asynchrone séquentielle-dépendante par environnement !
            LOG4CXX_INFO(logger, "Lancement du pipeline à multiples Jobs sur l'environnement de DEV...");
            realWorkflow.run(gateway, "DEV");  

            LOG4CXX_INFO(logger, "Lancement du pipeline à multiples Jobs sur l'environnement de TEST...");
            realWorkflow.run(gateway, "TEST"); 
        }

        else
        {
            LOG4CXX_ERROR(logger, "Échec critique du chargement du projet depuis le disque : " + project_loaded_res.error());
        }
        auto closeResult = conn.close();
    }
    LOG4CXX_INFO(logger, "Example DataIntegrationProject finished");
    return 0;
}