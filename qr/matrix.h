#ifndef QR_MATRIX_H
#define QR_MATRIX_H

#include <qr/types.h>
#include <stddef.h>
#include <stdio.h>

typedef enum
{
    QR_MODULE_LIGHT = 0,
    QR_MODULE_DARK  = 1,
} qr_module_state;

qr_module_state qr_module_get(const qr_code *qr, size_t i, size_t j);
void qr_module_set(qr_code *qr, size_t i, size_t j, qr_module_state value);
int qr_module_is_reserved(const qr_code *qr, size_t i, size_t j);
void qr_place_codewords(qr_code *qr);
void qr_matrix_print(const qr_code *qr, FILE *stream);

#endif // QR_MATRIX_H
