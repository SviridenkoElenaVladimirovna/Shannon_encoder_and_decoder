#include "Encoder.h"
#include "Decoder.h"
#include <iostream>
#include <fstream>
#include <cassert>

int main() {
    const std::string input_file = "test_input.bin";
    const std::string compressed_file = "test_compressed.bin";
    const std::string dict_file = "test_dict.bin";
    const std::string output_file = "test_output.bin";
    std::string test_data = "abracadabra";

    std::ofstream input_out(input_file, std::ios::binary);
    input_out.write(test_data.c_str(), test_data.size());
    input_out.close();

    Encoder encoder;
    encoder.encode(input_file, compressed_file, dict_file);
    std::cout << "Compression completed\n";

    Decoder decoder;
    decoder.decode(compressed_file, output_file, dict_file);
    std::cout << "Decompression completed\n";

    std::ifstream result_in(output_file, std::ios::binary);
    std::string result_data((std::istreambuf_iterator<char>(result_in)),
                            std::istreambuf_iterator<char>());
    result_in.close();

    if (result_data == test_data) {
        std::cout << "successfully\n";
    } else {
        std::cout << "failed: Data mismatch!\n";
    }

    remove(input_file.c_str());
    remove(compressed_file.c_str());
    remove(dict_file.c_str());
    remove(output_file.c_str());

    return 0;
}
