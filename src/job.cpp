#include "cppsqlx/job.hpp"
#include "cppsqlx/condition.hpp"
#include "cppsqlx/task.hpp"
#include <cppsqlx/connection.hpp>

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
    // Itération séquentielle et ordonnée sur les tâches du Job
    for (const auto& [pos, taskPtr] : tasks_) {
        if (taskPtr) {
            // Exécution de la tâche et capture d'une éventuelle erreur
            auto taskRes = taskPtr->execute(connection);
            
            if (!taskRes) {
                // Interruption immédiate du Job en cas d'échec d'une tâche
                return JobResult{
                    name_, 
                    JobStatus::Failed, 
                    "Job failed at task pos " + std::to_string(pos) + ": " + taskRes.error()
                };
            }
        }
    }
    
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
        Job job;
        job.name_ = data.at("name").get<std::string>();
        job.x_ = data.at("x").get<int>();
        job.y_ = data.at("y").get<int>();

        if (data.contains("requirements") && data["requirements"].is_array()) {
            for (const auto& condJson : data["requirements"]) {
                auto condRes = cppsqlx::Condition::fromJSON(condJson);
                if (!condRes) return std::unexpected("Job::fromJSON (requirements) -> " + condRes.error());
                job.requirements_.push_back(std::move(*condRes));
            }
        }

        if (data.contains("tasks") && data["tasks"].is_array()) {
            for (const auto& taskItemJson : data["tasks"]) {
                int pos = taskItemJson.at("pos").get<int>();
                
                auto taskRes = cppsqlx::Task::fromJSON(taskItemJson.at("task"));
                if (!taskRes) return std::unexpected("Job::fromJSON (tasks) -> " + taskRes.error());
                
                job.tasks_.insert_or_assign(pos, std::make_shared<cppsqlx::Task>(std::move(*taskRes)));
            }
        }

        return job;
    }
    catch (const std::exception& e) {
        return std::unexpected(std::string("Job::fromJSON failed: ") + e.what());
    }
}

} // namespace cppsqlx
