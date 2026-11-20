
/// @brief Internal helpers for validation and case-insensitive checks.
namespace detail {

    /// @brief Validate identifier: non-empty and only letters, digits, or underscore.
    inline bool isIdentifierValid(const std::string& id) noexcept 
    {
        auto islet = [] (char c) {  return std::isalnum(c) != 0 || c == '_'; };
        return !id.empty() &&
               std::all_of(id.begin(), id.end(), islet);
    }

    /// @brief Convert string to lowercase.
    inline std::string toLower(std::string s) 
    {
        std::transform(s.begin(), s.end(), s.begin(), [](char c){ return static_cast<char>(std::tolower(c)); } );
        return s;
    }

    /// @brief Case-insensitive substring check.
    inline bool containsCaseInsensitive(const std::string& haystack, const std::string& needle) 
    {
        auto h = toLower(haystack);
        auto n = toLower(needle);
        return h.find(n) != std::string::npos;
    }
}
