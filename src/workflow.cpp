#include <cppsqlx/workflow.hpp>

namespace cppsqlx
{

json Workflow::toJSON() const
{
    json jsonJobs = json::array();
    for (const auto& jobPtr : jobs_) {
        if (jobPtr) {
            jsonJobs.push_back(jobPtr->toJSON());
        }
    }

    return json{
        {"workflow", {
            {"name", name_},
            {"context", context_.toJSON()},
            {"jobs", jsonJobs}
        }}
    };
}

std::expected<Workflow, std::string> Workflow::fromJSON(const json& j)
{
    const json& data = j.contains("workflow") ? j["workflow"] : j;

    try {
        std::string name = data.at("name").get<std::string>();
        Workflow wf(std::move(name));

        // Reconstruction de l'objet ConnectionContext
        if (data.contains("context")) {
            auto ctxRes = ConnectionContext::fromJSON(data["context"]);
            if (!ctxRes) return std::unexpected("Workflow::fromJSON -> " + ctxRes.error());
            wf.context_ = std::move(*ctxRes);
        }

        // Reconstruction de la liste de Jobs (std::list<shared_ptr<Job>>)
        if (data.contains("jobs") && data["jobs"].is_array()) {
            for (const auto& jobJson : data["jobs"]) {
                auto jobRes = Job::fromJSON(jobJson);
                if (!jobRes) return std::unexpected("Workflow::fromJSON (jobs) -> " + jobRes.error());
                wf.jobs_.push_back(std::make_shared<Job>(std::move(*jobRes)));
            }
        }

        return wf;
    }
    catch (const std::exception& e) {
        return std::unexpected(std::string("Workflow::fromJSON failed: ") + e.what());
    }
}


void Workflow::run(std::shared_ptr<Connection> connection) // 🟢 Reçoit la connexion
{
    std::map<std::string, JobResult> workflowContext;
    std::list<std::shared_ptr<Job>> remainingJobs = jobs_;
    
    std::mutex contextMutex;
    std::condition_variable cv;
    
    size_t activeWorkers = 0; 
    std::vector<std::jthread> workerThreads;

    std::cout << "[Workflow] Démarrage de l'exécution parallèle pour : " << name_ << std::endl;

    while (true)
    {
        std::vector<std::shared_ptr<Job>> jobsToLaunch;

        {
            std::lock_guard<std::mutex> lock(contextMutex);
            for (auto it = remainingJobs.begin(); it != remainingJobs.end(); )
            {
                if ((*it)->canExecute(workflowContext))
                {
                    jobsToLaunch.push_back(*it);
                    it = remainingJobs.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }

        for (const auto& jobPtr : jobsToLaunch)
        {
            {
                std::lock_guard<std::mutex> lock(contextMutex);
                activeWorkers++;
            }

            std::cout << "[Workflow] Lancement asynchrone du Job : " << jobPtr->name() << std::endl;

            // Transmet la connexion à la lambda capturée
            workerThreads.emplace_back([jobPtr, connection, &workflowContext, &contextMutex, &cv, &activeWorkers]() {
                
                // 🟢 Appel correct de execute avec la connexion passée
                JobResult result = jobPtr->execute(connection);

                {
                    std::lock_guard<std::mutex> lock(contextMutex);
                    workflowContext[jobPtr->name()] = std::move(result);
                    activeWorkers--;
                }
                cv.notify_one(); 
            });
        }

        std::unique_lock<std::mutex> lock(contextMutex);
        if (remainingJobs.empty() && activeWorkers == 0)
        {
            break; 
        }

        if (jobsToLaunch.empty())
        {
            if (activeWorkers == 0 && !remainingJobs.empty())
            {
                std::cerr << "[🔥 CRITICAL] Impasse détectée (Deadlock) ! " << remainingJobs.size() << " jobs bloqués." << std::endl;
                break;
            }
            cv.wait(lock);
        }
    }

    std::cout << "[Workflow] Fin de l'exécution globale avec succès." << std::endl;
}

} // namespace cppsqlx
