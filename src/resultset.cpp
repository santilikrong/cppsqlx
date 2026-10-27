/*
MIT License

Copyright (c) 2025 Y-HRAH KRONG Santili

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "cppsqlx/resultset.hpp"
#include <stdexcept>
#include <iostream>  // Ajouté pour std::cout
#include <vector>    // Ajouté pour stocker les largeurs de colonnes
#include <iomanip>   // Ajouté pour std::setw et std::left
#include <algorithm> // Ajouté pour std::max

namespace cppsqlx {

Resultset::Resultset(PGresult* res)
    : result_(res, PQclear), currentRow_(0) {
    if (!res) {
        throw std::runtime_error("Invalid PGresult pointer");
    }
}

int Resultset::rows() const {
    return PQntuples(result_.get());
}

int Resultset::columns() const {
    return PQnfields(result_.get());
}

std::expected<std::string, std::string> Resultset::fetch(int row, int col) const {
    if (row < 0 || row >= rows() || col < 0 || col >= columns()) {
        return std::unexpected("Row or column index out of range");
    }
    return std::string(PQgetvalue(result_.get(), row, col));
}

std::expected<bool, std::string> Resultset::next() {
    if (currentRow_ + 1 < rows()) {
        ++currentRow_;
        return true;
    }
    return std::unexpected("Already at last row");
}

std::expected<bool, std::string> Resultset::previous() {
    if (currentRow_ - 1 >= 0) {
        --currentRow_;
        return true;
    }
    return std::unexpected("Already at first row");
}

std::expected<bool, std::string> Resultset::first() {
    if (rows() > 0) {
        currentRow_ = 0;
        return true;
    }
    return std::unexpected("Resultset is empty");
}

std::expected<bool, std::string> Resultset::last() {
    if (rows() > 0) {
        currentRow_ = rows() - 1;
        return true;
    }
    return std::unexpected("Resultset is empty");
}

int Resultset::currentRow() const {
    return currentRow_;
}

// Row methods

std::expected<std::string, std::string> Resultset::Row::operator [](int col) const {
    return rs_->fetch(index_, col);
}


void Resultset::print() const {
    int num_rows = rows();
    int num_cols = columns();

    if (num_cols == 0) {
        std::cout << "(Aucune colonne)\n";
        return;
    }

    // 1. Calculer la largeur nécessaire pour chaque colonne
    std::vector<int> col_widths(num_cols, 0);
    std::vector<std::string> col_names(num_cols);

    for (int col = 0; col < num_cols; ++col) {
        // Récupérer le nom du champ depuis libpq
        const char* name = PQfname(result_.get(), col);
        col_names[col] = name ? name : "";
        col_widths[col] = std::max(col_widths[col], static_cast<int>(col_names[col].length()));

        // Parcourir les lignes pour ajuster la largeur selon les valeurs maximales
        for (int row = 0; row < num_rows; ++row) {
            auto val_expected = fetch(row, col);
            if (val_expected) {
                col_widths[col] = std::max(col_widths[col], static_cast<int>(val_expected->length()));
            } else if (PQgetisnull(result_.get(), row, col)) {
                col_widths[col] = std::max(col_widths[col], 4); // Longueur de "NULL"
            }
        }
    }

    // 2. Afficher l'en-tête (Nom des champs)
    for (int col = 0; col < num_cols; ++col) {
        std::cout << " " << std::left << std::setw(col_widths[col]) << col_names[col] << " ";
        if (col < num_cols - 1) {
            std::cout << "|";
        }
    }
    std::cout << "\n";

    // 3. Afficher la ligne de séparation (-------+-------)
    for (int col = 0; col < num_cols; ++col) {
        // +2 pour compenser les espaces ajoutés de chaque côté de la valeur
        std::cout << std::string(col_widths[col] + 2, '-');
        if (col < num_cols - 1) {
            std::cout << "+";
        }
    }
    std::cout << "\n";

    // 4. Afficher les lignes de données
    for (int row = 0; row < num_rows; ++row) {
        for (int col = 0; col < num_cols; ++col) {
            std::cout << " ";
            if (PQgetisnull(result_.get(), row, col)) {
                std::cout << std::left << std::setw(col_widths[col]) << "NULL";
            } else {
                auto val_expected = fetch(row, col);
                std::cout << std::left << std::setw(col_widths[col]) << (val_expected ? *val_expected : "");
            }
            std::cout << " ";
            
            if (col < num_cols - 1) {
                std::cout << "+";
            }
        }
        std::cout << "\n";
    }
    
    std::cout << "(" << num_rows << " " << (num_rows > 1 ? "lignes" : "ligne") << ")\n";
}


std::ostream& operator<<(std::ostream& os, const Resultset& rs) {
    int num_rows = rs.rows();
    int num_cols = rs.columns();

    if (num_cols == 0) {
        os << "(Aucune colonne)\n";
        return os;
    }

    // 1. Calculer la largeur nécessaire pour chaque colonne
    std::vector<int> col_widths(num_cols, 0);
    std::vector<std::string> col_names(num_cols);

    for (int col = 0; col < num_cols; ++col) {
        const char* name = PQfname(rs.result_.get(), col);
        col_names[col] = name ? name : "";
        col_widths[col] = std::max(col_widths[col], static_cast<int>(col_names[col].length()));

        for (int row = 0; row < num_rows; ++row) {
            auto val_expected = rs.fetch(row, col);
            if (val_expected) {
                col_widths[col] = std::max(col_widths[col], static_cast<int>(val_expected->length()));
            } else if (PQgetisnull(rs.result_.get(), row, col)) {
                col_widths[col] = std::max(col_widths[col], 4); // "NULL"
            }
        }
    }

    // 2. Afficher l'en-tête
    for (int col = 0; col < num_cols; ++col) {
        os << " " << std::left << std::setw(col_widths[col]) << col_names[col] << " ";
        if (col < num_cols - 1) {
            os << "|";
        }
    }
    os << "\n";

    // 3. Afficher la ligne de séparation
    for (int col = 0; col < num_cols; ++col) {
        os << std::string(col_widths[col] + 2, '-');
        if (col < num_cols - 1) {
            os << "+";
        }
    }
    os << "\n";

    // 4. Afficher les données
    for (int row = 0; row < num_rows; ++row) {
        for (int col = 0; col < num_cols; ++col) {
            os << " ";
            if (PQgetisnull(rs.result_.get(), row, col)) {
                os << std::left << std::setw(col_widths[col]) << "NULL";
            } else {
                auto val_expected = rs.fetch(row, col);
                os << std::left << std::setw(col_widths[col]) << (val_expected ? *val_expected : "");
            }
            os << " ";
            
            if (col < num_cols - 1) {
                os << "+";
            }
        }
        os << "\n";
    }
    
    os << "(" << num_rows << " " << (num_rows > 1 ? "lignes" : "ligne") << ")\n";
    return os;
}


std::expected<int, std::string> Resultset::Row::getInt(int col) const {
    auto val = (*this)[col];
    if (!val) return std::unexpected(val.error());
    int result;
    auto [ptr, ec] = std::from_chars(val->data(), val->data() + val->size(), result);
    if (ec != std::errc()) return std::unexpected("Invalid integer format");
    return result;
}

std::expected<double, std::string> Resultset::Row::getDouble(int col) const {
    auto val = (*this)[col];
    if (!val) return std::unexpected(val.error());
    try {
        return std::stod(*val);
    } catch (...) {
        return std::unexpected("Invalid double format");
    }
}

bool Resultset::Row::isNull(int col) const {
    return PQgetisnull(rs_->result_.get(), index_, col);
}

} // namespace cppsqlx
