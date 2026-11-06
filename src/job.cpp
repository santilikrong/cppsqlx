#include "cppsqlx/job.hpp"
#include "cppsqlx/condition.hpp"
#include "cppsqlx/task.hpp"
#include <cppsqlx/connection.hpp>
#include <iostream>
#include "cppsqlx/logger.hpp"

namespace cppsqlx
{

bool Job::canExecute(const std::map<std::string, JobResult>& workflowContext) const
{
    for (const auto& req : requirements_) {
        if (!req.isSatisfied(workflowContext)) {
            return false;
        }
    }
    return true;
}

JobResult Job::execute(std::shared_ptr<Connection> connection)
{
    // 1. Récupération de l'instance du logger dédié aux Jobs
    auto logger = cppsqlx::Logger::instance().getLogger("cppsqlx.job");

    LOG4CXX_INFO(logger, "[Job: " << name_ << "] Démarrage de l'exécution du bloc de tâches (" << tasks_.size() << " tâches enregistrées)");

    // Itération séquentielle et ordonnée sur les tâches du Job
    for (const auto& [pos, taskPtr] : tasks_) {
        if (taskPtr) {
            
            // 2. Log d'annonce avant le traitement physique
            LOG4CXX_INFO(logger, "[Job: " << name_ << "] Lance la tâche '" << taskPtr->name());

            // Exécution de la tâche et capture d'une éventuelle erreur
            auto taskRes = taskPtr->execute(connection);
            
            if (!taskRes) {
                // 3. Log d'erreur critique pour identifier instantanément le maillon faible
                LOG4CXX_ERROR(logger, "[Job: " << name_ << "] 🔥 ÉCHEC CRITIQUE à la position " << pos 
                    << " lors de l'exécution de '" << taskPtr->name() << "' -> Erreur : " << taskRes.error());

                // Interruption immédiate du Job en cas d'échec d'une tâche
                return JobResult{
                    name_, 
                    JobStatus::Failed, 
                    "Job failed at task pos " + std::to_string(pos) + ": " + taskRes.error()
                };
            }

            // 4. Log de validation en cas de succès de l'étape
            LOG4CXX_INFO(logger, "[Job: " << name_ << "] Tâche '" << taskPtr->name());
        }
    }
    
    LOG4CXX_INFO(logger, "[Job: " << name_ << "] Exécution globale complétée avec SUCCÈS.");
    return JobResult{name_, JobStatus::Success, "Job executed successfully"};
}

json Job::toJSON() const
{
    json j;
    j["name"] = name_;
    j["x"] = x_;
    j["y"] = y_;

    json jsonReqs = json::array();
    for (const auto& cond : requirements_) {
        jsonReqs.push_back(cond.toJSON());
    }
    j["requirements"] = jsonReqs;

    json jsonTasks = json::array();
    for (const auto& [pos, taskPtr] : tasks_) {
        if (taskPtr) {
            json taskItem;
            taskItem["pos"] = pos;
            taskItem["task"] = taskPtr->toJSON();
            jsonTasks.push_back(taskItem);
        }
    }
    j["tasks"] = jsonTasks;

    return json{ {"job", j} };
}

std::expected<Job, std::string> Job::fromJSON(const json& j)
{
    const json& data = j.contains("job") ? j["job"] : j;
    try {
        Job job(data.at("name").get<std::string>());
        job.x_ = data.value("x", 0);
        job.y_ = data.value("y", 0);

        // 🟢 RECONSTRUCTION SÉCURISÉE DES TÂCHES ET DE LEURS ÉTAPES INTERNES
        if (data.contains("tasks") && data["tasks"].is_object()) {
            for (const auto& [key, taskJson] : data["tasks"].items()) {
                int pos = std::stoi(key);
                
                // Appel récursif au fromJSON de Task pour récupérer steps_
                auto taskRes = Task::fromJSON(taskJson);
                if (!taskRes) return std::unexpected("Job::fromJSON (task) -> " + taskRes.error());
                
                job.addTask(pos, std::make_shared<Task>(std::move(*taskRes)));
            }
        }

        // Reconstruction des conditions de dépendances
        if (data.contains("conditions") && data["conditions"].is_array()) {
            for (const auto& condJson : data["conditions"]) {
                auto condRes = Condition::fromJSON(condJson);
                if (condRes) job.addCondition(std::move(*condRes));
            }
        }

        return job;
    }
    catch (const std::exception& e) {
        return std::unexpected(std::string("Job::fromJSON failed: ") + e.what());
    }
}


} // namespace cppsqlx
