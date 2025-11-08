#ifndef QR_MATRIX_H
#define QR_MATRIX_H

#include <qr/types.h>
#include <stdint.h>
#include <stddef.h>

typedef enum
{
    QR_MODULE_LIGHT = 0,
    QR_MODULE_DARK = 1,
} qr_module_state;

void qr_add_finder_patterns(qr_code *qr);
void qr_add_alignment_patterns(qr_code *qr, size_t version);
void qr_add_timing_patterns(qr_code *qr);
void qr_add_format_info(qr_code *qr, qr_ec_level level, uint8_t mask_pattern);
void qr_place_codewords(qr_code *qr, const uint8_t *data, size_t data_len, const uint8_t *ecc, size_t ecc_len);
qr_code *qr_apply_best_mask(qr_code *qr, qr_ec_level level);
void qr_matrix_print(const qr_code *qr);
int qr_module_is_reserved(const qr_code *qr, size_t i, size_t j);
void qr_module_set(qr_code *qr, size_t i, size_t j, qr_module_state value);
qr_module_state qr_module_get(const qr_code *qr, size_t i, size_t j);

#endif // QR_MATRIX_H
