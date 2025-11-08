#ifndef QR_MASK
#define QR_MASK

#include <qr_types.h>

#define QR_MASK_PATTERN_COUNT 8

int qr_mask_evaluate(const qr_code *qr);
void qr_mask_apply_pattern(qr_code *qr, size_t mask_pattern);
void qr_mask_apply(qr_code *qr);

#endif // QR_MASK
