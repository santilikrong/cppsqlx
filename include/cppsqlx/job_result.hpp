#ifndef CPPSQLX_JOB_RESULT_HPP
#define CPPSQLX_JOB_RESULT_HPP

#include <string>
#include <nlohmann/json.hpp>

namespace cppsqlx
{

enum class JobStatus {
    NotStarted,
    Running,
    Success,
    Failed
};

struct JobResult
{
    std::string jobName;
    JobStatus status = JobStatus::NotStarted;
    std::string message;

    nlohmann::json toJSON() const {
        return nlohmann::json{
            {"job_name", jobName}, 
            {"status", static_cast<int>(status)}, 
            {"message", message}
        };
    }
};

} // namespace cppsqlx

#endif // CPPSQLX_JOB_RESULT_HPP
