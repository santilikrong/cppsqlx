#include <iostream>
#include <memory>
#include <map>
#include <list>
#include <cppsqlx/job.hpp>
#include <cppsqlx/condition.hpp>
#include <cppsqlx/workflow.hpp>
#include "cppsqlx/connection.hpp"

using namespace cppsqlx;

int main() 
{
    // 1. Instanciation des Jobs d'extraction (Pas de prérequis)
    auto jobA = std::make_shared<Job>("Extract_A");
    auto jobB = std::make_shared<Job>("Extract_B");

    // 2. Instanciation du Job de Transformation (Dépend de A ET B)
    auto jobTransform = std::make_shared<Job>("Transform_Data");
    
    // Déclaration des dépendances multiples
    Condition depA("Extract_A", JobStatus::Success);
    Condition depB("Extract_B", JobStatus::Success);
    
    // On exige la validation de toutes les conditions (AND implicite ou explicite)
    jobTransform->addCondition(depA);
    jobTransform->addCondition(depB);

    // 3. Instanciation du Job final de chargement (Dépend de Transform)
    auto jobLoad = std::make_shared<Job>("Load_To_Warehouse");
    jobLoad->addCondition(Condition("Transform_Data", JobStatus::Success));

    // 4. Ajout au Workflow orchestrateur
    Workflow monPipeline("Pipeline_ETL_Global");
    monPipeline.addJob(jobA);
    monPipeline.addJob(jobB);
    monPipeline.addJob(jobTransform);
    monPipeline.addJob(jobLoad);


    // Même si la chaîne est vide, l'objet existe en mémoire pour valider le pointeur du Workflow.
    cppsqlx::Connection conn(""); 

    // Lancement de l'orchestration orientée données
    std::cout << "--- Démarrage de l'orchestration multi-dépendances ---" << std::endl;
    auto connPtr = std::shared_ptr<cppsqlx::Connection>(&conn, [](cppsqlx::Connection*){/* no-op */});
    //monPipeline.run(connPtr);
    std::cout << "--- Fin de l'exécution du workflow ---" << std::endl;

    return 0;
}
