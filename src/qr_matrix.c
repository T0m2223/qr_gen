#include "qr_matrix.h"
#include "qr_types.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Module to handle QR code matrix generation

// Module-level constants
#define MODULE_EMPTY 0x00
#define MODULE_DARK 0x01
#define MODULE_LIGHT 0x00

// Get the size of the QR code matrix for a given version
static size_t get_matrix_size(size_t version)
{
    return 21 + (version - 1) * 4;
}


qr_module_state
qr_module_get(const qr_code *qr, size_t i, size_t j)
{
    return qr->data[i * qr->side_length + j] ? QR_MODULE_DARK : QR_MODULE_LIGHT;
}

void
qr_module_set(qr_code *qr, size_t i, size_t j, qr_module_state value)
{
    qr->data[i * qr->side_length + j] = value;
}


// Add a finder pattern at the specified position
static void add_finder_pattern(qr_matrix *matrix, size_t x, size_t y)
{
    // Outer black square (7x7)
    for (size_t dy = 0; dy < 7; dy++)
    {
        for (size_t dx = 0; dx < 7; dx++)
        {
            set_module(matrix, x + dx, y + dy, MODULE_DARK);
        }
    }

    // Inner white square (5x5)
    for (size_t dy = 1; dy < 6; dy++)
    {
        for (size_t dx = 1; dx < 6; dx++)
        {
            set_module(matrix, x + dx, y + dy, MODULE_LIGHT);
        }
    }

    // Center black square (3x3)
    for (size_t dy = 2; dy < 5; dy++)
    {
        for (size_t dx = 2; dx < 5; dx++)
        {
            set_module(matrix, x + dx, y + dy, MODULE_DARK);
        }
    }
}

// Add all three finder patterns to the QR code
void qr_add_finder_patterns(qr_matrix *matrix)
{
    if (!matrix) return;

    // Top-left finder pattern (at 0,0)
    add_finder_pattern(matrix, 0, 0);

    // Top-right finder pattern
    add_finder_pattern(matrix, matrix->size - 7, 0);

    // Bottom-left finder pattern
    add_finder_pattern(matrix, 0, matrix->size - 7);
}

// Add timing patterns to the QR code
void qr_add_timing_patterns(qr_matrix *matrix)
{
    if (!matrix) return;

    size_t size = matrix->size;

    // Horizontal timing pattern (row 6, starting from column 8 to size-9)
    for (size_t x = 8; x < size - 8; x++)
    {
        set_module(matrix, x, 6, (x % 2) ? MODULE_LIGHT : MODULE_DARK);
    }

    // Vertical timing pattern (column 6, starting from row 8 to size-9)
    for (size_t y = 8; y < size - 8; y++)
    {
        set_module(matrix, 6, y, (y % 2) ? MODULE_LIGHT : MODULE_DARK);
    }
}

// Print the QR code to console (for debugging)
void qr_matrix_print(const qr_matrix *matrix)
{
    if (!matrix || !matrix->data) return;

    printf("QR Code (%zux%zu):\n", matrix->size, matrix->size);

    for (size_t y = 0; y < matrix->size; y++)
    {
        for (size_t x = 0; x < matrix->size; x++)
        {
            printf("%s", get_module(matrix, x, y) ? "  " : "\x1b[7m  \x1b[27m");
        }
        printf("\n");
    }
}

// Add a single alignment pattern at the specified position
static void add_alignment_pattern(qr_matrix *matrix, size_t x, size_t y)
{
    // Skip if position would overlap with finder patterns
    if ((x < 8 && y < 8) ||                     // Top-left finder
            (x < 8 && y > matrix->size - 9) ||      // Bottom-left finder
            (x > matrix->size - 9 && y < 8))
    {      // Top-right finder
        return;
    }

    // Outer black square (5x5)
    for (size_t dy = 0; dy < 5; dy++)
    {
        for (size_t dx = 0; dx < 5; dx++)
        {
            set_module(matrix, x - 2 + dx, y - 2 + dy, MODULE_DARK);
        }
    }

    // Inner white square (3x3)
    for (size_t dy = 1; dy < 4; dy++)
    {
        for (size_t dx = 1; dx < 4; dx++)
        {
            set_module(matrix, x - 2 + dx, y - 2 + dy, MODULE_LIGHT);
        }
    }

    // Center black dot
    set_module(matrix, x, y, MODULE_DARK);
}

// Get the coordinates of alignment patterns for a given version
static void get_alignment_positions(size_t version, size_t positions[], size_t *count)
{
    if (version < 2)
    {
        *count = 0;
        return;
    }

    // Alignment pattern centers for each version (from QR Code spec)
    static const size_t alignment_positions[40][7] = {
        {0}, {0}, {6, 18, 0}, {6, 22, 0}, {6, 26, 0}, {6, 30, 0},
        {6, 34, 0}, {6, 22, 38, 0}, {6, 24, 42, 0}, {6, 26, 46, 0},
        {6, 28, 50, 0}, {6, 30, 54, 0}, {6, 32, 58, 0}, {6, 34, 62, 0},
        {6, 26, 46, 66, 0}, {6, 26, 48, 70, 0}, {6, 26, 50, 74, 0},
        {6, 30, 54, 78, 0}, {6, 30, 56, 82, 0}, {6, 30, 58, 86, 0},
        {6, 34, 62, 90, 0}, {6, 28, 50, 72, 94, 0}, {6, 26, 50, 74, 98, 0},
        {6, 30, 54, 78, 102, 0}, {6, 28, 54, 80, 106, 0}, {6, 32, 58, 84, 110, 0},
        {6, 30, 58, 86, 114, 0}, {6, 34, 62, 90, 118, 0}, {6, 26, 50, 74, 98, 122, 0},
        {6, 30, 54, 78, 102, 126, 0}, {6, 26, 52, 78, 104, 130, 0},
        {6, 30, 56, 82, 108, 134, 0}, {6, 34, 60, 86, 112, 138, 0},
        {6, 30, 58, 86, 114, 142, 0}, {6, 34, 62, 90, 118, 146, 0},
        {6, 30, 54, 78, 102, 126, 150}, {6, 24, 50, 76, 102, 128, 154},
        {6, 28, 54, 80, 106, 132, 158}, {6, 32, 58, 84, 110, 136, 162}
    };

    size_t i = 0;
    while (i < 7 && alignment_positions[version - 1][i] != 0)
    {
        positions[i] = alignment_positions[version - 1][i];
        i++;
    }
    *count = i;
}

// Add all alignment patterns for the given QR code version
void qr_add_alignment_patterns(qr_matrix *matrix, size_t version)
{
    if (!matrix || version < 2) return;

    size_t positions[7];
    size_t count;
    get_alignment_positions(version, positions, &count);

    // Add alignment patterns at all intersections of the alignment pattern positions
    for (size_t i = 0; i < count; i++)
    {
        for (size_t j = 0; j < count; j++)
        {
            // Skip positions that would overlap with finder patterns
            if ((i == 0 && j == 0) ||                       // Top-left finder
                    (i == 0 && j == count - 1) ||               // Bottom-left finder
                    (i == count - 1 && j == 0))
            {               // Top-right finder
                continue;
            }
            add_alignment_pattern(matrix, positions[i], positions[j]);
        }
    }
}

// Format information bits for each error correction level and mask pattern
// [ec_level][mask_pattern] = 15-bit format information
static const uint16_t FORMAT_INFO_TABLE[4][8] = {
    // L (00) mask 0-7
    { 0x77c4, 0x72f3, 0x7daa, 0x789d, 0x662f, 0x6318, 0x6c41, 0x6976 },
    // M (01) mask 0-7
    { 0x5412, 0x5125, 0x5e7c, 0x5b4b, 0x45f9, 0x40ce, 0x4f97, 0x4aa0 },
    // Q (10) mask 0-7
    { 0x355f, 0x3068, 0x3f31, 0x3a06, 0x24b4, 0x2183, 0x2eda, 0x2bed },
    // H (11) mask 0-7
    { 0x1689, 0x13be, 0x1ce7, 0x19d0, 0x0762, 0x0255, 0x0d0c, 0x083b }
};

// Add format information to the QR code
void qr_add_format_info(qr_matrix *matrix, qr_ec_level level, uint8_t mask_pattern)
{
    if (!matrix || level >= 4 || mask_pattern >= 8) return;

    // Get the 15-bit format information
    uint16_t format_info = FORMAT_INFO_TABLE[level][mask_pattern];

    // Add format information around the finder patterns
    // Top-left (around top-left finder)
    for (int i = 0; i < 8; i++)
    {
        // Horizontal (top)
        if (i != 6)
        {  // Skip timing pattern
            set_module(matrix, 8, i, (format_info >> (14 - i)) & 1);
        }

        // Vertical (left)
        if (i != 8)
        {  // Skip timing pattern
            set_module(matrix, i, 8, (format_info >> (14 - i)) & 1);
        }
    }

    // Bottom-left (vertical)
    for (int i = 0; i < 7; i++)
    {
        set_module(matrix, 8, matrix->size - 1 - i, (format_info >> i) & 1);
    }

    // Top-right (horizontal)
    for (int i = 0; i < 8; i++)
    {
        set_module(matrix, matrix->size - 8 + i, 8, (format_info >> (6 - i)) & 1);
    }

    // Dark module (always dark)
    set_module(matrix, 8, matrix->size - 8, MODULE_DARK);
}

// Check if a module is part of an alignment pattern
static int is_in_alignment_patterns(const qr_matrix *matrix, size_t x, size_t y)
{
    // For version 1, there are no alignment patterns
    if (matrix->size <= 21) return 0;

    // Get alignment pattern positions for this version
    size_t version = (matrix->size - 21) / 4 + 1;
    size_t positions[7];
    size_t count;
    get_alignment_positions(version, positions, &count);

    // Check if (x,y) is within any alignment pattern
    for (size_t i = 0; i < count; i++)
    {
        for (size_t j = 0; j < count; j++)
        {
            // Skip positions that would overlap with finder patterns
            if ((i == 0 && j == 0) ||               // Top-left finder
                    (i == 0 && j == count - 1) ||        // Bottom-left finder
                    (i == count - 1 && j == 0))
            {        // Top-right finder
                continue;
            }

            // Check if (x,y) is within 2 modules of an alignment center
            size_t ax = positions[i];
            size_t ay = positions[j];
            if (x >= ax - 2 && x <= ax + 2 && y >= ay - 2 && y <= ay + 2)
            {
                return 1;
            }
        }
    }

    return 0;
}

int qr_module_is_reserved(const qr_matrix *matrix, size_t i, size_t j)
{
    // finder pattern (7) + separator (1)
    int in_finder_upper_left = i < 8 && j < 8;
    int in_finder_upper_right = i < 8 && j >= matrix->size - 8;
    int in_finder_lower_left = i >= matrix->size - 8 && j < 8;
    int in_finder = in_finder_upper_left || in_finder_upper_right || in_finder_lower_left;

    int in_timing = i == 6 || j == 6;
    int in_alignment = is_in_alignment_patterns(matrix, i, j);

    int in_version_lower_left = i < 6 && j >= matrix->size - 11;
    int in_version_upper_right = i >= matrix->size - 11 && j < 6;
    int in_version = matrix->version >= 7 && (in_version_lower_left || in_version_upper_right);

    int in_format_upper_left = i < 9 && j < 9;
    int in_format_upper_right = i < 9 && j >= matrix->size - 8;
    int in_format_lower_left = i >= matrix->size - 8 && j < 9;
    int in_format = in_format_upper_left || in_format_upper_right || in_format_lower_left;

    return in_finder || in_timing || in_alignment || in_version || in_format;
}

// Place a single bit in the matrix, skipping reserved areas
static void place_bit(qr_matrix *matrix, size_t *x, size_t *y, int *up, uint8_t bit)
{
    size_t size = matrix->size;

    while (1)
    {
        // Skip if this is a reserved module
        if (!is_reserved_module(matrix, *x, *y))
        {
            set_module(matrix, *x, *y, bit);

            // Move to the next position after placing the bit
            if (*up)
            {
                if (*x == 0 || *y == size - 1)
                {
                    (*y)--;
                    *up = 0;
                    if (*y == 6) (*y)--;  // Skip timing pattern
                } else {
                    (*x)--;
                    (*y)++;
                }
            } else {
                if (*x == size - 1 || *y == 0)
                {
                    (*y)--;
                    *up = 1;
                    if (*y == 6) (*y)--;  // Skip timing pattern
                } else {
                    (*x)++;
                    (*y)--;
                }
            }
            break;
        }

        // Move to the next position if current is reserved
        if (*up)
        {
            if (*x == 0 || *y == size - 1)
            {
                (*y)--;
                *up = 0;
                if (*y == 6) (*y)--;  // Skip timing pattern
            } else {
                (*x)--;
                (*y)++;
            }
        } else {
            if (*x == size - 1 || *y == 0)
            {
                (*y)--;
                *up = 1;
                if (*y == 6) (*y)--;  // Skip timing pattern
            } else {
                (*x)++;
                (*y)--;
            }
        }
    }
}

// Place codewords in the QR code matrix
void qr_place_codewords(qr_matrix *matrix, const uint8_t *data, size_t data_len, const uint8_t *ecc, size_t ecc_len)
{
    if (!matrix || !data || !ecc) return;

    size_t x = matrix->size - 1;
    size_t y = matrix->size - 1;
    int up = 1;  // Direction: 1 = up-right, 0 = down-left

    // Place data codewords
    for (size_t i = 0; i < data_len; i++)
    {
        uint8_t byte = data[i];
        for (int j = 7; j >= 0; j--)
        {
            place_bit(matrix, &x, &y, &up, (byte >> j) & 1);
        }
    }

    // Place ECC codewords
    for (size_t i = 0; i < ecc_len; i++)
    {
        uint8_t byte = ecc[i];
        for (int j = 7; j >= 0; j--)
        {
            place_bit(matrix, &x, &y, &up, (byte >> j) & 1);
        }
    }
}

