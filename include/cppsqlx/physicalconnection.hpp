#ifndef CPPSQLX_PHYSICAL_CONNECTION_HPP
#define CPPSQLX_PHYSICAL_CONNECTION_HPP

#include <string>
#include <expected>
#include <cppsqlx/json_concepts.hpp>

namespace cppsqlx
{

class PhysicalConnection
{
public:
    std::string name_;
    std::string dbname_;
    std::string dbhost_;
    int dbport_ = 5432;
    std::string dbuser_;
    std::string dbpassword_;
    std::string schema_;

    PhysicalConnection() = default;
    ~PhysicalConnection() = default;

    void refresh() {} // Maintenu pour la compatibilité avec vos appels existants

    [[nodiscard]] json toJSON() const;
    static std::expected<PhysicalConnection, std::string> fromJSON(const json& j);
};

} // namespace cppsqlx

#endif // CPPSQLX_PHYSICAL_CONNECTION_HPP
