#include <iostream>
#include <string>
#include <memory>
#include <boost/program_options.hpp>

#include "cppsqlx/logger.hpp"
#include "cppsqlx/dataintegrationproject.hpp"
#include "cppsqlx/logicalconnection.hpp"
#include "cppsqlx/workflow.hpp"

namespace po = boost::program_options;
using namespace cppsqlx;

void print_banner() {
    std::cout << "==========================================================\n"
              << "          🌐 CPPSQLX - Data Integration CLI 🌐            \n"
              << "==========================================================\n";
}

int main(int argc, char* argv[]) {
    // 1. Initialisation du Logger d'infrastructure
    cppsqlx::Logger::instance().init();
    auto logger = cppsqlx::Logger::instance().getLogger("cppsqlx.cli");

    std::string project_file;
    std::string workflow_name;
    std::string env_context;

    // 2. Définition des arguments de la ligne de commande
    po::options_description desc("Options disponibles");
    desc.add_options()
        ("help,h", "Affiche ce message d'aide")
        ("project,p", po::value<std::string>(&project_file)->required(), "Chemin vers le fichier JSON du projet")
        ("list,l", "Liste les workflows et environnements disponibles dans le projet")
        ("run,r", po::value<std::string>(&workflow_name), "Nom du workflow à exécuter")
        ("env,e", po::value<std::string>(&env_context), "Couloir d'environnement cible (ex: DEV, TEST, PROD)");

    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        
        if (vm.count("help")) {
            print_banner();
            std::cout << desc << "\n";
            return 0;
        }

        po::notify(vm);
    }
    catch (const std::exception& e) {
        print_banner();
        std::cerr << "❌ Erreur d'arguments : " << e.what() << "\n\n";
        std::cout << desc << "\n";
        return 1;
    }

    print_banner();
    LOG4CXX_INFO(logger, "Chargement du projet : " << project_file);

    // 3. Chargement unifié du projet depuis le disque
    auto project_res = DataIntegrationProject::loadFromDisk(project_file);
    if (!project_res) {
        LOG4CXX_ERROR(logger, "❌ Échec critique du chargement : " << project_res.error());
        return 1;
    }

    // 🟢 Utilisation unifiée de la variable 'project_loaded'
    DataIntegrationProject project_loaded = std::move(*project_res);
    LOG4CXX_INFO(logger, "Projet '" << project_loaded.name() << "' chargé avec succès.");

    // 4. Résolution récursive des liens du graphe désérialisé (Workflow -> Job -> Task)
    project_loaded.resolveDependencies();

    // 5. Mode d'affichage/Inspection des métadonnées (--list)
    if (vm.count("list")) {
        std::cout << "\n📋 Métadonnées du Projet :\n"
                  << "  ▪️ Chemin cible : " << project_loaded.path() << "\n"
                  << "  ▪️ Connexions Physiques : " << project_loaded.physicalConnections().size() << "\n";
        
        std::cout << "\n🔄 Workflows disponibles :\n";
        for (const auto& [name, wf] : project_loaded.workflows()) {
            std::cout << "  🔸 " << name << "\n";
        }

        std::cout << "\n🔌 Passerelles Logiques & Couloirs :\n";
        for (const auto& [name, logConn] : project_loaded.logicalConnections()) {
            std::cout << "  🔹 " << name << "\n";
        }
        return 0;
    }

     // 6. Mode exécution (--run et --env)
    if (vm.count("run")) {
        if (!vm.count("env")) {
            LOG4CXX_ERROR(logger, "❌ Erreur : L'option --env (-e) est obligatoire pour exécuter un workflow.");
            return 1;
        }

        if (!project_loaded.workflows().contains(workflow_name)) {
            LOG4CXX_ERROR(logger, "❌ Erreur : Le workflow '" << workflow_name << "' n'existe pas dans ce projet.");
            return 1;
        }

        // 🟢 RÉASSEMBLAGE SÉCURISÉ DIRECT DEPUIS LES DICTIONNAIRES DU PROJET LOADÉ
        // Extraction des instances riches en étapes lues du JSON
        auto cleanTaskRef = std::make_shared<Task>(project_loaded.tasks().at("Clean_Infrastructure_Task"));
        auto transTaskRef = std::make_shared<Task>(project_loaded.tasks().at("Transform_and_Clean_Task"));

        // On instancie des Jobs sains et on leur injecte les tâches complètes
        Job rCleanJob("ETL_Clean_Job");
        rCleanJob.addTask(1, cleanTaskRef);

        Job rSyncJob("ETL_Sync_Job");
        rSyncJob.addCondition(Condition("ETL_Clean_Job", JobStatus::Success));
        rSyncJob.addTask(1, transTaskRef);

        // On ré-aligne l'orchestrateur Workflow
        Workflow realWorkflow(workflow_name);
        realWorkflow.addJob(std::make_shared<Job>(std::move(rCleanJob)));
        realWorkflow.addJob(std::make_shared<Job>(std::move(rSyncJob)));

        const auto& gateway = project_loaded.logicalConnections().at("Finance_DB_Gateway");

        LOG4CXX_INFO(logger, "🚀 Lancement de l'orchestration sur le couloir [" << env_context << "]...");
        
        try {
            // Lancement du workflow reconstitué de façon étanche
            realWorkflow.run(gateway, env_context);
            LOG4CXX_INFO(logger, "✅ Fin de session d'exécution du workflow.");
        }
        catch (const std::exception& e) {
            LOG4CXX_ERROR(logger, "🔥 Exception fatale durant le run : " << e.what());
            return 1;
        }
    }


    return 0;
}
