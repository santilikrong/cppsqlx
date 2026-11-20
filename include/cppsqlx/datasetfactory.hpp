
/*
 * MIT License
 * Copyright (c) 2025 cppsqlx
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef CPPSQLX_DATASET_FACTORY_HPP
#define CPPSQLX_DATASET_FACTORY_HPP

#include <cppsqlx/dataset.hpp>
#include <cppsqlx/connection.hpp>
#include <string>
#include <expected>

namespace cppsqlx {

class DatasetFactory {
public:
    /**
     * @brief Crée un Dataset complet en introspectant une table de la base de données.
     * @param conn Référence vers une connexion active.
     * @param tableName Nom de la table cible (ex: "t1").
     * @param schemaName Nom du schéma (par défaut "public").
     * @return Un Dataset configuré ou une chaîne de caractères décrivant l'erreur.
     */
    static std::expected<Dataset, std::string> createFromDatabase(
        Connection& conn, 
        const std::string& tableName, 
        const std::string& schemaName = "public"
    );
};

} // namespace cppsqlx

#endif // CPPSQLX_DATASET_FACTORY_HPP
