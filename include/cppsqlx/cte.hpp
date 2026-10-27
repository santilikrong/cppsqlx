
#ifndef CPPSQLX_CTE_HPP
#define CPPSQLX_CTE_HPP

#include <string>
#include <memory>

namespace cppsqlx {

class SelectQuery; // Forward declaration

class CTE {
public:
    CTE(std::string name, std::shared_ptr<SelectQuery> query)
        : name_(std::move(name)), query_(std::move(query)) {}

    CTE(const CTE&) = default;
    CTE& operator=(const CTE&) = default;
    CTE(CTE&&) noexcept = default;
    CTE& operator=(CTE&&) noexcept = default;

    ~CTE();

    std::string toSQL() const; // Implemented in .cpp

    const std::string& name() const { return name_; }
    const SelectQuery& query() const { return *query_; }

private:
    std::string name_;
    std::shared_ptr<SelectQuery> query_;
};

} // namespace cppsqlx

#endif // CPPSQLX_CTE_HPP
