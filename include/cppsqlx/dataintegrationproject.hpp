#ifndef CPPSQLX_DATA_INTEGRATION_PROJECT_HPP
#define CPPSQLX_DATA_INTEGRATION_PROJECT_HPP

#include <string>
#include <map>
#include <memory>
#include <expected>

#include <cppsqlx/json_concepts.hpp>
#include <cppsqlx/physicalconnection.hpp>
#include <cppsqlx/logicalconnection.hpp>
#include <cppsqlx/unit_select.hpp> // Contient l'alias QueryVariant
#include <cppsqlx/dataset.hpp>
#include <cppsqlx/task.hpp>
#include <cppsqlx/job.hpp>
#include <cppsqlx/workflow.hpp>

namespace cppsqlx
{

    class DataIntegrationProject
    {
    private:
        std::string name_;
        std::string path_;

        std::map<std::string, PhysicalConnection> physicalConnections_;
        std::map<std::string, LogicalConnection> logicalConnections_;
        std::map<std::string, QueryVariant> queries_;
        std::map<std::string, Dataset> datasets_;
        std::map<std::string, Task> tasks_;
        std::map<std::string, Job> jobs_;
        std::map<std::string, Workflow> workflows_;

    public:
        DataIntegrationProject() = default;
        DataIntegrationProject(std::string name, std::string path)
            : name_(std::move(name)), path_(std::move(path)) {}
        ~DataIntegrationProject() = default;

        void resolveDependencies();

        // ---------- Accessors ----------
        const std::string &name() const noexcept { return name_; }
        const std::string &path() const noexcept { return path_; }

        const std::map<std::string, PhysicalConnection> &physicalConnections() const noexcept { return physicalConnections_; }
        const std::map<std::string, LogicalConnection> &logicalConnections() const noexcept { return logicalConnections_; }
        const std::map<std::string, QueryVariant> &queries() const noexcept { return queries_; }
        const std::map<std::string, Dataset> &datasets() const noexcept { return datasets_; }
        const std::map<std::string, Task> &tasks() const noexcept { return tasks_; }
        const std::map<std::string, Job> &jobs() const noexcept { return jobs_; }
        const std::map<std::string, Workflow> &workflows() const noexcept { return workflows_; }

        std::map<std::string, PhysicalConnection> &physicalConnections() noexcept { return physicalConnections_; }
        std::map<std::string, LogicalConnection> &logicalConnections() noexcept { return logicalConnections_; }
        std::map<std::string, QueryVariant> &queries() noexcept { return queries_; }
        std::map<std::string, Dataset> &datasets() noexcept { return datasets_; }
        std::map<std::string, Task> &tasks() noexcept { return tasks_; }
        std::map<std::string, Job> &jobs() noexcept { return jobs_; }
        std::map<std::string, Workflow> &workflows() noexcept { return workflows_; }


        // ---------- Modifiers / Registration ----------
        void setName(std::string name) { name_ = std::move(name); }
        void setPath(std::string path) { path_ = std::move(path); }

        void addPhysicalConnection(PhysicalConnection conn) { physicalConnections_.insert_or_assign(conn.name_, std::move(conn)); }
        void addLogicalConnection(LogicalConnection conn) { logicalConnections_.insert_or_assign(conn.name(), std::move(conn)); }
        void addDataset(Dataset ds) { datasets_.insert_or_assign(ds.name(), std::move(ds)); }
        void addTask(Task t) { tasks_.insert_or_assign(t.name(), std::move(t)); }
        void addJob(Job j) { jobs_.insert_or_assign(j.name(), std::move(j)); }
        void addWorkflow(Workflow w) { workflows_.insert_or_assign(w.name(), std::move(w)); }

        // Pour les requêtes, on extrait le nom selon le type stocké dans le variant
        void addQuery(std::string queryName, QueryVariant q) { queries_.insert_or_assign(std::move(queryName), std::move(q)); }

        /**
         * @brief Sauvegarde le projet sur le disque de manière paramétrable.
         * @param indent Niveau d'indentation du JSON (-1 pour un format compact sans espaces).
         * @param customPath Chemin alternatif optionnel (si vide, utilise le path_ interne du projet).
         * @param customFilename Nom de fichier alternatif (par défaut "project.json").
         * @return Un std::expected indiquant le succès ou le message d'erreur.
         */
        [[nodiscard]] std::expected<void, std::string> saveToDisk(
            int indent = -1,
            const std::string &customPath = "",
            const std::string &customFilename = "project.json") const;

        /**
         * @brief Charge et reconstruit un projet complet depuis un fichier sur le disque.
         * @param filePath Chemin complet vers le fichier JSON du projet (ex: "/path/to/project.json").
         * @return Un std::expected contenant le projet reconstruit ou un message d'erreur.
         */
        [[nodiscard]] static std::expected<DataIntegrationProject, std::string> loadFromDisk(
            const std::string &filePath);

        // ---------- JSON Concepts Contract ----------
        [[nodiscard]] json toJSON() const;
        static std::expected<DataIntegrationProject, std::string> fromJSON(const json &j);
    };

} // namespace cppsqlx

#endif // CPPSQLX_DATA_INTEGRATION_PROJECT_HPP
