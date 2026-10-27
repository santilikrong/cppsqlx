#include "cppsqlx/datasetfactory.hpp"
#include <sstream>

namespace cppsqlx {

std::expected<Dataset, std::string> DatasetFactory::createFromDatabase(
    Connection& conn, 
    const std::string& tableName, 
    const std::string& schemaName) 
{
    if (!conn.isConnected()) {
        return std::unexpected("Erreur : La connexion à la base de données n'est pas active.");
    }

    auto stmtPtr = conn.createStatement();
    if (!stmtPtr) {
        return std::unexpected("Erreur : Impossible de créer un Statement.");
    }

    // Récupération complète des métadonnées depuis les catalogues système
    std::ostringstream sql;
    sql << "SELECT column_name, ordinal_position, data_type, is_nullable, column_default, "
        << "character_maximum_length, numeric_precision, numeric_scale "
        << "FROM information_schema.columns "
        << "WHERE table_name = '" << tableName << "' "
        << "AND table_schema = '" << schemaName << "' "
        << "ORDER BY ordinal_position ASC;";

    auto result = stmtPtr->execute(sql.str());
    if (!result.has_value()) {
        return std::unexpected("Échec de la récupération des métadonnées : " + result.error());
    }

    auto& resultset = result.value();
    
    if (resultset.rows() == 0) { 
        return std::unexpected("Erreur : La table '" + schemaName + "." + tableName + "' n'existe pas ou est vide.");
    }

    Dataset ds(tableName);
    ds.setSchema(schemaName);

    // Parcours sécurisé des lignes du Resultset
    for (Resultset::Row row : resultset) {
        // 1. Extraction stricte via les index numériques du SELECT (0 à 7)
        auto nameExpected      = row[0];
        auto positionExpected  = row.getInt(1);
        auto typeExpected      = row[2];
        auto nullableExpected  = row[3];
        auto defaultExpected   = row[4];
        auto sizeExpected      = row.getInt(5);
        auto precisionExpected = row.getInt(6);
        auto scaleExpected     = row.getInt(7);

        // 2. Validation de la présence des informations obligatoires lues
        if (!nameExpected || !positionExpected || !typeExpected || !nullableExpected) {
            return std::unexpected("Erreur de format dans les métadonnées système lues.");
        }

        std::string columnName = nameExpected.value();
        int position           = positionExpected.value();
        std::string dataType   = typeExpected.value();
        bool isNullable        = (nullableExpected.value() == "YES");
        
        // 3. Fallback sain pour les attributs optionnels d'information_schema
        std::string defaultVal = defaultExpected.value_or("");
        int size               = sizeExpected.value_or(0);
        int precision          = precisionExpected.value_or(0);
        int scale              = scaleExpected.value_or(0);
        bool isUnique          = false; 

        // 4. Utilisation recommandée de votre Factory Method de création de colonne
        auto columnCreation = Column::create(
            columnName, dataType, isNullable, isUnique, defaultVal, size, precision, scale, position
        );

        // Si le nom ou les propriétés de la colonne violent les règles de validation de Column::create
        if (!columnCreation.has_value()) {
            return std::unexpected("Impossible de valider la colonne '" + columnName + "' : " + columnCreation.error());
        }

        // 5. Déplacement et insertion de l'objet Column valide dans votre map
        ds.columns_.try_emplace(position, std::move(columnCreation.value()));
    }

    return ds;
}

} // namespace cppsqlx
