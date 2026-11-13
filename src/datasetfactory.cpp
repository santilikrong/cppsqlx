#include "cppsqlx/datasetfactory.hpp"
#include "cppsqlx/connection.hpp"
#include "cppsqlx/statement.hpp" // 🟢 AJOUT CRUCIAL : Rend le type Statement complet
#include "cppsqlx/resultset.hpp" // 🟢 AJOUT CRUCIAL : Rend le type Resultset complet
#include <sstream>

namespace cppsqlx
{

    std::expected<Dataset, std::string> DatasetFactory::createFromDatabase(
        Connection &conn,
        const std::string &tableName,
        const std::string &schemaName)
    {
        if (!conn.isConnected())
        {
            return std::unexpected("Erreur : La connexion à la base de données n'est pas active.");
        }

        auto stmtPtr = conn.createStatement();
        if (!stmtPtr)
        {
            return std::unexpected("Erreur : Impossible de créer un Statement.");
        }

        // Récupération complète des métadonnées depuis les catalogues système
        std::ostringstream sql;
        sql << "SELECT current_database(), column_name, ordinal_position, data_type, is_nullable, column_default, "
            << "character_maximum_length, numeric_precision, numeric_scale "
            << "FROM information_schema.columns "
            << "WHERE table_name = '" << tableName << "' "
            << "AND table_schema = '" << schemaName << "' "
            << "ORDER BY ordinal_position ASC;";

        // 🟢 Fonctionne parfaitement maintenant que l'en-tête de Statement est inclus
        auto result = stmtPtr->execute(sql.str());
        if (!result.has_value())
        {
            return std::unexpected("Échec de la récupération des métadonnées : " + result.error());
        }

        auto &resultset = result.value();

        if (resultset.rows() == 0)
        {
            return std::unexpected("Erreur : La table '" + schemaName + "." + tableName + "' n'existe pas ou est vide.");
        }

        Dataset ds(tableName);
        ds.setSchema(schemaName);

        auto firstRow = resultset.begin();
        if (firstRow != resultset.end())
        {
            auto catalogExpected = (*firstRow)[0];
            if (catalogExpected)
            {
                ds.setCatalog(catalogExpected.value()); // Assurez-vous d'avoir une méthode setCatalog() dans Dataset
            }
        }

        for (Resultset::Row row : resultset)
        {
            auto nameExpected = row["column_name"];
            auto positionExpected = row.getInt("ordinal_position");
            auto typeExpected = row["data_type"];
            auto nullableExpected = row["is_nullable"];
            auto defaultExpected = row["column_default"];
            auto sizeExpected = row.getInt("character_maximum_length");
            auto precisionExpected = row.getInt("numeric_precision");
            auto scaleExpected = row.getInt("numeric_scale");


            if (!nameExpected || !positionExpected || !typeExpected || !nullableExpected)
            {
                return std::unexpected("Erreur de format dans les métadonnées système lues.");
            }

            std::string columnName = nameExpected.value();
            int position = positionExpected.value();
            std::string dataType = typeExpected.value();
            bool isNullable = (nullableExpected.value() == "YES");

            // 3. Fallback sain pour les attributs optionnels d'information_schema
            std::string defaultVal = defaultExpected.value_or("");
            int size = sizeExpected.value_or(0);
            int precision = precisionExpected.value_or(0);
            int scale = scaleExpected.value_or(0);
            bool isUnique = false;

            // 🟢 FORMATAGE DU TYPE ENRICHI AVEC SA TAILLE :
            // Si la colonne possède une longueur ou une précision spécifique, on l'annote dynamiquement
            if (size > 0)
            {
                dataType += "(" + std::to_string(size) + ")";
            }
            else if (precision > 0 && scale >= 0 && dataType != "integer")
            {
                dataType += "(" + std::to_string(precision) + "," + std::to_string(scale) + ")";
            }

            // 4. Utilisation recommandée de votre Factory Method de création de colonne avec le type formaté
            auto columnCreation = Column::create(
                columnName, dataType, isNullable, isUnique, defaultVal, size, precision, scale, position);

            // Si le nom ou les propriétés de la colonne violent les règles de validation de Column::create
            if (!columnCreation.has_value())
            {
                return std::unexpected("Impossible de valider la colonne '" + columnName + "' : " + columnCreation.error());
            }

            // 5. Déplacement et insertion de l'objet Column valide dans votre map
            ds.columns_.try_emplace(position, std::move(columnCreation.value()));
        }

        return ds;
    }


} // namespace cppsqlx
