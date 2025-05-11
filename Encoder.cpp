#include "Encoder.h"
#include "Dictionary.h"
#include <fstream>
#include <bitset>

void Encoder::encode(const std::string& input_file,
                     const std::string& output_file,
                     const std::string& dict_file) {
    std::ifstream in(input_file, std::ios::binary);
    std::string data((std::istreambuf_iterator<char>(in)),
                     std::istreambuf_iterator<char>());

    Dictionary dict;
    dict.analyze_frequencies(data);
    dict.print_statistics();
    dict.save_to_file(dict_file);

    std::ofstream out(output_file, std::ios::binary);

    uint32_t original_size = static_cast<uint32_t>(data.size());
    out.write(reinterpret_cast<const char*>(&original_size), sizeof(original_size));

    unsigned char buffer = 0;
    int bit_pos = 0;

    for (char c : data) {
        const std::string& code = dict.get_code(c);

        for (char bit : code) {
            buffer |= (bit == '1') << (7 - bit_pos);
            if (++bit_pos == 8) {
                out.put(buffer);
                buffer = 0;
                bit_pos = 0;
            }
        }
    }

    if (bit_pos > 0) {
        out.put(buffer);
    }
}
