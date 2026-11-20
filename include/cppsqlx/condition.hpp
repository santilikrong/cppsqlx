#ifndef CPPSQLX_CONDITION_HPP
#define CPPSQLX_CONDITION_HPP

#include <string>
#include <map>
#include <vector>
#include <expected>
#include <cppsqlx/json_concepts.hpp>
#include <cppsqlx/job_result.hpp>

namespace cppsqlx
{

enum class ConditionType {
    SingleJob,
    AndGroup,
    OrGroup
};

class Condition
{
private:
    ConditionType type_ = ConditionType::SingleJob;
    std::string expectedJobName_;
    JobStatus expectedStatus_ = JobStatus::Success;
    std::vector<Condition> subConditions_;

public:
    Condition() = default;
    
    // Constructeur de base (Single Job)
    Condition(std::string jobName, JobStatus status)
        : type_(ConditionType::SingleJob), expectedJobName_(std::move(jobName)), expectedStatus_(status) {}

    // Constructeurs de groupes logiques
    static Condition makeAnd(std::vector<Condition> conds) {
        Condition c; c.type_ = ConditionType::AndGroup; c.subConditions_ = std::move(conds); return c;
    }

    static Condition makeOr(std::vector<Condition> conds) {
        Condition c; c.type_ = ConditionType::OrGroup; c.subConditions_ = std::move(conds); return c;
    }

    bool isSatisfied(const std::map<std::string, JobResult>& currentResults) const
    {
        switch (type_) 
        {
            case ConditionType::SingleJob: {
                auto it = currentResults.find(expectedJobName_);
                if (it == currentResults.end()) return false;
                return it->second.status == expectedStatus_;
            }
            case ConditionType::AndGroup: {
                for (const auto& cond : subConditions_) {
                    if (!cond.isSatisfied(currentResults)) return false;
                }
                return true;
            }
            case ConditionType::OrGroup: {
                for (const auto& cond : subConditions_) {
                    if (cond.isSatisfied(currentResults)) return true;
                }
                return false;
            }
        }
        return false;
    }

    [[nodiscard]] json toJSON() const;
    static std::expected<Condition, std::string> fromJSON(const json& j);
};

} // namespace cppsqlx

#endif // CPPSQLX_CONDITION_HPP
