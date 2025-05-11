#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

class Dictionary {
private:
    struct SymbolInfo {
        uint32_t frequency;
        std::string code;
    };
    size_t total_symbols = 0;
    std::unordered_map<char, SymbolInfo> symbol_map;
    std::unordered_map<std::string, char> code_map;

    void build_shannon_codes(const std::vector<std::pair<char, uint32_t>>& symbols,
                             int start, int end,
                             const std::string& code);

public:
    void analyze_frequencies(const std::string& data);
    void save_to_file(const std::string& filename) const;
    void load_from_file(const std::string& filename);
    void print_statistics() const;
    const std::string& get_code(char symbol) const;
    char get_symbol(const std::string& code) const;
};

#endif