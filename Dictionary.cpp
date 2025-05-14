#include "Dictionary.h"
#include <fstream>
#include <algorithm>
#include <numeric>
#include <iomanip>
#include <bitset>

void Dictionary::analyze_frequencies(const std::string& data) {
    symbol_map.clear();
    code_map.clear();
    total_symbols = data.size();

    for (char c : data) {
        symbol_map[c].frequency++;
    }

    for (int i = 0; i < 256; ++i) {
        char c = static_cast<char>(i);
        if (symbol_map.find(c) == symbol_map.end()) {
            symbol_map[c] = {1, ""};
        }
    }

    build_shannon_fano_codes();
}

void Dictionary::build_shannon_fano_codes() {
    std::vector<std::pair<char, uint32_t>> symbols;
    for (const auto& [symbol, info] : symbol_map) {
        symbols.emplace_back(symbol, info.frequency);
    }

    std::sort(symbols.begin(), symbols.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    build_codes_recursive(symbols, 0, symbols.size() - 1, "");
}

void Dictionary::build_codes_recursive(const std::vector<std::pair<char, uint32_t>>& symbols,
                                       size_t start, size_t end, const std::string& code) {
    if (start == end) {
        symbol_map[symbols[start].first].code = code.empty() ? "0" : code;
        code_map[code.empty() ? "0" : code] = symbols[start].first;
        return;
    }

    size_t split = start;
    uint32_t total = std::accumulate(symbols.begin() + start, symbols.begin() + end + 1, 0u,
                                     [](uint32_t sum, const auto& p) { return sum + p.second; });

    uint32_t min_diff = UINT32_MAX, current_sum = 0;
    for (size_t i = start; i < end; ++i) {
        current_sum += symbols[i].second;
        uint32_t diff = abs(2 * current_sum - total);
        if (diff < min_diff) {
            min_diff = diff;
            split = i;
        }
    }

    build_codes_recursive(symbols, start, split, code + "0");
    build_codes_recursive(symbols, split + 1, end, code + "1");
}

void Dictionary::save_to_file(const std::string& filename) const {
    std::ofstream out(filename, std::ios::binary);
    if (!out) throw std::runtime_error("Cannot open dictionary file for writing");

    const char signature[] = "SFAN";
    out.write(signature, 4);
    uint32_t version = 1;
    out.write(reinterpret_cast<const char*>(&version), sizeof(version));

    uint32_t size = symbol_map.size();
    out.write(reinterpret_cast<const char*>(&size), sizeof(size));

    for (const auto& [symbol, info] : symbol_map) {
        out.put(symbol);
        out.write(reinterpret_cast<const char*>(&info.frequency), sizeof(info.frequency));
        uint8_t code_length = info.code.size();
        out.write(reinterpret_cast<const char*>(&code_length), sizeof(code_length));
        out.write(info.code.c_str(), code_length);
    }

    uint32_t crc = 0;
    out.write(reinterpret_cast<const char*>(&crc), sizeof(crc));
}

void Dictionary::load_from_file(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in) throw std::runtime_error("Cannot open dictionary file");

    char signature[5] = {0};
    in.read(signature, 4);
    if (std::string(signature) != "SFAN") {
        throw std::runtime_error("Invalid dictionary file format");
    }

    uint32_t version;
    in.read(reinterpret_cast<char*>(&version), sizeof(version));
    if (version != 1) throw std::runtime_error("Unsupported dictionary version");

    symbol_map.clear();
    code_map.clear();

    uint32_t size;
    in.read(reinterpret_cast<char*>(&size), sizeof(size));

    for (uint32_t i = 0; i < size; ++i) {
        char symbol;
        in.get(symbol);

        uint32_t frequency;
        in.read(reinterpret_cast<char*>(&frequency), sizeof(frequency));

        uint8_t code_length;
        in.read(reinterpret_cast<char*>(&code_length), sizeof(code_length));

        std::string code(code_length, '\0');
        in.read(&code[0], code_length);

        symbol_map[symbol] = {frequency, code};
        code_map[code] = symbol;
    }
}

const std::string& Dictionary::get_code(char symbol) const {
    static const std::string empty;
    auto it = symbol_map.find(symbol);
    return it != symbol_map.end() ? it->second.code : empty;
}

std::optional<char> Dictionary::get_symbol(const std::string& code) const {
    auto it = code_map.find(code);
    return it != code_map.end() ? std::optional<char>(it->second) : std::nullopt;
}

void Dictionary::print_statistics() const {
    std::ofstream log("stats.log", std::ios::app);
    if (!log) return;

    log << "Dictionary Statistics:\n";
    log << "Total symbols: " << total_symbols << "\n";
    log << "Unique symbols: " << symbol_map.size() << "\n\n";

    std::vector<std::pair<char, SymbolInfo>> sorted(symbol_map.begin(), symbol_map.end());
    std::sort(sorted.begin(), sorted.end(),
              [](const auto& a, const auto& b) { return a.second.frequency > b.second.frequency; });

    log << "Top 5 frequent symbols:\n";
    for (size_t i = 0; i < std::min(size_t(5), sorted.size()); ++i) {
        log << "  '" << sorted[i].first << "': " << sorted[i].second.frequency
            << " (" << std::fixed << std::setprecision(2)
            << (100.0 * sorted[i].second.frequency / total_symbols) << "%)\n";
    }
}
