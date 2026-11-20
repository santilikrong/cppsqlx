#ifndef CPPSQLX_LOGICAL_CONNECTION_HPP
#define CPPSQLX_LOGICAL_CONNECTION_HPP

#include <string>
#include <map>
#include <expected>
#include <cppsqlx/json_concepts.hpp>
#include <cppsqlx/physicalconnection.hpp>

namespace cppsqlx
{

class LogicalConnection
{
private:
    std::string name_; // Ex: "Finance", "HR", "Core"
    std::map<std::string, PhysicalConnection> environments_; // Clé = "DEV", "TEST", "PROD"

public:
    LogicalConnection() = default;
    explicit LogicalConnection(std::string name) : name_(std::move(name)) {}
    ~LogicalConnection() = default;

    // ---------- Accessors & Modifiers ----------
    const std::string& name() const noexcept { return name_; }
    void setName(std::string name) { name_ = std::move(name); }

    const std::map<std::string, PhysicalConnection>& environments() const noexcept { return environments_; }
    
    /**
     * @brief Associe un couloir applicatif (DEV, TEST...) à sa configuration physique.
     */
    void registerEnvironment(std::string environmentContext, PhysicalConnection physicalConfig) {
        environments_.insert_or_assign(std::move(environmentContext), std::move(physicalConfig));
    }

    /**
     * @brief Récupère la configuration physique pour un couloir donné.
     */
    std::expected<PhysicalConnection, std::string> getConfiguration(const std::string& environmentContext) const {
        auto it = environments_.find(environmentContext);
        if (it == environments_.end()) {
            return std::unexpected("Le contexte d'environnement '" + environmentContext + "' n'existe pas pour " + name_);
        }
        return it->second;
    }

    // ---------- JSON Concepts Contract ----------
    [[nodiscard]] json toJSON() const;
    static std::expected<LogicalConnection, std::string> fromJSON(const json& j);
};

} // namespace cppsqlx

#endif // CPPSQLX_LOGICAL_CONNECTION_HPP
