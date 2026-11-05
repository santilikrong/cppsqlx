#ifndef CPPSQLX_LOGICAL_CONNECTION_HPP
#define CPPSQLX_LOGICAL_CONNECTION_HPP

#include <string>
#include <vector>
#include <tuple>
#include <expected>
#include <cppsqlx/json_concepts.hpp>
#include <cppsqlx/connectioncontext.hpp>
#include <cppsqlx/physicalconnection.hpp>

namespace cppsqlx
{

class LogicalConnection
{
public:
    std::string name_;
    std::vector<std::tuple<ConnectionContext, PhysicalConnection>> connections_;

    LogicalConnection() = default;
    ~LogicalConnection() = default;

    [[nodiscard]] json toJSON() const;
    static std::expected<LogicalConnection, std::string> fromJSON(const json& j);
};

} // namespace cppsqlx

#endif // CPPSQLX_LOGICAL_CONNECTION_HPP
