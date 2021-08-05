#include "utf_convert.hpp"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>

namespace {
union utf32 {
    char32_t ch;
    uint8_t  v[4];
};

bool convert_u32str_to_u8str_without_bom(const uint8_t *         u32str,
                                         size_t                  u32size,
                                         utf_convert::UTF_ENDIAN endian,
                                         std::string &           target) {
    for (size_t i = 0; i < u32size; i++) {
        const uint8_t *cur = u32str + i * (sizeof(char32_t) / sizeof(uint8_t));
        uint32_t       value = 0;

        if (endian == utf_convert::UTF_ENDIAN_BIG_ENDIAN) {
            value = ((static_cast<uint32_t>(cur[0]) << 24) |
                     (static_cast<uint32_t>(cur[1]) << 16) |
                     (static_cast<uint32_t>(cur[2]) << 8) | cur[3]);
        } else if (endian == utf_convert::UTF_ENDIAN_LITTLE_ENDIAN) {
            value = ((static_cast<uint32_t>(cur[3]) << 24) |
                     (static_cast<uint32_t>(cur[2]) << 16) |
                     (static_cast<uint32_t>(cur[1]) << 8) | cur[0]);
        } else {
            return false;  // Unsupported endian
        }

        if (value < 0x80) {
            /*
             * +-----------------------------------------+
             * |                  UTF-32                 |
             * | 0000 0000 0000 0000 0000 0000 0ABC DEFG |
             * +-----------------------------------------+
             * The single byte is 0ABC DEFG
             */
            target.push_back(value);
        } else if (value < 0x0800) {
            /*
             * +-----------------------------------------+
             * |                  UTF-32                 |
             * | 0000 0000 0000 0000 0000 0ABC DEFG HIJK |
             * +-----------------------------------------+
             * The higher byte is 110A BCDE
             * The lower byte is 10FG HIJK
             */
            target.push_back((value >> 6) & 0x1f | 0xc0);
            target.push_back(value & 0x3f | 0x80);
        } else if (value < 0x010000) {
            /*
             * +-----------------------------------------+
             * |                  UTF-32                 |
             * | 0000 0000 0000 0000 ABCD EFGH IJKL MNOP |
             * +-----------------------------------------+
             * The first byte is 1110 ABCD
             * The second byte is  10EF GHIJ
             * The third byte is 10KL MNOP
             */
            target.push_back((value >> 12) & 0x0f | 0xe0);
            target.push_back((value >> 6) & 0x3f | 0x80);
            target.push_back(value & 0x3f | 0x80);
        } else if (value < 0x110000) {
            target.push_back((value >> 18) & 0x07 | 0xf0);
            target.push_back((value >> 12) & 0x3f | 0x80);
            target.push_back((value >> 6) & 0x3f | 0x80);
            target.push_back(value & 0x3f | 0x80);
        } else {
            return false;
        }
    }
    return true;
}

char32_t get_bom(utf_convert::UTF_ENDIAN endian) {
    utf32 res;
    if (endian == utf_convert::UTF_ENDIAN_BIG_ENDIAN) {
        res.v[0] = 0x00;
        res.v[1] = 0x00;
        res.v[2] = 0xfe;
        res.v[3] = 0xff;
        return res.ch;
    } else if (endian == utf_convert::UTF_ENDIAN_LITTLE_ENDIAN) {
        res.v[0] = 0xff;
        res.v[1] = 0xfe;
        res.v[2] = 0x00;
        res.v[3] = 0x00;
        return res.ch;
    } else {
        return 0;
    }
}

bool match_bom(char32_t bom, utf_convert::UTF_ENDIAN endian) {
    utf32 u32;
    u32.ch = bom;
    if (u32.v[0] == 0x00 && u32.v[1] == 0x00 && u32.v[2] == 0xfe &&
        u32.v[3] == 0xff) {
        return endian == utf_convert::UTF_ENDIAN_BIG_ENDIAN;
    } else if (u32.v[0] == 0xff && u32.v[1] == 0xfe && u32.v[2] == 0x00 &&
               u32.v[3] == 0x00) {
        return endian == utf_convert::UTF_ENDIAN_LITTLE_ENDIAN;
    }
    return false;
}

bool convert_u8str_to_u32str_little_endian(const std::string &u8str,
                                           std::u32string &   target) {
    utf32 u32;

    for (size_t i = 0; i < u8str.size();) {
        u32.ch = 0;

        if ((u8str[i] & 0xf0) == 0xf0) {
            /*
             * +-----------------------------------------------+
             * |                     UTF-8                     |
             * +-----------+-----------+-----------+-----------+
             * | 1111 0ABC | 10DE FGHI | 10JK LMNO | 10PQ RSTU |
             * +-----------+-----------+-----------+-----------+
             *
             * The utf-32 character is
             * 0000 0000 000A BCDE FGHI JKLM NOPQ RSTU
             */

            if (i + 3 >= u8str.size())
                return false;

            u32.v[3] = 0;
            u32.v[2] = ((u8str[i] & 0x07) << 2) | ((u8str[i + 1] & 0x30) >> 4);
            u32.v[1] =
                ((u8str[i + 1] & 0x0f) << 4) | ((u8str[i + 2] & 0x3c) >> 2);
            u32.v[0] = ((u8str[i + 2] & 0x03) << 6) | ((u8str[i + 3] & 0x3f));

            target.push_back(u32.ch);
            i += 4;
        } else if ((u8str[i] & 0xe0) == 0xe0) {
            /*
             * +-----------------------------------+
             * |               UTF-8               |
             * +-----------+-----------+-----------+
             * | 1110 ABCD | 10EF GHIJ | 10KL MNOP |
             * +-----------+-----------+-----------+
             *
             * The utf-32 character is
             * 0000 0000 0000 0000 ABCD EFGH IJKL MNOP
             */

            if (i + 2 >= u8str.size())
                return false;

            u32.v[3] = 0;
            u32.v[2] = 0;
            u32.v[1] = ((u8str[i] & 0x0f) << 4) | ((u8str[i + 1] & 0x3c) >> 2);
            u32.v[0] = ((u8str[i + 1] & 0x03) << 6) | ((u8str[i + 2] & 0x3f));

            target.push_back(u32.ch);
            i += 3;
        } else if ((u8str[i] & 0xc0) == 0xc0) {
            /*
             * +-----------------------+
             * |         UTF-8         |
             * +-----------------------+
             * | 110A BCDE | 10FG HIJK |
             * +-----------------------+
             *
             * The utf-32 character is
             * 0000 0000 0000 0000 0000 0ABC DEFG HIJK
             */

            if (i + 1 >= u8str.size())
                return false;

            u32.v[3] = 0;
            u32.v[2] = 0;
            u32.v[1] = ((u8str[i] & 0x1c) >> 2);
            u32.v[0] = ((u8str[i] & 0x03) << 6) | (u8str[i + 1] & 0x3f);

            target.push_back(u32.ch);
            i += 2;
        } else if ((u8str[i] & 0x7f) == u8str[i]) {
            u32.v[3] = u32.v[2] = u32.v[1] = 0;
            u32.v[0]                       = u8str[i];

            target.push_back(u32.ch);
            i += 1;
        } else {
            return false;
        }
    }
    return true;
}

bool convert_u8str_to_u32str_big_endian(const std::string &u8str,
                                        std::u32string &   target) {
    utf32 u32;

    for (size_t i = 0; i < u8str.size();) {
        u32.ch = 0;

        if ((u8str[i] & 0xf0) == 0xf0) {
            /*
             * +-----------------------------------------------+
             * |                     UTF-8                     |
             * +-----------+-----------+-----------+-----------+
             * | 1111 0ABC | 10DE FGHI | 10JK LMNO | 10PQ RSTU |
             * +-----------+-----------+-----------+-----------+
             *
             * The utf-32 character is
             * 0000 0000 000A BCDE FGHI JKLM NOPQ RSTU
             */

            if (i + 3 >= u8str.size())
                return false;

            u32.v[0] = 0;
            u32.v[1] = ((u8str[i] & 0x07) << 2) | ((u8str[i + 1] & 0x30) >> 4);
            u32.v[2] =
                ((u8str[i + 1] & 0x0f) << 4) | ((u8str[i + 2] & 0x3c) >> 2);
            u32.v[3] = ((u8str[i + 2] & 0x03) << 6) | ((u8str[i + 3] & 0x3f));

            target.push_back(u32.ch);
            i += 4;
        } else if ((u8str[i] & 0xe0) == 0xe0) {
            /*
             * +-----------------------------------+
             * |               UTF-8               |
             * +-----------+-----------+-----------+
             * | 1110 ABCD | 10EF GHIJ | 10KL MNOP |
             * +-----------+-----------+-----------+
             *
             * The utf-32 character is
             * 0000 0000 0000 0000 ABCD EFGH IJKL MNOP
             */

            if (i + 2 >= u8str.size())
                return false;

            u32.v[0] = 0;
            u32.v[1] = 0;
            u32.v[2] = ((u8str[i] & 0x0f) << 4) | ((u8str[i + 1] & 0x3c) >> 2);
            u32.v[3] = ((u8str[i + 1] & 0x03) << 6) | ((u8str[i + 2] & 0x3f));

            target.push_back(u32.ch);
            i += 3;
        } else if ((u8str[i] & 0xc0) == 0xc0) {
            /*
             * +-----------------------+
             * |         UTF-8         |
             * +-----------------------+
             * | 110A BCDE | 10FG HIJK |
             * +-----------------------+
             *
             * The utf-32 character is
             * 0000 0000 0000 0000 0000 0ABC DEFG HIJK
             */

            if (i + 1 >= u8str.size())
                return false;

            u32.v[0] = 0;
            u32.v[1] = 0;
            u32.v[2] = ((u8str[i] & 0x1c) >> 2);
            u32.v[3] = ((u8str[i] & 0x03) << 6) | (u8str[i + 1] & 0x3f);

            target.push_back(u32.ch);
            i += 2;
        } else if ((u8str[i] & 0x7f) == u8str[i]) {
            u32.v[0] = u32.v[1] = u32.v[2] = 0;
            u32.v[3]                       = u8str[i];

            target.push_back(u32.ch);
            i += 1;
        } else {
            return false;
        }
    }
    return true;
}
}  // namespace

bool utf_convert::to_u8string(const std::u32string &u32str_without_bom,
                              UTF_ENDIAN            u32str_endian,
                              std::string &         target) {
    target.clear();

    return convert_u32str_to_u8str_without_bom(
        reinterpret_cast<const uint8_t *>(u32str_without_bom.data()),
        u32str_without_bom.size(),
        u32str_endian,
        target);
}

bool utf_convert::to_u8string(const std::u32string &u32str_with_bom,
                              std::string &         target) {
    if (u32str_with_bom.empty())
        return false;  // u32string with bom should never be empty.

    target.clear();

    const uint8_t *u32str_ptr =
        reinterpret_cast<const uint8_t *>(u32str_with_bom.data());

    if (match_bom(u32str_with_bom[0], utf_convert::UTF_ENDIAN_BIG_ENDIAN)) {
        // Big endian
        return convert_u32str_to_u8str_without_bom(
            u32str_ptr + 4,
            u32str_with_bom.size() - 1,
            utf_convert::UTF_ENDIAN_BIG_ENDIAN,
            target);

    } else if (match_bom(u32str_with_bom[0],
                         utf_convert::UTF_ENDIAN_LITTLE_ENDIAN)) {
        // Little endian
        return convert_u32str_to_u8str_without_bom(
            u32str_ptr + 4,
            u32str_with_bom.size() - 1,
            utf_convert::UTF_ENDIAN_LITTLE_ENDIAN,
            target);
    } else {
        return false;  // Unknown bom
    }
}

bool utf_convert::to_u32string(const std::string &u8str,
                               std::u32string &   target,
                               UTF_ENDIAN         target_endian,
                               bool               add_bom) {
    target.clear();
    if (add_bom) {
        target.push_back(get_bom(target_endian));
    }

    if (target_endian == UTF_ENDIAN_LITTLE_ENDIAN) {
        return convert_u8str_to_u32str_little_endian(u8str, target);
    } else {
        return convert_u8str_to_u32str_big_endian(u8str, target);
    }
}
