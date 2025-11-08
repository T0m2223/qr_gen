#ifndef QR_ECC_H
#define QR_ECC_H

#include <stdint.h>
#include <stddef.h>

typedef struct
{
    uint8_t *data;
    uint8_t *ecc;
    size_t data_length;
    size_t ecc_length;
    uint8_t *generator;
} qr_ec;

qr_ec *qr_ec_create(size_t data_length, size_t ecc_length);
void qr_ec_destroy(qr_ec *ec);
int qr_ec_encode(qr_ec *ec, const uint8_t *data, uint8_t *ecc);

#endif // QR_ECC_H
