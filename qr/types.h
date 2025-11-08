#ifndef QR_TYPES_H
#define QR_TYPES_H

#include <stddef.h>

// Error correction levels
typedef enum
{
    QR_EC_LEVEL_L = 0,
    QR_EC_LEVEL_M,
    QR_EC_LEVEL_Q,
    QR_EC_LEVEL_H,
    QR_EC_LEVEL_COUNT
} qr_ec_level;

#define QR_VERSION_COUNT 40

// Data encoding modes
typedef enum
{
    QR_MODE_BYTE
} qr_encoding_mode;

typedef struct
{
    qr_ec_level ec_level;
    qr_encoding_mode encoding_mode;
    unsigned version;
    size_t mask;

    int *data;  // TODO: rename to matrix
    size_t side_length;
} qr_code;

#endif // QR_TYPES_H
