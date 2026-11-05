#ifndef CPPSQLX_ETL_TASK_HPP
#define CPPSQLX_ETL_TASK_HPP

#include <string>
#include <memory>
#include <map>
#include <expected>
#include <cppsqlx/json_concepts.hpp>
#include <cppsqlx/selectquery.hpp>
#include <cppsqlx/dataset.hpp>

namespace cppsqlx
{

class ETLTask 
{
private:
    std::string name_;
    std::shared_ptr<SelectQuery> sqlselect_;
    std::shared_ptr<Dataset> targetds_;
    std::map<std::shared_ptr<Dataset>, std::string> connections_;

public:
    ETLTask() = default;
    ~ETLTask() = default;

    // ---------- Accessors ----------
    const std::string& name() const noexcept { return name_; }
    std::shared_ptr<SelectQuery> getSelect() const noexcept { return sqlselect_; }
    std::shared_ptr<Dataset> targetDS() const noexcept { return targetds_; }
    const std::map<std::shared_ptr<Dataset>, std::string>& connections() const noexcept { return connections_; }

    // ---------- Mutators ----------
    void setName(std::string name) { name_ = std::move(name); }
    void setSelect(std::shared_ptr<SelectQuery> sqlselect) { sqlselect_ = std::move(sqlselect); }
    void setTargetDS(std::shared_ptr<Dataset> ds) { targetds_ = std::move(ds); }
    void setConnections(std::map<std::shared_ptr<Dataset>, std::string> cnx) { connections_ = std::move(cnx); }

    // ---------- JSON Serialization & Deserialization ----------
    [[nodiscard]] json toJSON() const;
    static std::expected<ETLTask, std::string> fromJSON(const json& j);
};

} // namespace cppsqlx

#endif // CPPSQLX_ETL_TASK_HPP
