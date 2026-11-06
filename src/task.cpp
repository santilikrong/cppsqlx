#include <cppsqlx/task.hpp>
#include <iostream>
#include "cppsqlx/logger.hpp"

namespace cppsqlx
{

    json Task::toJSON() const
    {
        json jsonSteps = json::array();
        for (const auto &[index, unit] : steps_)
        {
            json item;
            item["step_index"] = index;
            item["unit_select"] = unit.toJSON();
            jsonSteps.push_back(item);
        }

        return json{
            {"task", {{"name", name_}, {"steps", jsonSteps}}}};
    }

    std::expected<Task, std::string> Task::fromJSON(const json &j)
    {
        const json &data = j.contains("task") ? j["task"] : j;
        try
        {
            Task task;
            task.name_ = data.at("name").get<std::string>();

            if (data.contains("steps") && data["steps"].is_array())
            {
                for (const auto &stepJson : data["steps"])
                {
                    int index = stepJson.at("step_index").get<int>();

                    auto unitRes = UnitSelect::fromJSON(stepJson.at("unit_select"));
                    if (!unitRes)
                        return std::unexpected("Task::fromJSON (steps) -> " + unitRes.error());

                    task.steps_.insert_or_assign(index, std::move(*unitRes));
                }
            }

            return task;
        }
        catch (const std::exception &e)
        {
            return std::unexpected(std::string("Task::fromJSON failed: ") + e.what());
        }
    }

    std::expected<void, std::string> Task::execute(std::shared_ptr<Connection> connection)
    {
        auto logger = cppsqlx::Logger::instance().getLogger("cppsqlx.task");
        if (!connection || !connection->isConnected())
        {
            return std::unexpected("Task::execute failed: Database connection is null or closed");
        }

        // Itération séquentielle et ordonnée sur les étapes (UnitSelect) de la tâche
        for (const auto &[stepIndex, unit] : steps_)
        {
            // 1. Extraction du SQL polymorphe depuis le std::variant (Select, Insert, CTAS, Update, Delete)
            std::string sql = unit.toSQL();

            if (sql.empty())
            {
                return std::unexpected("Task '" + name_ + "' step " + std::to_string(stepIndex) + ": SQL query is empty");
            }

            try
            {
                // 2. Création du Statement via votre API cppsqlx
                auto stmt = connection->createStatement();
                if (!stmt)
                {
                    return std::unexpected("Failed to create SQL statement for step " + std::to_string(stepIndex));
                }

                // 3. Exécution de la requête brute sur PostgreSQL
                auto res = stmt->execute(sql);

                if (!res)
                {
                    return std::unexpected("SQL Error at step " + std::to_string(stepIndex) + " (" + unit.name + "): " + res.error());
                }

        if (res->rows() > 0 || res->columns() > 0) {
            LOG4CXX_INFO(logger, "[Task: " << name_ << "] Step " << stepIndex 
                << " executed successfully (" << res->rows() << " rows affected)");
        } else {
            LOG4CXX_INFO(logger, "[Task: " << name_ << "] Step " << stepIndex 
                << " executed successfully (Command/DDL OK)");
        }
            }
            catch (const std::exception &e)
            {
                return std::unexpected("Exception caught at step " + std::to_string(stepIndex) + ": " + e.what());
            }
        }

        return {}; // Tout s'est bien passé
    }

} // namespace cppsqlx
