#ifndef UTF_CONVERT_HPP
#define UTF_CONVERT_HPP

#include <string>

namespace utf_convert {
enum UTF_ENDIAN {
    UTF_ENDIAN_LITTLE_ENDIAN,
    UTF_ENDIAN_BIG_ENDIAN,
};

/*!
 * Convert utf-32 string to utf-8 string. The utf-32 string should not contain
 * BOM.
 *
 * @param[in] u32str utf-32 string to be converted.
 * @param u32str_endian Encode endian of the utf-32 string, must be one of
 * UTF_ENDIAN_LITTLE_ENDIAN or UTF_ENDIAN_BIG_ENDIAN.
 * @param[out] target the converted string.
 * @return true if succeeded.
 */
bool to_u8string(const std::u32string &u32str,
                 UTF_ENDIAN            u32str_endian,
                 std::string &         target);

/*!
 * Convert utf-32 string to utf-8 string. The endian is specified with BOM. You
 * must make sure that the utf-32 string contains BOM.
 *
 * @param[in] u32str utf-32 string to be converted. BOM is required.
 * @param[out] target the converted string.
 * @return true if succeeded.
 */
bool to_u8string(const std::u32string &u32str_with_bom, std::string &target);

/*!
 * Convert utf-8 string to utf-32 string.
 *
 * @param[in] u8str utf-8 string to be converted.
 * @param[out] target the converted utf-32 string.
 * @param target_endian endian for the converted utf-32 string.
 * @param add_bom add BOM to the converted utf-32 string if true.
 * @return true if succeeded.
 */
bool to_u32string(const std::string &u8str,
                  std::u32string &   target,
                  UTF_ENDIAN         target_endian,
                  bool               add_bom = false);
}  // namespace utf_convert

#endif  // UTF_CONVERT_HPP
