#include <cppsqlx/dataintegrationproject.hpp>
#include <filesystem>
#include <fstream>

namespace cppsqlx
{

    void DataIntegrationProject::resolveDependencies()
    {
        // 1. On parcourt et résout les Jobs au sein des Workflows
        for (auto &[wfName, wf] : workflows_)
        {
            for (const auto &jobPtr : wf.jobs())
            {
                if (jobPtr && jobs_.contains(jobPtr->name()))
                {
                    // Le conteneur renvoie du const, on cast la référence pour mettre à jour le pointeur partagé
                    auto &mutableJobPtr = const_cast<std::shared_ptr<Job> &>(jobPtr);
                    mutableJobPtr = std::make_shared<Job>(jobs_.at(jobPtr->name()));
                }
            }
        }

        // 2. On parcourt et résout les Tâches au sein des Jobs
        for (auto &[jobName, job] : jobs_)
        {
            for (const auto &[pos, taskPtr] : job.tasks())
            {
                if (taskPtr && tasks_.contains(taskPtr->name()))
                {
                    // On cast la référence du pointeur de tâche pour lui injecter l'instance riche du projet
                    auto &mutableTaskPtr = const_cast<std::shared_ptr<Task> &>(taskPtr);
                    mutableTaskPtr = std::make_shared<Task>(tasks_.at(taskPtr->name()));
                }
            }
        }
    }

    std::expected<void, std::string> DataIntegrationProject::saveToDisk(
        int indent,
        const std::string &customPath,
        const std::string &customFilename) const
    {
        // 1. Détermination du dossier cible (priorité au chemin personnalisé s'il est fourni)
        std::string finalFolderPath = customPath.empty() ? path_ : customPath;

        if (finalFolderPath.empty())
        {
            return std::unexpected("DataIntegrationProject::saveToDisk failed: No target directory specified (path_ is empty)");
        }

        try
        {
            std::filesystem::path dirPath(finalFolderPath);

            // 2. Création physique de toute l'arborescence s'il n'existe pas sur Fedora
            std::filesystem::create_directories(dirPath);

            // 3. Construction du chemin complet du fichier final
            std::filesystem::path filePath = dirPath / customFilename;

            std::ofstream file(filePath);
            if (!file.is_open())
            {
                return std::unexpected("Failed to open file for writing at: " + filePath.string());
            }

            // 4. Écriture du JSON selon l'indentation choisie (-1 = compact, >0 = indenté)
            file << cppsqlx::io::dump(*this, indent);
            file.close();

            return {};
        }
        catch (const std::exception &e)
        {
            return std::unexpected(std::string("DataIntegrationProject::saveToDisk exception: ") + e.what());
        }
    }

    std::expected<DataIntegrationProject, std::string> DataIntegrationProject::loadFromDisk(
        const std::string &filePath)
    {
        if (filePath.empty())
        {
            return std::unexpected("DataIntegrationProject::loadFromDisk failed: File path is empty");
        }

        try
        {
            std::filesystem::path targetFile(filePath);

            // 1. Vérification de sécurité physique
            if (!std::filesystem::exists(targetFile))
            {
                return std::unexpected("Project file does not exist: " + filePath);
            }

            if (!std::filesystem::is_regular_file(targetFile))
            {
                return std::unexpected("Specified path is not a valid file: " + filePath);
            }

            // 2. Lecture du flux de données
            std::ifstream file(targetFile);
            if (!file.is_open())
            {
                return std::unexpected("Failed to open project file for reading: " + filePath);
            }

            // 3. Parsing de la chaîne JSON globale
            nlohmann::json parsedJson;
            file >> parsedJson;
            file.close();

            // 4. Reconstruction polymorphe et récursive de tout l'arbre du projet
            return DataIntegrationProject::fromJSON(parsedJson);
        }
        catch (const std::exception &e)
        {
            return std::unexpected(std::string("DataIntegrationProject::loadFromDisk exception: ") + e.what());
        }
    }

    json DataIntegrationProject::toJSON() const
    {
        json j;
        j["name"] = name_;
        j["path"] = path_;

        // 1. Physical Connections
        json jsonPhys = json::object();
        for (const auto &[name, conn] : physicalConnections_)
        {
            jsonPhys[name] = conn.toJSON();
        }
        j["physical_connections"] = jsonPhys;

        // 2. Logical Connections
        json jsonLog = json::object();
        for (const auto &[name, conn] : logicalConnections_)
        {
            jsonLog[name] = conn.toJSON();
        }
        j["logical_connections"] = jsonLog;

        // 3. Queries (std::variant)
        json jsonQueries = json::object();
        for (const auto &[name, qVariant] : queries_)
        {
            std::visit(overloads{[&](const SelectQuery &q)
                                 { jsonQueries[name] = {{"type", "SelectQuery"}, {"data", q.toJSON()}}; },
                                 [&](const InsertQuery &q)
                                 { jsonQueries[name] = {{"type", "InsertQuery"}, {"data", q.toJSON()}}; },
                                 [&](const CTASQuery &q)
                                 { jsonQueries[name] = {{"type", "CTASQuery"}, {"data", q.toJSON()}}; },
                                 [&](const UpdateQuery &q)
                                 { jsonQueries[name] = {{"type", "UpdateQuery"}, {"data", q.toJSON()}}; },
                                 [&](const DeleteQuery &q)
                                 { jsonQueries[name] = {{"type", "DeleteQuery"}, {"data", q.toJSON()}}; },
                                 [&](const DropTableQuery &q)
                                 { jsonQueries[name] = {{"type", "DropTableQuery"}, {"data", q.toJSON()}}; }},
                       qVariant);
        }
        j["queries"] = jsonQueries;

        // 4. Datasets
        json jsonDS = json::object();
        for (const auto &[name, ds] : datasets_)
        {
            jsonDS[name] = ds.toJSON();
        }
        j["datasets"] = jsonDS;

        // 5. Tasks
        json jsonTasks = json::object();
        for (const auto &[name, t] : tasks_)
        {
            jsonTasks[name] = t.toJSON();
        }
        j["tasks"] = jsonTasks;

        // 6. Jobs
        json jsonJobs = json::object();
        for (const auto &[name, jb] : jobs_)
        {
            jsonJobs[name] = jb.toJSON();
        }
        j["jobs"] = jsonJobs;

        // 7. Workflows
        json jsonWfs = json::object();
        for (const auto &[name, wf] : workflows_)
        {
            jsonWfs[name] = wf.toJSON();
        }
        j["workflows"] = jsonWfs;

        return json{{"dataintegrationproject", j}};
    }

    std::expected<DataIntegrationProject, std::string> DataIntegrationProject::fromJSON(const json &j)
    {
        const json &data = j.contains("dataintegrationproject") ? j["dataintegrationproject"] : j;
        try
        {
            DataIntegrationProject proj;
            proj.name_ = data.at("name").get<std::string>();
            proj.path_ = data.at("path").get<std::string>();

            // 1. Physical Connections
            if (data.contains("physical_connections") && data["physical_connections"].is_object())
            {
                for (const auto &[key, valJson] : data["physical_connections"].items())
                {
                    auto res = PhysicalConnection::fromJSON(valJson);
                    if (!res)
                        return std::unexpected("Project::fromJSON (physical_connections) -> " + res.error());
                    proj.physicalConnections_.insert_or_assign(key, std::move(*res));
                }
            }

            // 2. Logical Connections
            if (data.contains("logical_connections") && data["logical_connections"].is_object())
            {
                for (const auto &[key, valJson] : data["logical_connections"].items())
                {
                    auto res = LogicalConnection::fromJSON(valJson);
                    if (!res)
                        return std::unexpected("Project::fromJSON (logical_connections) -> " + res.error());
                    proj.logicalConnections_.insert_or_assign(key, std::move(*res));
                }
            }

            // 3. Queries (std::variant dynamic dispatch)
            if (data.contains("queries") && data["queries"].is_object())
            {
                for (const auto &[key, valJson] : data["queries"].items())
                {
                    std::string type = valJson.at("type").get<std::string>();
                    const json &qData = valJson.at("data");

                    if (type == "SelectQuery")
                    {
                        auto res = SelectQuery::fromJSON(qData);
                        if (!res)
                            return std::unexpected("Project::fromJSON (query " + key + ") -> " + res.error());
                        proj.queries_.insert_or_assign(key, std::move(*res));
                    }
                    else if (type == "InsertQuery")
                    {
                        auto res = InsertQuery::fromJSON(qData);
                        if (!res)
                            return std::unexpected("Project::fromJSON (query " + key + ") -> " + res.error());
                        proj.queries_.insert_or_assign(key, std::move(*res));
                    }
                    // Même correction de tolérance de casse pour le registre racine :
                    if (type == "CTASQuery" || type == "ctasquery")
                    {
                        auto res = CTASQuery::fromJSON(qData);
                        if (!res)
                            return std::unexpected("Project::fromJSON (query " + key + ") -> " + res.error());
                        proj.queries_.insert_or_assign(key, std::move(*res));
                    }

                    else if (type == "UpdateQuery")
                    {
                        auto res = UpdateQuery::fromJSON(qData);
                        if (!res)
                            return std::unexpected("Project::fromJSON (query " + key + ") -> " + res.error());
                        proj.queries_.insert_or_assign(key, std::move(*res));
                    }
                    else if (type == "DropTableQuery")
                    {
                        auto res = DropTableQuery::fromJSON(qData);
                        if (!res)
                            return std::unexpected("Project::fromJSON (query " + key + ") -> " + res.error());
                        proj.queries_.insert_or_assign(key, std::move(*res));
                    }
                    else if (type == "DeleteQuery")
                    {
                        auto res = DeleteQuery::fromJSON(qData);
                        if (!res)
                            return std::unexpected("Project::fromJSON (query " + key + ") -> " + res.error());
                        proj.queries_.insert_or_assign(key, std::move(*res));
                    }
                }
            }

            // 4. Datasets
            if (data.contains("datasets") && data["datasets"].is_object())
            {
                for (const auto &[key, valJson] : data["datasets"].items())
                {
                    auto res = Dataset::fromJSON(valJson);
                    if (!res)
                        return std::unexpected("Project::fromJSON (datasets) -> " + res.error());
                    proj.datasets_.insert_or_assign(key, std::move(*res));
                }
            }

            // 5. Tasks
            if (data.contains("tasks") && data["tasks"].is_object())
            {
                for (const auto &[key, valJson] : data["tasks"].items())
                {
                    auto res = Task::fromJSON(valJson);
                    if (!res)
                        return std::unexpected("Project::fromJSON (tasks) -> " + res.error());
                    proj.tasks_.insert_or_assign(key, std::move(*res));
                }
            }

            // 6. Jobs
            if (data.contains("jobs") && data["jobs"].is_object())
            {
                for (const auto &[key, valJson] : data["jobs"].items())
                {
                    auto res = Job::fromJSON(valJson);
                    if (!res)
                        return std::unexpected("Project::fromJSON (jobs) -> " + res.error());
                    proj.jobs_.insert_or_assign(key, std::move(*res));
                }
            }

            // 7. Workflows
            if (data.contains("workflows") && data["workflows"].is_object())
            {
                for (const auto &[key, valJson] : data["workflows"].items())
                {
                    auto res = Workflow::fromJSON(valJson);
                    if (!res)
                        return std::unexpected("Project::fromJSON (workflows) -> " + res.error());
                    proj.workflows_.insert_or_assign(key, std::move(*res));
                }
            }

            return proj;
        }
        catch (const std::exception &e)
        {
            return std::unexpected(std::string("DataIntegrationProject::fromJSON failed: ") + e.what());
        }
    }

} // namespace cppsqlx
