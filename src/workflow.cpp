#include <cppsqlx/workflow.hpp>
#include "cppsqlx/logger.hpp"

namespace cppsqlx
{

    json Workflow::toJSON() const
    {
        json jsonJobs = json::array();
        for (const auto &jobPtr : jobs_)
        {
            if (jobPtr)
            {
                jsonJobs.push_back(jobPtr->toJSON());
            }
        }

        return json{
            {"workflow", {{"name", name_}, {"context", context_.toJSON()}, {"jobs", jsonJobs}}}};
    }

    std::expected<Workflow, std::string> Workflow::fromJSON(const json &j)
    {
        const json &data = j.contains("workflow") ? j["workflow"] : j;

        try
        {
            std::string name = data.at("name").get<std::string>();
            Workflow wf(std::move(name));

            // Reconstruction de l'objet ConnectionContext
            if (data.contains("context"))
            {
                auto ctxRes = ConnectionContext::fromJSON(data["context"]);
                if (!ctxRes)
                    return std::unexpected("Workflow::fromJSON -> " + ctxRes.error());
                wf.context_ = std::move(*ctxRes);
            }

            // Reconstruction de la liste de Jobs (std::list<shared_ptr<Job>>)
            if (data.contains("jobs") && data["jobs"].is_array())
            {
                for (const auto &jobJson : data["jobs"])
                {
                    auto jobRes = Job::fromJSON(jobJson);
                    if (!jobRes)
                        return std::unexpected("Workflow::fromJSON (jobs) -> " + jobRes.error());
                    wf.jobs_.push_back(std::make_shared<Job>(std::move(*jobRes)));
                }
            }

            return wf;
        }
        catch (const std::exception &e)
        {
            return std::unexpected(std::string("Workflow::fromJSON failed: ") + e.what());
        }
    }

    void Workflow::run(const LogicalConnection &logicalConn, const std::string &environmentContext)
    {
        // 1. Résolution dynamique de la configuration physique du couloir
        auto physConfigRes = logicalConn.getConfiguration(environmentContext);
        if (!physConfigRes)
        {
            std::cerr << "[Workflow ERR] Impossible de démarrer : " << physConfigRes.error() << std::endl;
            return;
        }
        const auto &physConfig = *physConfigRes;

        // 2. Construction de la chaîne de connexion PostgreSQL brute
        std::string connStr = "dbname=" + physConfig.dbname_ +
                              " user=" + physConfig.dbuser_ +
                              " host=" + physConfig.dbhost_ +
                              " port=" + std::to_string(physConfig.dbport_) +
                              " sslmode=disable";

        // 3. Initialisation de la connexion pour cette session de Workflow
        auto connection = std::make_shared<Connection>(connStr);
        if (auto connectRes = connection->connect(); !connectRes)
        {
            std::cerr << "[Workflow ERR] Échec de connexion au cluster physique : " << connectRes.error() << std::endl;
            return;
        }

        // --- À partir d'ici, le moteur de threads s'exécute normalement sur cette connexion ---
        std::map<std::string, JobResult> workflowContext;
        std::list<std::shared_ptr<Job>> remainingJobs = jobs_;
        std::mutex contextMutex;
        std::condition_variable cv;
        size_t activeWorkers = 0;
        std::vector<std::jthread> workerThreads;

        std::cout << "[Workflow] Démarrage asynchrone sur [" << logicalConn.name() << " -> " << environmentContext << "]" << std::endl;

        while (true)
        {
            std::vector<std::shared_ptr<Job>> jobsToLaunch;

            {
                std::lock_guard<std::mutex> lock(contextMutex);
                for (auto it = remainingJobs.begin(); it != remainingJobs.end();)
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

            for (const auto &jobPtr : jobsToLaunch)
            {
                {
                    std::lock_guard<std::mutex> lock(contextMutex);
                    activeWorkers++;
                }

                // Chaque job reçoit la connexion résolue dynamiquement
                workerThreads.emplace_back([jobPtr, connection, &workflowContext, &contextMutex, &cv, &activeWorkers]()
                                           {
                JobResult result = jobPtr->execute(connection);
                {
                    std::lock_guard<std::mutex> lock(contextMutex);
                    workflowContext[jobPtr->name()] = std::move(result);
                    activeWorkers--;
                }
                cv.notify_one(); });
            }

            std::unique_lock<std::mutex> lock(contextMutex);
            if (remainingJobs.empty() && activeWorkers == 0)
                break;

            if (jobsToLaunch.empty())
            {
                if (activeWorkers == 0 && !remainingJobs.empty())
                {
                    std::cerr << "[CRITICAL] Deadlock détecté !" << std::endl;
                    break;
                }
                cv.wait(lock);
            }
        }

        auto closeRes = connection->close();
        if (!closeRes)
        {
            auto logger = cppsqlx::Logger::instance().getLogger("cppsqlx.workflow");
            LOG4CXX_WARN(logger, "Erreur lors de la fermeture de la connexion physique : " << closeRes.error());
        }
        std::cout << "[Workflow] Exécution terminée pour l'environnement : " << environmentContext << std::endl;
    }

} // namespace cppsqlx
