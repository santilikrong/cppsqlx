#ifndef CPPSQLX_WORKFLOW_HPP
#define CPPSQLX_WORKFLOW_HPP

#include <iostream>
#include <thread>
#include <string>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <expected>
#include <functional>
#include <cppsqlx/json_concepts.hpp>
#include <cppsqlx/connectioncontext.hpp>
#include <cppsqlx/job.hpp>
#include <cppsqlx/job_result.hpp>
#include <cppsqlx/logicalconnection.hpp>

namespace cppsqlx
{

    class Workflow
    {
    private:
        std::string name_;
        ConnectionContext context_;
        std::list<std::shared_ptr<Job>> jobs_;

    public:
        // ---------- Constructors & Destructor ----------
        explicit Workflow(std::string name) : name_(std::move(name)) {}
        ~Workflow() = default;

        // ---------- Accessors ----------
        const std::string &name() const noexcept { return name_; }
        const ConnectionContext &context() const noexcept { return context_; }
        const std::list<std::shared_ptr<Job>> &jobs() const noexcept { return jobs_; }

        // ---------- Mutators & Modifiers ----------
        void setContext(ConnectionContext context) { context_ = std::move(context); }
        void addJob(std::shared_ptr<Job> job) { jobs_.push_back(std::move(job)); }


using JobStateCallback = std::function<void(const std::string&, JobStatus, const std::string&)>;

        /**
         * @brief Exécute le workflow sur un couloir spécifique d'une connexion logique.
         * @param logicalConn La passerelle réseau logique (ex: Finance_DB_Gateway).
         * @param environmentContext Le couloir d'exécution (ex: "DEV", "TEST", "PROD").
         */
void run(const LogicalConnection& logicalConn, 
         const std::string& environmentContext, 
         JobStateCallback onJobStateChange = nullptr);

        // ---------- JSON Concepts Contract ----------
        [[nodiscard]] json toJSON() const;
        static std::expected<Workflow, std::string> fromJSON(const json &j);
    };

} // namespace cppsqlx

#endif // CPPSQLX_WORKFLOW_HPP
