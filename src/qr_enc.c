#include "qr_enc.h"
#include "qr_types.h"
#include <string.h>
#include <stdint.h>
#include <assert.h>

#define QR_EC_BLOCK_TYPES_PER_VERSION 2
// Total number of codewords for each QR code version (1-40)
static const size_t qr_ec_blocks[QR_EC_LEVEL_COUNT][QR_VERSION_COUNT][QR_EC_BLOCK_TYPES_PER_VERSION] = {
    { // L
        { 1, 0 }, { 1, 0 }, { 1, 0 }, { 1, 0 }, { 1, 0 }, { 2, 0 }, { 2, 0 }, { 2, 0 }, { 2, 0 }, { 2, 2 },
        { 4, 0 }, { 2, 2 }, { 4, 0 }, { 3, 1 }, { 5, 1 }, { 5, 1 }, { 1, 5 }, { 5, 1 }, { 3, 4 }, { 3, 5 },
        { 4, 4 }, { 2, 7 }, { 4, 5 }, { 6, 4 }, { 8, 4 }, { 10, 2 }, { 8, 4 }, { 3, 10 }, { 7, 7 }, { 5, 10 },
        { 13, 3 }, { 17, 0 }, { 17, 1 }, { 13, 6 }, { 12, 7 }, { 6, 14 }, { 17, 4 }, {4, 18 }, { 20, 4 }, { 19, 6 },
    },
    { // M
        { 1, 0 }, { 1, 0 }, { 1, 0 }, { 2, 0 }, { 2, 0 }, { 4, 0 }, { 4, 0 }, { 2, 2 }, { 3, 2 }, { 4, 1 },
        { 1, 4 }, { 6, 2 }, { 8, 1 }, { 4, 5 }, { 5, 5 }, { 7, 3 }, { 10, 1 }, { 9, 4 }, { 3, 11 }, { 3, 13 },
        { 17, 0 }, { 17, 0 }, { 4, 14 }, { 6, 14 }, { 8, 13 }, { 19, 4 }, { 22, 3 }, { 3, 23 }, { 21, 7 }, { 19, 10 },
        { 2, 29 }, { 10, 23 }, { 14, 21 }, { 14, 23 }, { 12, 26 }, { 6, 34 }, { 29, 14 }, { 13, 32 }, { 40, 7 }, { 18, 31 },
    },
    { // Q
        { 1, 0 }, { 1, 0 }, { 2, 0 }, { 2, 0 }, { 2, 2 }, { 4, 0 }, { 2, 4 }, { 4, 2 }, { 4, 4 }, { 6, 2 },
        { 4, 4 }, { 4, 6 }, { 8, 4 }, { 11, 5 }, { 5, 7 }, { 15, 2 }, { 1, 15 }, { 17, 1 }, { 17, 4 }, { 15, 5 },
        { 17, 6 }, { 7, 16 }, { 11, 14 }, { 11, 16 }, { 7, 22 }, { 28, 6 }, { 8, 26 }, { 4, 31 }, { 1, 37 }, { 15, 25 },
        { 42, 1 }, { 10, 35 }, { 29, 19 }, { 44, 7 }, { 39, 14 }, { 46, 10 }, { 49, 10 }, { 48, 14 }, { 43, 22 }, { 43, 43 },
    },
    { // H
        { 1, 0 }, { 1, 0 }, { 2, 0 }, { 4, 0 }, { 2, 2 }, { 4, 0 }, { 4, 1 }, { 4, 2 }, { 4, 4 }, { 6, 2 },
        { 3, 8 }, { 7, 4 }, { 12, 4 }, { 11, 5 }, { 11, 7 }, { 3, 13 }, { 2, 17 }, { 2, 19 }, { 9, 16 }, { 15, 10 },
        { 19, 6 }, { 34, 0 }, { 16, 14 }, { 30, 2 }, { 22, 13 }, { 33, 4 }, { 12, 28 }, { 11, 31 }, { 19, 26 }, { 23, 25 },
        { 23, 28 }, { 19, 35 }, { 11, 46 }, { 59, 1 }, { 22, 41 }, { 2, 64 }, { 24, 46 }, { 42, 32 }, { 10, 67 }, { 20, 61 },
    }
};


static const size_t qr_ec_total_codewords[QR_EC_LEVEL_COUNT][QR_VERSION_COUNT][QR_EC_BLOCK_TYPES_PER_VERSION] = {
    { // L
        { 26, 0 }, { 44, 0 }, { 70, 0 }, { 100, 0 }, { 134, 0 }, { 86, 0 }, { 98, 0 }, { 121, 0 }, { 149, 0 }, { 86, 87 },
        { 101, 0 }, { 116, 117 }, { 133, 0 }, { 145, 146 }, { 109, 110 }, { 122, 123 }, { 135, 136 }, { 150, 151 }, { 141, 142 }, { 135, 136 },
        { 144, 145 }, { 139, 140 }, { 151, 152 }, { 147, 148 }, { 132, 133 }, { 142, 143 }, { 152, 153 }, { 147, 148 }, { 146, 147 }, { 145, 146 },
        { 145, 146 }, { 145, 0 }, { 145, 146 }, { 145, 146 }, { 151, 152 }, { 151, 152 }, { 152, 153 }, {152, 153 }, { 147, 148 }, { 148, 149 },
    },
    { // M
        { 26, 0 }, { 44, 0 }, { 70, 0 }, { 50, 0 }, { 67, 0 }, { 43, 0 }, { 49, 0 }, { 60, 61 }, { 58, 59 }, { 69, 70 },
        { 80, 81 }, { 58, 59 }, { 59, 60 }, { 64, 65 }, { 65, 66 }, { 73, 74 }, { 74, 75 }, { 69, 70 }, { 70, 71 }, { 67, 68 },
        { 68, 0 }, { 74, 0 }, { 75, 76 }, { 73, 74 }, { 75, 76 }, { 74, 75 }, { 73, 74 }, { 73, 74 }, { 73, 74 }, { 75, 76 },
        { 74, 75 }, { 74, 75 }, { 74, 75 }, { 74, 75 }, { 75, 76 }, { 75, 76 }, { 74, 75 }, { 74, 75 }, { 75, 76 }, { 75, 76 },
    },
    { // Q
        { 26, 0 }, { 44, 0 }, { 35, 0 }, { 50, 0 }, { 33, 34 }, { 43, 0 }, { 32, 33 }, { 40, 41 }, { 36, 37 }, { 43, 44 },
        { 50, 51 }, { 46, 47 }, { 44, 45 }, { 36, 37 }, { 54, 55 }, { 43, 44 }, { 5, 51 }, { 50, 51 }, { 47, 48 }, { 54, 55 },
        { 50, 51 }, { 54, 55 }, { 54, 55 }, { 54, 55 }, { 54, 55 }, { 50, 51 }, { 53, 54 }, { 54, 55 }, { 53, 54 }, { 54, 55 },
        { 54, 55 }, { 54, 55 }, { 54, 55 }, { 54, 55 }, { 54, 55 }, { 54, 55 }, { 54, 55 }, { 54, 55 }, { 54, 55 }, { 54, 55 },
    },
    { // H
        { 26, 0 }, { 44, 0 }, { 35, 0 }, { 25, 0 }, { 33, 34 }, { 43, 0 }, { 39, 40 }, { 40, 41 }, { 36, 37 }, { 43, 44 },
        { 36, 37 }, { 42, 43 }, { 33, 34 }, { 36, 37 }, { 36, 37 }, { 45, 46 }, { 42, 43 }, { 42, 43 }, { 39, 40 }, { 43, 44 },
        { 46, 47 }, { 37, 0 }, { 45, 46 }, { 46, 47 }, { 45, 46 }, { 46, 47 }, { 45, 46 }, { 45, 46 }, { 45, 46 }, { 45, 46 },
        { 45, 46 }, { 45, 46 }, { 45, 46 }, { 46, 47 }, { 45, 46 }, { 45, 46 }, { 45, 46 }, { 45, 46 }, { 45, 46 }, { 45, 46 },
    }
}

static const size_t qr_ec_data_codewords[QR_EC_LEVEL_COUNT][QR_VERSION_COUNT][QR_EC_BLOCK_TYPES_PER_VERSION] = {
    { // L
        { 19, 0 }, { 34, 0 }, { 55, 0 }, { 80, 0 }, { 108, 0 }, { 68, 0 }, { 78, 0 }, { 97, 0 }, { 116, 0 }, { 68, 69 },
        { 81, 0 }, { 92, 93 }, { 107, 0 }, { 115, 116 }, { 87, 88 }, { 98, 99 }, { 107, 108 }, { 120, 121 }, { 113, 114 }, { 107, 108 },
        { 116, 117 }, { 111, 112 }, { 121, 122 }, { 117, 118 }, { 106, 107 }, { 114, 115 }, { 122, 123 }, { 117, 118 }, { 116, 117 }, { 115, 116 },
        { 115, 116 }, { 115, 0 }, { 115, 116 }, { 115, 116 }, { 121, 122 }, { 121, 122 }, { 122, 123 }, { 122, 123 }, { 117, 118 }, { 118, 119 },
    },
    { // M
        { 16, 0 }, { 28, 0 }, { 44, 0 }, { 32, 0 }, { 43, 0 }, { 27, 0 }, { 31, 0 }, { 38, 39 }, { 36, 37 }, { 43, 44 },
        { 50, 51 }, { 36, 37 }, { 37, 38 }, { 40, 41 }, { 41, 42 }, { 45, 46 }, { 46, 47 }, { 43, 44 }, { 44, 45 }, { 41, 42 },
        { 42, 0 }, { 46, 0 }, { 47, 48 }, { 45, 46 }, { 47, 48 }, { 46, 47 }, { 45, 46 }, { 45, 46 }, { 45, 46 }, { 47, 48 },
        { 46, 47 }, { 46, 47 }, { 46, 47 }, { 46, 47 }, { 47, 48 }, { 47, 48 }, { 46, 47 }, { 46, 47 }, { 47, 48 }, { 47, 48 },
    },
    { // Q
        { 13, 0 }, { 22, 0 }, { 17, 0 }, { 24, 0 }, { 15, 16 }, { 19, 0 }, { 14, 15 }, { 18, 19 }, { 16, 17 }, { 19, 20 },
        { 22, 23 }, { 20, 21 }, { 20, 21 }, { 16, 17 }, { 24, 25 }, { 19, 20 }, { 22, 23 }, { 22, 23 }, { 21, 22 }, { 24, 25 },
        { 22, 23 }, { 24, 25 }, { 24, 25 }, { 24, 25 }, { 24, 25 }, { 22, 23 }, { 23, 24 }, { 24, 25 }, { 23, 24 }, { 24, 25 },
        { 24, 25 }, { 24, 25 }, { 24, 25 }, { 24, 25 }, { 24, 25 }, { 24, 25 }, { 24, 25 }, { 24, 25 }, { 24, 25 }, { 24, 25 },
    },
    { // H
        { 9, 0 }, { 16, 0 }, { 13, 0 }, { 9, 0 }, { 11, 12 }, { 15, 0 }, { 13, 14 }, { 14, 15 }, { 12, 13 }, { 15, 16 },
        { 12, 13 }, { 14, 15 }, { 11, 12 }, { 12, 13 }, { 12, 13 }, { 15, 16 }, { 14, 15 }, { 14, 15 }, { 13, 14 }, { 15, 16 },
        { 16, 17 }, { 13, 0 }, { 15, 16 }, { 16, 17 }, { 15, 16 }, { 16, 17 }, { 15, 16 }, { 15, 16 }, { 15, 16 }, { 15, 16 },
        { 15, 16 }, { 15, 16 }, { 15, 16 }, { 16, 17 }, { 15, 16 }, { 15, 16 }, { 15, 16 }, { 15, 16 }, { 15, 16 }, { 15, 16 },
    }
};
    

// TODO: test for manual errors in tables
// - qr_ec_blocks 0 => 0 in qr_ec_total_codewords and qr_ec_data_codewords
// - qr_ec_blocks * qr_ec_total_codewords = qr_capacity_bytes


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

size_t qr_get_total_codewords(unsigned version) {
    if (version < 1 || version > QR_VERSION_COUNT) {
        return 0; // Invalid version
    }
    return qr_total_codewords[version - 1]; // Convert to 0-based index
}