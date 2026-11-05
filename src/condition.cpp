#include <cppsqlx/condition.hpp>

namespace cppsqlx
{

json Condition::toJSON() const
{
    json j;
    j["type"] = static_cast<int>(type_);
    j["expected_job"] = expectedJobName_;
    j["expected_status"] = static_cast<int>(expectedStatus_);

    json jsonSubs = json::array();
    for (const auto& cond : subConditions_) {
        jsonSubs.push_back(cond.toJSON());
    }
    j["sub_conditions"] = jsonSubs;

    return json{ {"condition", j} };
}

std::expected<Condition, std::string> Condition::fromJSON(const json& j)
{
    const json& data = j.contains("condition") ? j["condition"] : j;

    try {
        Condition cond;
        cond.type_ = static_cast<ConditionType>(data.at("type").get<int>());
        cond.expectedJobName_ = data.value("expected_job", "");
        cond.expectedStatus_ = static_cast<JobStatus>(data.value("expected_status", static_cast<int>(JobStatus::Success)));

        if (data.contains("sub_conditions") && data["sub_conditions"].is_array()) {
            for (const auto& subJson : data["sub_conditions"]) {
                auto subRes = Condition::fromJSON(subJson);
                if (!subRes) return std::unexpected("Condition::fromJSON sub -> " + subRes.error());
                cond.subConditions_.push_back(std::move(*subRes));
            }
        }

        return cond;
    }
    catch (const std::exception& e) {
        return std::unexpected(std::string("Condition::fromJSON failed: ") + e.what());
    }
}

} // namespace cppsqlx
