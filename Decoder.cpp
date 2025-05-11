#include "Decoder.h"
#include "Dictionary.h"
#include <fstream>

void Decoder::decode(const std::string& input_file,
                     const std::string& output_file,
                     const std::string& dict_file) {
    Dictionary dict;
    dict.load_from_file(dict_file);

    std::ifstream in(input_file, std::ios::binary);
    std::ofstream out(output_file, std::ios::binary);

    uint32_t original_size = 0;
    in.read(reinterpret_cast<char*>(&original_size), sizeof(original_size));

    std::string current_code;
    char byte;
    size_t decoded_symbols = 0;

    while (in.get(byte) && decoded_symbols < original_size) {
        for (int i = 7; i >= 0; --i) {
            current_code += (byte & (1 << i)) ? '1' : '0';

            char symbol = dict.get_symbol(current_code);
            if (symbol != '\0') {
                out.put(symbol);
                current_code.clear();
                decoded_symbols++;

                if (decoded_symbols >= original_size)
                    break;
            }
        }
    }
}
