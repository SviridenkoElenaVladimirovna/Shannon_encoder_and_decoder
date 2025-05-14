#include <gtest/gtest.h>
#include "Encoder.h"
#include "Decoder.h"
#include <fstream>
#include <filesystem>
#include <random>
#include <algorithm>

namespace fs = std::filesystem;

class CodecTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = fs::temp_directory_path() / "codec_test";
        fs::remove_all(test_dir);
        fs::create_directory(test_dir);

        input_file = test_dir / "input.bin";
        compressed_file = test_dir / "compressed.bin";
        dict_file = test_dir / "dict.bin";
        output_file = test_dir / "output.bin";
    }

    void TearDown() override {
        fs::remove_all(test_dir);
    }

    void create_test_file(const fs::path& path, const std::string& content) {
        std::ofstream out(path, std::ios::binary);
        if (!out) throw std::runtime_error("Cannot create test file");
        out.write(content.data(), content.size());
    }

    bool compare_files(const fs::path& p1, const fs::path& p2) {
        std::ifstream f1(p1, std::ios::binary);
        std::ifstream f2(p2, std::ios::binary);

        if (!f1 || !f2) return false;

        return std::equal(std::istreambuf_iterator<char>(f1),
                          std::istreambuf_iterator<char>(),
                          std::istreambuf_iterator<char>(f2),
                          std::istreambuf_iterator<char>());
    }

    void encode_decode() {
        Encoder encoder;
        encoder.encode(input_file.string(), compressed_file.string(), dict_file.string());

        Decoder decoder;
        decoder.decode(compressed_file.string(), output_file.string(), dict_file.string());
    }

    fs::path test_dir;
    fs::path input_file;
    fs::path compressed_file;
    fs::path dict_file;
    fs::path output_file;
};

TEST_F(CodecTest, EmptyFile) {
    std::ofstream(input_file).close();

    {
        Encoder encoder;
        ASSERT_NO_THROW(encoder.encode(input_file.string(), compressed_file.string(), dict_file.string()));
        ASSERT_TRUE(fs::exists(compressed_file));
        ASSERT_TRUE(fs::exists(dict_file));
    }

    {
        Decoder decoder;
        ASSERT_NO_THROW(decoder.decode(compressed_file.string(), output_file.string(), dict_file.string()));
        ASSERT_TRUE(fs::exists(output_file));
        ASSERT_EQ(fs::file_size(output_file), 0);
    }
}

TEST_F(CodecTest, SingleCharacter) {
    for (char c : {'a', '0', '\n', '\xff'}) {
        create_test_file(input_file, std::string(1, c));

        ASSERT_NO_THROW(encode_decode());
        EXPECT_TRUE(compare_files(input_file, output_file)) << "Failed for character: " << (int)c;

        fs::remove(compressed_file);
        fs::remove(dict_file);
        fs::remove(output_file);
    }
}

TEST_F(CodecTest, RepeatedCharacters) {
    const std::vector<std::string> test_cases = {
            std::string(100, 'a'),
            std::string(1000, '\0'),
            std::string(500, 'x') + std::string(500, 'y')
    };

    for (const auto& data : test_cases) {
        create_test_file(input_file, data);

        ASSERT_NO_THROW(encode_decode());
        EXPECT_TRUE(compare_files(input_file, output_file)) << "Failed for data: " << data.substr(0, 20);

        fs::remove(compressed_file);
        fs::remove(dict_file);
        fs::remove(output_file);
    }
}

TEST_F(CodecTest, AllPossibleBytes) {
    std::string all_bytes;
    for (int i = 0; i < 256; ++i) {
        all_bytes += static_cast<char>(i);
    }

    std::shuffle(all_bytes.begin(), all_bytes.end(), std::mt19937{std::random_device{}()});

    create_test_file(input_file, all_bytes);

    ASSERT_NO_THROW(encode_decode());
    EXPECT_TRUE(compare_files(input_file, output_file));
}

TEST_F(CodecTest, LargeRandomFile) {
    const size_t size = 1024 * 1024;
    std::string data;
    data.reserve(size);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    for (size_t i = 0; i < size; ++i) {
        data += static_cast<char>(dis(gen));
    }

    create_test_file(input_file, data);

    ASSERT_NO_THROW(encode_decode());
    EXPECT_TRUE(compare_files(input_file, output_file));
}

TEST_F(CodecTest, TextData) {
    const std::vector<std::string> texts = {
            "The quick brown fox jumps over the lazy dog",
            "Hello, world!",
            "Привет, мир!",
            "Line1\nLine2\tTabbed"
    };

    for (const auto& text : texts) {
        create_test_file(input_file, text);

        ASSERT_NO_THROW(encode_decode());
        EXPECT_TRUE(compare_files(input_file, output_file)) << "Failed for text: " << text;

        fs::remove(compressed_file);
        fs::remove(dict_file);
        fs::remove(output_file);
    }
}

TEST_F(CodecTest, MultipleOperations) {
    const std::string test_data = "Test data for multiple operations";

    for (int i = 0; i < 5; ++i) {
        create_test_file(input_file, test_data + std::to_string(i));

        ASSERT_NO_THROW(encode_decode());
        EXPECT_TRUE(compare_files(input_file, output_file)) << "Failed on iteration " << i;

        fs::remove(compressed_file);
        fs::remove(dict_file);
        fs::remove(output_file);
    }
}
TEST_F(CodecTest, RepeatedDecodingProducesSameResult) {
    std::string content = "Redundancy check!";

    create_test_file(input_file, content);
    encode_decode();

    fs::path second_output = test_dir / "output2.bin";
    Decoder decoder;
    decoder.decode(compressed_file.string(), second_output.string(), dict_file.string());

    EXPECT_TRUE(compare_files(output_file, second_output));
}
TEST_F(CodecTest, CorruptedCompressedFile) {
    create_test_file(input_file, "Some data to encode");
    encode_decode();

    std::ofstream out(compressed_file, std::ios::binary | std::ios::in);
    out.seekp(2);
    out.put('\xFF');
    out.close();

    Decoder decoder;
    EXPECT_THROW(decoder.decode(compressed_file.string(), output_file.string(), dict_file.string()), std::exception);
}
TEST_F(CodecTest, DictionaryEdgeCases) {
    create_test_file(input_file, std::string(1024, 'A'));
    ASSERT_NO_THROW(encode_decode());
    EXPECT_TRUE(compare_files(input_file, output_file));

    std::string diverse;
    for (int i = 0; i < 256; ++i) diverse += static_cast<char>(i);
    std::string extended = diverse;
    while (extended.size() < 4096) extended += diverse;

    create_test_file(input_file, extended);
    ASSERT_NO_THROW(encode_decode());
    EXPECT_TRUE(compare_files(input_file, output_file));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}