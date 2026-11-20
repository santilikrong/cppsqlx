
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


json Dataset::toJSON() const
{
    // 1. Initialisation de l'objet principal avec les métadonnées du Dataset
    json j = json::object({
        {"name", name_},
        {"schema", schema_},
        {"catalog", catalog_}
    });

    // 2. Création d'un tableau JSON pour accueillir la liste des colonnes
    json columns_array = json::array();

    // 3. Itération sur votre structure associative columns_
    for (const auto& c : columns_)
    {
        // c.first est la clé, c.second est l'instance de votre classe Column
        // On appelle la méthode toJSON() de l'objet Column récursivement
        columns_array.push_back(c.second.toJSON());
    }

    // 4. Imbrication du tableau de colonnes dans l'objet principal
    j["columns"] = columns_array;

    return json{ {"dataset", j} };
}

std::expected<Dataset, std::string> Dataset::fromJSON(const json& j)
{
    const json& data = j.contains("dataset") ? j["dataset"] : j;

    try {
        std::string name = data.at("name").get<std::string>();
        Dataset ds(name);
        
        ds.setSchema(data.value("schema", ""));
        ds.setCatalog(data.value("catalog", ""));
        
        if (data.contains("alias") && !data["alias"].get<std::string>().empty()) {
            ds.as(data["alias"].get<std::string>());
        }

        // Reconstruction récursive de la liste des colonnes
        if (data.contains("columns")) {
            for (const auto& colJson : data["columns"]) {
                auto colRes = Column::fromJSON(colJson);
                if (!colRes) {
                    return std::unexpected(colRes.error()); // Propage l'erreur de validation
                }
                // Réinsertion dans la map à l'index de sa position d'origine
                ds.columns_.insert_or_assign(colRes->position(), std::move(*colRes));
            }
        }

        return ds;
    }
    catch (const std::exception& e) {
        return std::unexpected(std::string("Dataset::fromJSON failed: ") + e.what());
    }
}

const std::map<int, Column>& Dataset::columns() const
{
    return columns_;
}

} // namespace cppsqlx
