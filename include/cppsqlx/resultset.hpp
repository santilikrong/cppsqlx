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

#ifndef CPPSQLX_RESULTSET_HPP
#define CPPSQLX_RESULTSET_HPP

#include <libpq-fe.h>
#include <string>
#include <expected>
#include <memory>
#include <charconv>

namespace cppsqlx
{

    class Resultset
    {
    public:
        explicit Resultset(PGresult *res);
        ~Resultset() = default;

        // Move-only semantics
        Resultset(Resultset &&) noexcept = default;
        Resultset &operator=(Resultset &&) noexcept = default;

        // Disable copy
        Resultset(const Resultset &) = delete;
        Resultset &operator=(const Resultset &) = delete;

        int rows() const;
        int columns() const;

        void print() const;

        std::expected<std::string, std::string> fetch(int row, int col) const;

        std::expected<bool, std::string> next();
        std::expected<bool, std::string> previous();
        std::expected<bool, std::string> first();
        std::expected<bool, std::string> last();
        int currentRow() const;

        // Row proxy class

        class Row
        {
        public:
            Row(const Resultset *rs, int index) : rs_(rs), index_(index) {}

            std::expected<std::string, std::string> operator[](int col) const;
            std::expected<int, std::string> getInt(int col) const;
            std::expected<double, std::string> getDouble(int col) const;
            bool isNull(int col) const;

        private:
            const Resultset *rs_;
            int index_;
        };

        // Iterator class
        class RowIterator
        {
        public:
            RowIterator(const Resultset *rs, int pos) : rs_(rs), pos_(pos) {}
            bool operator!=(const RowIterator &other) const { return pos_ != other.pos_; }
            RowIterator &operator++()
            {
                ++pos_;
                return *this;
            }
            Row operator*() const { return Row(rs_, pos_); }

        private:
            const Resultset *rs_;
            int pos_;
        };

        RowIterator begin() const { return RowIterator(this, 0); }
        RowIterator end() const { return RowIterator(this, rows()); }

        friend std::ostream& operator<<(std::ostream& os, const Resultset& rs);
    private:
        std::unique_ptr<PGresult, void (*)(PGresult *)> result_;
        int currentRow_;
    };

    std::ostream &operator<<(std::ostream &os, const Resultset &rs);
} // namespace cppsqlx

#endif
