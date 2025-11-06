#include "qr_enc.h"
#include "qr_types.h"
#include <string.h>
#include <stdint.h>
#include <assert.h>

static const size_t qr_capacity_bytes[QR_EC_LEVEL_COUNT][QR_VERSION_COUNT] = {
    { // L
        17, 32, 53, 78, 106, 134, 154, 192, 230, 271,
        321, 367, 425, 458, 520, 586, 644, 718, 792, 858,
        929, 1003, 1091, 1171, 1273, 1367, 1465, 1528, 1628, 1732,
        1840, 1952, 2068, 2188, 2303, 2431, 2563, 2699, 2809, 2953,
    },
    { // M
        14, 26, 42, 62, 84, 106, 122, 152, 180, 213,
        251, 287, 331, 362, 412, 450, 504, 560, 624, 666,
        711, 779, 857, 911, 997, 1059, 1125, 1190, 1264, 1370,
        1452, 1538, 1628, 1722, 1809, 1911, 1989, 2099, 2213, 2331,
    },
    { // Q
        11, 20, 32, 46, 60, 74, 86, 108, 130, 151,
        177, 203, 241, 258, 292, 322, 364, 394, 442, 482,
        509, 565, 611, 661, 715, 751, 805, 868, 908, 982,
        1030, 1112, 1168, 1228, 1283, 1351, 1423, 1499, 1579, 1663,
    },
    { // H
        7, 14, 24, 34, 44, 58, 64, 84, 98, 119,
        137, 155, 177, 194, 220, 250, 280, 310, 338, 382,
        403, 439, 461, 511, 535, 593, 625, 658, 698, 742,
        790, 842, 898, 958, 983, 1051, 1093, 1139, 1219, 1273,
    }
};

// Get number of bits needed for character count based on version
static size_t get_char_count_bits(unsigned version) {
    if (version <= 9) return 8;    // Versions 1-9: 8 bits
    return 16;                     // Versions 10-40: 16 bits
}

// Add bits to buffer (MSB first)
static void add_bits(uint8_t *buffer, size_t *bit_offset, unsigned value, size_t num_bits) {
    for (size_t i = 0; i < num_bits; i++) {
        size_t byte_pos = *bit_offset / 8;
        size_t bit_pos = 7 - (*bit_offset % 8);
        
        if (value & (1 << (num_bits - 1 - i))) {
            buffer[byte_pos] |= (1 << bit_pos);
        } else {
            buffer[byte_pos] &= ~(1 << bit_pos);
        }
        (*bit_offset)++;
    }
}

unsigned qr_min_version(const char *str, qr_ec_level level) {
    size_t str_len = strlen(str);
    size_t i;

    for (i = 0; i < QR_VERSION_COUNT && str_len > qr_capacity_bytes[level][i]; ++i);

    return (unsigned) i + 1;
}

size_t qr_calculate_encoded_bits(const char *str, unsigned *required_version, qr_ec_level level) {
    size_t str_len = strlen(str);
    *required_version = qr_min_version(str, level);
    size_t char_count_bits = get_char_count_bits(*required_version);
    
    // Mode indicator (4) + char count + data bits + 4 (terminator)
    return 4 + char_count_bits + (str_len * 8) + 4;
}

size_t qr_encode_byte_mode(const char *str, uint8_t *buffer, size_t buffer_size, 
                          unsigned version, qr_ec_level level) {
    if (!str || !buffer || version < 1 || version > QR_VERSION_COUNT) {
        return 0;
    }

    size_t str_len = strlen(str);
    size_t char_count_bits = get_char_count_bits(version);
    size_t total_bits = 4 + char_count_bits + (str_len * 8) + 4; // +4 for terminator
    size_t required_bytes = (total_bits + 7) / 8;
    
    if (buffer_size < required_bytes) {
        return 0; // Buffer too small
    }

    // Clear buffer
    memset(buffer, 0, required_bytes);
    
    size_t bit_offset = 0;
    
    // 1. Mode indicator (0100 for byte mode)
    add_bits(buffer, &bit_offset, 0x4, 4);
    
    // 2. Character count
    add_bits(buffer, &bit_offset, (unsigned)str_len, char_count_bits);
    
    // 3. Data bytes
    for (size_t i = 0; i < str_len; i++) {
        add_bits(buffer, &bit_offset, (unsigned char)str[i], 8);
    }
    
    // 4. Terminator (up to 4 zeros)
    add_bits(buffer, &bit_offset, 0, 4);
    
    // 5. Pad with zeros to make length a multiple of 8
    while (bit_offset % 8 != 0) {
        add_bits(buffer, &bit_offset, 0, 1);
    }
    
    // 6. Add padding bytes (0xEC, 0x11, 0xEC, 0x11, ...) if needed
    // TODO: Implement padding if needed for the specific version/level
    
    return (bit_offset + 7) / 8; // Return number of bytes used
}