#ifndef QR_TYPES_H
#define QR_TYPES_H

#include <stddef.h>
#include <stdint.h>

typedef enum
{
	QR_EC_LEVEL_L = 0,
	QR_EC_LEVEL_M,
	QR_EC_LEVEL_Q,
	QR_EC_LEVEL_H,
	QR_EC_LEVEL_COUNT
} qr_ec_level;

#define QR_VERSION_COUNT 40

typedef enum
{
	QR_MODE_BYTE,
} qr_encoding_mode;

typedef uint8_t word;

typedef struct
{
	qr_ec_level level;
	qr_encoding_mode mode;
	unsigned version;

	int *matrix;
	size_t side_length;

	unsigned mask;

	size_t codeword_count;
	word *codewords;
} qr_code;

#endif // QR_TYPES_H
