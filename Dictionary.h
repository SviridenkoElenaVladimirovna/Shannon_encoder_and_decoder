#include <string>
#include <unordered_map>
#include <optional>
#include <cstdint>
#include <vector>

class Dictionary {
public:
    static constexpr size_t MAX_CODE_LENGTH = 32;

    void analyze_frequencies(const std::string& data);
    void save_to_file(const std::string& filename) const;
    void load_from_file(const std::string& filename);
    const std::string& get_code(char symbol) const;
    std::optional<char> get_symbol(const std::string& code) const;
    void print_statistics() const;

private:
    struct SymbolInfo {
        uint32_t frequency = 0;
        std::string code;
    };

    void build_shannon_fano_codes();
    void build_codes_recursive(const std::vector<std::pair<char, uint32_t>>& symbols,
                               size_t start, size_t end, const std::string& code);

    std::unordered_map<char, SymbolInfo> symbol_map;
    std::unordered_map<std::string, char> code_map;
    size_t total_symbols = 0;
};
