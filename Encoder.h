#ifndef ENCODER_H
#define ENCODER_H

#include "Dictionary.h"
#include <string>

class Encoder {
public:
    void encode(const std::string& input_file,
                const std::string& output_file,
                const std::string& dict_file);
};

#endif
