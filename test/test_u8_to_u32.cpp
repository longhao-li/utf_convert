#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "utf_convert.hpp"

using namespace utf_convert;

char s[1024];

int main(int argc, char **argv) {
    if (argc != 3) {
        return 0;
    }

    std::string u8_filename = argv[1];

    FILE *u8_file  = fopen(u8_filename.c_str(), "r");
    FILE *out_file = fopen("out.txt", "w");

    std::string u8;
    while (!std::feof(u8_file)) {
        u8.push_back(std::fgetc(u8_file));
    }

    std::u32string converted_str;
    to_u32string(u8, converted_str, UTF_ENDIAN_LITTLE_ENDIAN);

    bool temp = false;
    for (size_t i = 0; i < converted_str.size(); i++) {
        if (temp)
            std::fprintf(out_file, " ");
        else
            temp = true;
        std::fprintf(out_file, "%08x", (uint32_t)converted_str[i]);
    }

    std::fclose(u8_file);
    std::fclose(out_file);

    std::string cmd = "diff ";
    cmd += std::string("out.txt ");
    cmd += std::string(argv[2]);

    std::printf("cmd: %s\n", cmd.c_str());
    assert(!std::system(cmd.c_str()));
    return 0;
}
