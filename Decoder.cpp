#include "Decoder.h"
#include "Dictionary.h"
#include <fstream>
#include <vector>

void Decoder::decode(const std::string& input_file,
                     const std::string& output_file,
                     const std::string& dict_file) {
    Dictionary dict;
    dict.load_from_file(dict_file);

    std::ifstream in(input_file, std::ios::binary);
    if (!in) throw std::runtime_error("Cannot open input file");

    uint32_t original_size;
    in.read(reinterpret_cast<char*>(&original_size), sizeof(original_size));

    if (!original_size) {
        std::ofstream(output_file).close();
        return;
    }

    std::vector<char> file_data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if (file_data.empty()) throw std::runtime_error("Compressed file is empty");

    uint8_t useful_bits_in_last_byte = static_cast<uint8_t>(file_data.back());
    file_data.pop_back();

    size_t total_bits = (file_data.size() - 1) * 8 + useful_bits_in_last_byte;

    std::vector<bool> bits;
    bits.reserve(total_bits);

    for (size_t i = 0; i < file_data.size(); ++i) {
        char byte = file_data[i];
        int bit_count = (i == file_data.size() - 1) ? useful_bits_in_last_byte : 8;

        for (int b = 7; b >= 8 - bit_count; --b) {
            bits.push_back((byte >> b) & 1);
        }
    }

    std::ofstream out(output_file, std::ios::binary);
    if (!out) throw std::runtime_error("Cannot open output file");

    size_t decoded = 0;
    size_t pos = 0;

    while (decoded < original_size) {
        bool found = false;
        for (size_t len = 1; len <= Dictionary::MAX_CODE_LENGTH && pos + len <= bits.size(); ++len) {
            std::string code;
            for (size_t i = 0; i < len; ++i) {
                code += bits[pos + i] ? '1' : '0';
            }

            auto symbol_opt = dict.get_symbol(code);
            if (symbol_opt.has_value()) {
                out.put(symbol_opt.value());
                decoded++;
                pos += len;
                found = true;
                break;
            }
        }

        if (!found) {
            throw std::runtime_error("Invalid or incomplete bitstream during decoding");
        }
    }

    if (pos < bits.size()) {
        std::string remaining_code;
        for (size_t i = pos; i < bits.size(); ++i)
            remaining_code += bits[i] ? '1' : '0';
        throw std::runtime_error("Extra bits remaining after decoding: " + remaining_code);
    }
}
