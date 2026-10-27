
#include "cppsqlx/dataset.hpp"
#include "cppsqlx/join.hpp" // Assuming you will define this
#include <stdexcept>
#include <sstream>

namespace cppsqlx
{

Dataset::Dataset() = default;
Dataset::Dataset(std::string name) : name_(std::move(name)) {}
Dataset::~Dataset() = default;

std::string Dataset::name() const { return name_; }
std::string Dataset::schema() const { return schema_; }
std::string Dataset::catalog() const { return catalog_; }
std::string Dataset::alias() const { return alias_; }

void Dataset::setSchema(std::string schema) { schema_ = std::move(schema); }
void Dataset::setCatalog(std::string catalog) { catalog_ = std::move(catalog); }

Dataset& Dataset::as(std::string alias)
{
    alias_ = std::move(alias);
    return *this;
}

std::optional<Column> Dataset::at(int index) const
{
    auto it = columns_.find(index);
    if (it != columns_.end())
        return it->second;
    return std::nullopt;
}

int Dataset::rowSize() const { return static_cast<int>(columns_.size()); }

std::string Dataset::joinidentifier() const
{
    std::string id = schema_.empty() ? name_ : schema_ + "." + name_;
    if (!alias_.empty())
        id += " AS " + alias_;
    return id;
}

Join Dataset::joinTo(const Dataset& rightds) const
{
   Join join(*this);
   join.to(rightds);
   return join;
}



std::string Dataset::toString() const
{
    std::ostringstream oss;
    
    // En-tête du Dataset avec séparateurs pour une meilleure lisibilité
    oss << "========================================\n"
        << " DATASET : " << schema() << "." << name() << "\n"
        << "========================================\n";
    
    // Parcours ordonné des colonnes de la map via structured binding (C++17)
    for (const auto& [pos, col] : columns_)
    {
        oss << "  -> [" << pos << "] " << col.name() << " " << col.type();
        
        // 💡 Affichage conditionnel de la taille uniquement si elle est différente de 0
        if (col.size() != 0) {
            oss << " (" << col.size() << ")";
        }
        
        oss << "\n";
    }
    
    oss << "========================================\n";
    
    return oss.str();
}




} // namespace cppsqlx
