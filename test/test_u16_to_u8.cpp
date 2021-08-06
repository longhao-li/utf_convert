#include <array>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "utf_convert.hpp"

using namespace utf_convert;

uint32_t read_hex(const std::string &str) {
    uint32_t res = 0;
    for (size_t i = 0; i < str.size(); i++) {
        if (str[i] >= '0' && str[i] <= '9') {
            res = res * 16 + str[i] - '0';
        } else if (str[i] >= 'a' && str[i] <= 'f') {
            res = res * 16 + str[i] - 'a' + 10;
        } else if (str[i] >= 'A' && str[i] <= 'F') {
            res = res * 16 + str[i] - 'A' + 10;
        }
    }
    return res;
}

void simple_test(const std::vector<int> &arr, const std::string &ans) {
    std::u16string u16;
    for (size_t i = 0; i < arr.size(); i++) {
        u16.push_back(arr[i]);
    }

    std::string u8;
    assert(to_u8string(u16, UTF_ENDIAN_LITTLE_ENDIAN, u8));
    printf("\nu8:  ");
    for (auto &i : u8) {
        printf("%02x ", (uint8_t)i);
    }
    printf("\nans: ");
    for (auto &i : ans) {
        printf("%02x ", (uint8_t)i);
    }
    printf("\n");

    assert(u8 == ans);
}

int main(int argc, char **argv) {
    std::vector<int> vec1{0x0048,
                          0x0065,
                          0x006c,
                          0x006c,
                          0x006f,
                          0x002c,
                          0x0020,
                          0x0077,
                          0x006f,
                          0x0072,
                          0x006c,
                          0x0064,
                          0x0021};
    std::string      ans1 = "Hello, world!";

    simple_test(vec1, ans1);

    std::vector<int> vec2{0x4f60, 0x597d, 0xff0c, 0x4e16, 0x754c, 0xff01};
    std::string      ans2 = "你好，世界！";

    simple_test(vec2, ans2);

    std::vector<int> vec3{0x0072, 0x0065, 0x0067, 0x0075, 0x006c, 0x0061,
                          0x0072, 0x0020, 0x0074, 0x0065, 0x0078, 0x0074,
                          0x0020, 0x002d, 0x0020, 0x24be, 0x24c7, 0x24c7,
                          0x24ba, 0x24bc, 0x24ca, 0x24c1, 0x24b6, 0x24c7,
                          0x2423, 0x24c9, 0x24ba, 0x24cd, 0x24c9};
    std::string ans3 = "regular text - ⒾⓇⓇⒺⒼⓊⓁⒶⓇ␣ⓉⒺⓍⓉ";

    simple_test(vec3, ans3);

    if (argc != 3)
        return 0;

    std::fstream u16_file(argv[1]);
    std::string  temp;

    std::u16string u16;

    while (u16_file >> temp) {
        uint32_t value = read_hex(temp);
        // std::cout << value << std::endl;
        if (value > 0xfffd) {
            uint16_t value_high = value >> 16;
            uint16_t value_low = value & 0xffff;
            std::cout << "high: " << value_high << " low: " << value_low << "\n";
            u16.push_back(value_high);
            u16.push_back(value_low);
        } else {
            std::cout << "value: " << value << "\n";
            u16.push_back(value);
        }
    }

    u16_file.close();
    std::string u8;

    to_u8string(u16, UTF_ENDIAN_LITTLE_ENDIAN, u8);
    FILE *out = std::fopen("out.txt", "w");
    for (size_t i = 0; i < u8.size(); i++) {
        std::fputc(u8[i], out);
    }
    std::fclose(out);

    std::string cmd = "diff out.txt ";
    cmd += std::string(argv[2]);
    assert(!std::system(cmd.c_str()));
    return 0;
}
