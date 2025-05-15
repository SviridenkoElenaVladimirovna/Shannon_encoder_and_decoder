#include "Encoder.h"
#include "Dictionary.h"
#include <fstream>
#include <stdexcept>

void Encoder::encode(const std::string& input_file,
                     const std::string& output_file,
                     const std::string& dict_file) {
    std::ifstream in(input_file, std::ios::binary);
    if (!in) throw std::runtime_error("Cannot open input file");

    std::string data((std::istreambuf_iterator<char>(in)),
                     std::istreambuf_iterator<char>());

    Dictionary dict;
    dict.analyze_frequencies(data);
    dict.save_to_file(dict_file);

    std::ofstream out(output_file, std::ios::binary);
    if (!out) throw std::runtime_error("Cannot open output file");

    uint32_t original_size = static_cast<uint32_t>(data.size());
    out.write(reinterpret_cast<const char*>(&original_size), sizeof(original_size));

    if (data.empty()) {
        uint8_t bit_count = 0;
        out.write(reinterpret_cast<const char*>(&bit_count), sizeof(bit_count));
        return;
    }

    uint8_t buffer = 0;
    uint8_t bit_pos = 0;
    uint64_t total_bits = 0;

    std::string bitstream;

    for (char c : data) {
        const std::string& code = dict.get_code(c);
        if (code.empty()) {
            throw std::runtime_error("No code found for symbol: " + std::string(1, c));
        }

        for (char bit : code) {
            if (bit == '1') {
                buffer |= (1 << (7 - bit_pos));
            }
            bit_pos++;
            total_bits++;

            if (bit_pos == 8) {
                out.put(buffer);
                buffer = 0;
                bit_pos = 0;
            }
        }
    }

    if (bit_pos > 0) {
        out.put(buffer);
    }

    uint8_t useful_bits = static_cast<uint8_t>(total_bits % 8);
    if (useful_bits == 0) useful_bits = 8;
    out.write(reinterpret_cast<const char*>(&useful_bits), sizeof(useful_bits));
}
