#ifndef CPPSQLX_CONNECTION_CONTEXT_HPP
#define CPPSQLX_CONNECTION_CONTEXT_HPP

#include <string>
#include <expected>
#include <cppsqlx/json_concepts.hpp>

namespace cppsqlx
{

class ConnectionContext
{
public:
    std::string name_;
    std::string description_;

    ConnectionContext() = default;
    ~ConnectionContext() = default;

    void refresh() {}

    [[nodiscard]] json toJSON() const;
    static std::expected<ConnectionContext, std::string> fromJSON(const json& j);
};

} // namespace cppsqlx

#endif // CPPSQLX_CONNECTION_CONTEXT_HPP
