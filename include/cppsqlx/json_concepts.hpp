#ifndef CPPSQLX_JSON_CONCEPTS_HPP
#define CPPSQLX_JSON_CONCEPTS_HPP

#include <nlohmann/json.hpp>
#include <concepts>
#include <string>
#include <expected>

using json = nlohmann::json;

namespace cppsqlx 
{

// ============================================================================
// CONCEPTS (Contrats à la compilation)
// ============================================================================

/**
 * @brief Exige que l'objet possède une méthode toJSON() retournant du JSON.
 */
template <typename T>
concept JSONRepresentable = requires(const T& obj) {
    { obj.toJSON() } -> std::same_as<json>;
};

/**
 * @brief Exige que la classe possède une méthode statique fromJSON() 
 * retournant un std::expected.
 */
template <typename T>
concept JSONReversible = requires(const json& j) {
    { T::fromJSON(j) } -> std::same_as<std::expected<T, std::string>>;
};

/**
 * @brief Combine la sérialisation et la désérialisation.
 */
template <typename T>
concept JSONSerializable = JSONRepresentable<T> && JSONReversible<T>;

// ============================================================================
// FONCTIONS TEMPLATES GÉNÉRIQUES (Remplacent les méthodes de classe mère)
// ============================================================================
namespace io 
{
    /**
     * @brief Rentre une représentation sous forme de chaîne de caractères (remplace .dump())
     */
    template <JSONRepresentable T>
    std::string dump(const T& obj, int indent = -1) 
    {
        return obj.toJSON().dump(indent);
    }

    /**
     * @brief Sérialise un objet dans une archive (remplace .save())
     */
    template <JSONRepresentable T, class Archive>
    void save(Archive& ar, const T& obj) 
    {
        ar(dump(obj));
    } 

    /**
     * @brief Désérialise et met à jour un objet existant (remplace .load())
     */
    template <JSONSerializable T, class Archive>
    std::expected<void, std::string> load(Archive& ar, T& obj) 
    {
        std::string serialized;
        ar(serialized);
        
        try {
            auto result = T::fromJSON(json::parse(serialized));
            if (!result) {
                return std::unexpected(result.error());
            }
            obj = std::move(*result);
            return {};
        }
        catch (const std::exception& e) {
            return std::unexpected(std::string("io::load failed: ") + e.what());
        }
    }
} // namespace io

} // namespace cppsqlx

#endif // CPPSQLX_JSON_CONCEPTS_HPP
