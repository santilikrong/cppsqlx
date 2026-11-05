#ifndef CPPSQLX_JOB_HPP
#define CPPSQLX_JOB_HPP

#include <string>
#include <list>
#include <map>
#include <memory>
#include <expected>
#include <cppsqlx/json_concepts.hpp>
#include <cppsqlx/condition.hpp>
#include <cppsqlx/task.hpp>
#include <cppsqlx/job_result.hpp>

namespace cppsqlx
{

class Connection;
class Job 
{
private:
    std::string name_;
    int x_ = 0;
    int y_ = 0; 
    std::list<Condition> requirements_;
    std::map<int, std::shared_ptr<Task>> tasks_;

public:
    Job() = default;
    explicit Job(std::string name) : name_(std::move(name)) {}
    ~Job() = default;

    // ---------- Accessors ----------
    const std::string& name() const noexcept { return name_; }
    int x() const noexcept { return x_; }
    int y() const noexcept { return y_; }
    const std::list<Condition>& requirements() const noexcept { return requirements_; }
    const std::map<int, std::shared_ptr<Task>>& tasks() const noexcept { return tasks_; }

    // ---------- Mutators & Modifiers ----------
    void setName(std::string name) { name_ = std::move(name); }
    void setX(int x) noexcept { x_ = x; }
    void setY(int y) noexcept { y_ = y; }
    void setRequirements(std::list<Condition> requirements) { requirements_ = std::move(requirements); }
    void setTasks(std::map<int, std::shared_ptr<Task>> tasks) { tasks_ = std::move(tasks); }
    
    void addTask(int pos, std::shared_ptr<Task> task) { tasks_.insert_or_assign(pos, std::move(task)); }
    void addCondition(Condition condition) { requirements_.push_back(std::move(condition)); }

    // ---------- Execution Logic (Déclarations uniquement) ----------
    [[nodiscard]] bool canExecute(const std::map<std::string, JobResult>& workflowContext) const;
    JobResult execute(std::shared_ptr<Connection> connection);

    // ---------- JSON Concepts Contract ----------
    [[nodiscard]] json toJSON() const;
    static std::expected<Job, std::string> fromJSON(const json& j);
};

} // namespace cppsqlx

#endif // CPPSQLX_JOB_HPP
