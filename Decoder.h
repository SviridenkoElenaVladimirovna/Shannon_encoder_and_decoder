#ifndef DECODER_H
#define DECODER_H

#include "Dictionary.h"
#include <string>

class Decoder {
public:
    void decode(const std::string& input_file,
                const std::string& output_file,
                const std::string& dict_file);
};

#endif
