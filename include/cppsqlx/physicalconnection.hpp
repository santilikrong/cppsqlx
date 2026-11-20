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
    std::string schema_ = "public";
    std::string sslmode_ = "prefer"; // Valeurs : "disable", "allow", "prefer", "require", "verify-ca", "verify-full"

    PhysicalConnection() = default;
    ~PhysicalConnection() = default;

    // Génère automatiquement la chaîne de connexion libpq complète
    [[nodiscard]] std::string toConnectionString() const
    {
        std::string connStr = "dbname=" + dbname_ +
                              " user=" + dbuser_ +
                              " host=" + dbhost_ +
                              " port=" + std::to_string(dbport_) +
                              " sslmode=" + (sslmode_.empty() ? "prefer" : sslmode_);
        if (!dbpassword_.empty())
        {
            connStr += " password=" + dbpassword_;
        }
        return connStr;
    }

    void refresh() {}

    [[nodiscard]] json toJSON() const;
    static std::expected<PhysicalConnection, std::string> fromJSON(const json& j);
};


} // namespace cppsqlx

#endif // CPPSQLX_PHYSICAL_CONNECTION_HPP
