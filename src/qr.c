#include <qr_enc.h>
#include <qr_ecc.h>
#include <qr_matrix.h>
#include <qr_types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void print_usage(const char *program_name)
{
    fprintf(stderr, "Usage: %s <string> [error_correction]\n", program_name);
    fprintf(stderr, "  error_correction: L (7%%), M (15%%), Q (25%%), H (30%%). Default: L\n");
}

static qr_ec_level parse_ec_level(const char *level_str)
{
    if (!level_str) return QR_EC_LEVEL_L;

    switch (level_str[0])
    {
    case 'L': case 'l': return QR_EC_LEVEL_L;
    case 'M': case 'm': return QR_EC_LEVEL_M;
    case 'Q': case 'q': return QR_EC_LEVEL_Q;
    case 'H': case 'h': return QR_EC_LEVEL_H;
    default: return QR_EC_LEVEL_L;
    }
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        print_usage(argv[0]);
        return 1;
    }

    const char *input = argv[1];
    qr_ec_level ec_level = (argc > 2) ? parse_ec_level(argv[2]) : QR_EC_LEVEL_L;

    // Determine minimum version needed
    unsigned version = qr_min_version(input, ec_level);
    if (version > QR_VERSION_COUNT)
    {
        fprintf(stderr, "Error: Input too large for QR code\n");
        return 1;
    }

    // Calculate required buffer size
    unsigned required_version;
    size_t bits_needed = qr_calculate_encoded_bits(input, &required_version, ec_level);
    size_t bytes_needed = (bits_needed + 7) / 8;

    // Allocate buffer for encoded data
    uint8_t *encoded_data = malloc(bytes_needed);
    if (!encoded_data)
    {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return 1;
    }

    // Encode the data
    size_t bytes_used = qr_encode_byte_mode(input, encoded_data, bytes_needed, version, ec_level);
    if (bytes_used == 0)
    {
        fprintf(stderr, "Error: Failed to encode data\n");
        free(encoded_data);
        return 1;
    }

    // Print encoding information
    printf("QR Code Generation:\n");
    printf("  Input: %s\n", input);
    printf("  Error Correction: %s\n", (const char*[]){"L (7%)", "M (15%)", "Q (25%)", "H (30%)"}[ec_level]);
    printf("  Version: %u\n", version);
    printf("  Encoded data size: %zu bytes\n", bytes_used);

    // Add error correction codes
    size_t total_codewords = qr_get_total_codewords(version);
    size_t data_codewords = qr_get_data_codewords(version, ec_level);
    size_t ecc_codewords = total_codewords - data_codewords;

    // Create error correction codec
    qr_ec *ec = qr_ec_create(data_codewords, ecc_codewords);
    if (!ec)
    {
        fprintf(stderr, "Error: Failed to create error correction codec\n");
        free(encoded_data);
        return 1;
    }

    // Allocate buffer for error correction codes
    uint8_t *ecc_data = malloc(ecc_codewords);
    if (!ecc_data)
    {
        fprintf(stderr, "Error: Memory allocation for ECC failed\n");
        qr_ec_destroy(ec);
        free(encoded_data);
        return 1;
    }

    // Encode with error correction
    if (qr_ec_encode(ec, encoded_data, ecc_data) != 0)
    {
        fprintf(stderr, "Error: Error correction encoding failed\n");
        free(ecc_data);
        qr_ec_destroy(ec);
        free(encoded_data);
        return 1;
    }

    printf("  Data codewords: %zu\n", data_codewords);
    printf("  ECC codewords: %zu\n", ecc_codewords);
    printf("  Total codewords: %zu\n", total_codewords);

    // Create QR code matrix
    qr_matrix *matrix = qr_matrix_create(version);
    if (!matrix)
    {
        fprintf(stderr, "Error: Failed to create QR code matrix\n");
        free(ecc_data);
        qr_ec_destroy(ec);
        free(encoded_data);
        return 1;
    }

    // Add basic patterns to the QR code
    qr_add_finder_patterns(matrix);
    qr_add_timing_patterns(matrix);
    qr_add_alignment_patterns(matrix, version);

    // Place the data and ECC codewords
    qr_place_codewords(matrix, encoded_data, data_codewords, ecc_data, ecc_codewords);

    // Apply mask pattern and add format information
    qr_matrix *final_matrix = qr_apply_best_mask(matrix, ec_level);

    // Print the QR code to console
    qr_matrix_print(final_matrix);

    // Cleanup
    if (final_matrix != matrix)
    {
        // Only free the original matrix if it's different from final_matrix
        qr_matrix_free(matrix);
    }
    qr_matrix_free(final_matrix);
    free(ecc_data);
    qr_ec_destroy(ec);
    free(encoded_data);

    return 0;
}
