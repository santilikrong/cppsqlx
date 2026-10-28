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

#ifndef CPPSQLX_SELECTABLE_H
#define CPPSQLX_SELECTABLE_H

#include <string>
#include <concepts>
#include <type_traits> // Ajout indispensable pour std::decay_t et std::is_convertible_v

namespace cppsqlx {

/**
 * @brief Concept C++20 définissant ce qui peut être passé à la clause SELECT.
 */
template <typename T>
concept Selectable = requires(T a) {
    { a.toSQL() } -> std::convertible_to<std::string>;
} || std::is_convertible_v<std::decay_t<T>, std::string>;

} // namespace cppsqlx

#endif // CPPSQLX_SELECTABLE_H

