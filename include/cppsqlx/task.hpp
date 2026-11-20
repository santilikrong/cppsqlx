#ifndef CPPSQLX_TASK_HPP
#define CPPSQLX_TASK_HPP

#include <string>
#include <map>
#include <expected>
#include <cppsqlx/unit_select.hpp>
#include <cppsqlx/json_concepts.hpp>
#include <cppsqlx/connection.hpp>

namespace cppsqlx
{

    class Task
    {
    private:
        std::string name_;
        std::map<int, UnitSelect> steps_;

    public:
        Task() = default;
        explicit Task(std::string name) : name_(std::move(name)) {}
        ~Task() = default;

        // ---------- Accessors & Mutators ----------
        const std::string &name() const noexcept { return name_; }
        void setName(std::string name) { name_ = std::move(name); }

        const std::map<int, UnitSelect> &steps() const noexcept { return steps_; }
        void addStep(int stepIndex, UnitSelect unitSelect) { steps_.insert_or_assign(stepIndex, std::move(unitSelect)); }

        // Dans la section public de la classe Task :
        std::expected<void, std::string> execute(std::shared_ptr<Connection> connection);

        // ---------- JSON Serialization & Deserialization ----------
        [[nodiscard]] json toJSON() const;
        static std::expected<Task, std::string> fromJSON(const json &j);
    };

} // namespace cppsqlx

#endif // CPPSQLX_TASK_HPP
