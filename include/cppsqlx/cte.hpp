
#ifndef CPPSQLX_CTE_HPP
#define CPPSQLX_CTE_HPP

#include <string>
#include <memory>
#include <cppsqlx/json_concepts.hpp> // 💡 Ajout de l'en-tête

namespace cppsqlx
{

    class SelectQuery; // Forward declaration

    // 💡 Héritage public de JSONRepresentable
    class CTE
    {
    public:
        CTE(std::string name, std::shared_ptr<SelectQuery> query)
            : name_(std::move(name)), query_(std::move(query)) {}

        CTE(const CTE &) = default;
        static std::expected<cppsqlx::CTE, std::string> fromJSON(const json &j);

        CTE &operator=(const CTE &) = default;
        CTE(CTE &&) noexcept = default;
        CTE &operator=(CTE &&) noexcept = default;

        ~CTE();

        std::string toSQL() const;

        // 💡 Déclaration de toJSON
        json toJSON() const;

        const std::string &name() const { return name_; }
        const SelectQuery &query() const { return *query_; }

    private:
        std::string name_;
        std::shared_ptr<SelectQuery> query_;
    };

} // namespace cppsqlx

#endif // CPPSQLX_CTE_HPP
