#ifndef QR_ENC_H
#define QR_ENC_H

#include <qr/types.h>
#include <stddef.h>
#include <stdint.h>

// Function to get the minimum version needed for a given string length and error correction level
unsigned qr_min_version(const char *str, qr_ec_level level);

// Calculate the number of bits needed for the encoded data
// Returns total number of bits needed (including mode, count, data, and terminator)
// Sets required_version to the minimum version that can hold the data
size_t qr_calculate_encoded_bits(const char *str, unsigned *required_version, qr_ec_level level);

// Encode data into a buffer
// Returns number of bytes written, or 0 on error
// buffer_size must be at least (bits_required + 7) / 8
size_t qr_encode_byte_mode(const char *str, uint8_t *buffer, size_t buffer_size, unsigned version, qr_ec_level level);

// Get the total number of codewords for a given QR code version
// version: QR code version (1-40)
// Returns: Total number of codewords, or 0 if version is invalid
size_t qr_get_total_codewords(unsigned version);

// Get the number of data codewords for a given QR code version
// version: QR code version (1-40)
// Returns: Number of data codewords, or 0 if version is invalid
size_t qr_get_data_codewords(unsigned version, qr_ec_level level);

#endif // QR_ENC_H
