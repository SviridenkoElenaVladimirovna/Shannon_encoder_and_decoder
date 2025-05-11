#include "Dictionary.h"
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <bitset>

void Dictionary::analyze_frequencies(const std::string& data) {
    symbol_map.clear();
    code_map.clear();
    total_symbols = data.size();
    for (char c : data) {
        symbol_map[c].frequency++;
    }

    std::vector<std::pair<char, uint32_t>> symbols;
    symbols.reserve(symbol_map.size());
    for (const auto& [symbol, info] : symbol_map) {
        symbols.emplace_back(symbol, info.frequency);
    }

    std::sort(symbols.begin(), symbols.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    build_shannon_codes(symbols, 0, symbols.size() - 1, "");
}

void Dictionary::build_shannon_codes(const std::vector<std::pair<char, uint32_t>>& symbols,
                                     int start, int end,
                                     const std::string& code) {
    if (start == end) {
        if (!code.empty()) {
            symbol_map[symbols[start].first].code = code;
            code_map[code] = symbols[start].first;
        }
        return;
    }
    int mid = start;
    uint32_t left_sum = 0;
    uint32_t total = 0;

    for (int i = start; i <= end; ++i) total += symbols[i].second;

    for (mid = start; mid <= end; ++mid) {
        left_sum += symbols[mid].second;
        if (left_sum * 2 >= total) break;
    }

    build_shannon_codes(symbols, start, mid, code + "0");
    build_shannon_codes(symbols, mid + 1, end, code + "1");
}

void Dictionary::save_to_file(const std::string& filename) const {
    std::ofstream out(filename, std::ios::binary);
    uint32_t size = symbol_map.size();
    out.write(reinterpret_cast<const char*>(&size), sizeof(size));

    for (const auto& [symbol, info] : symbol_map) {
        out.put(symbol);
        out.write(reinterpret_cast<const char*>(&info.frequency), sizeof(info.frequency));
        uint8_t code_length = info.code.size();
        out.write(reinterpret_cast<const char*>(&code_length), sizeof(code_length));
        out.write(info.code.c_str(), code_length);
    }
}

void Dictionary::load_from_file(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    uint32_t size;
    in.read(reinterpret_cast<char*>(&size), sizeof(size));

    symbol_map.clear();
    code_map.clear();

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

char Dictionary::get_symbol(const std::string& code) const {
    auto it = code_map.find(code);
    return it != code_map.end() ? it->second : '\0';
}
void Dictionary::print_statistics() const {
    std::ofstream log("stats.log", std::ios::app);

    if (!log.is_open()) {
        std::cerr << "Failed to open stats.log\n";
        return;
    }

    log << "Input analysis:\n";
    log << "  Total symbols      : " << total_symbols << "\n";
    log << "  Unique symbols     : " << symbol_map.size() << "\n";

    auto max_it = std::max_element(symbol_map.begin(), symbol_map.end(),
                                   [](const auto& a, const auto& b) {
                                       return a.second.frequency < b.second.frequency;
                                   });

    auto min_it = std::min_element(symbol_map.begin(), symbol_map.end(),
                                   [](const auto& a, const auto& b) {
                                       return a.second.frequency < b.second.frequency;
                                   });

    auto printable = [](char c) -> std::string {
        if (c == ' ') return "' '";
        if (c == '\n') return "'\\n'";
        if (c == '\t') return "'\\t'";
        if (isprint(c)) return std::string("'") + c + "'";
        std::ostringstream oss;
        oss << "'\\x" << std::hex << std::uppercase << (int)(unsigned char)c << "'";
        return oss.str();
    };

    log << "  Most frequent      : " << printable(max_it->first)
        << " — " << max_it->second.frequency
        << " times (" << std::fixed << std::setprecision(1)
        << (100.0 * max_it->second.frequency / total_symbols) << "%)\n";

    log << "  Least frequent     : " << printable(min_it->first)
        << " — " << min_it->second.frequency
        << " time" << (min_it->second.frequency > 1 ? "s" : "") << " ("
        << (100.0 * min_it->second.frequency / total_symbols) << "%)\n\n";

    log << "Top 5 symbols:\n";

    std::vector<std::pair<char, SymbolInfo>> sorted(symbol_map.begin(), symbol_map.end());
    std::sort(sorted.begin(), sorted.end(),
              [](const auto& a, const auto& b) {
                  return a.second.frequency > b.second.frequency;
              });

    int count = std::min(5, (int)sorted.size());
    for (int i = 0; i < count; ++i) {
        const auto& [ch, info] = sorted[i];
        log << "  " << std::setw(4) << printable(ch)
            << " : " << std::setw(4) << info.frequency
            << " times — " << std::fixed << std::setprecision(1)
            << (100.0 * info.frequency / total_symbols) << "%\n";
    }

    log.close();
}
