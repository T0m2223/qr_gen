#ifndef QR_PATTERNS_H
#define QR_PATTERNS_H

#include <qr/types.h>
#include <stddef.h>

void qr_finder_patterns_apply(qr_code *qr);
void qr_separators_apply(qr_code *qr);
void qr_timing_patterns_apply(qr_code *qr);
void qr_alignment_patterns_apply(qr_code *qr);

int qr_is_in_alignment_patterns(const qr_code *qr, size_t i, size_t j);

#endif // QR_PATTERNS_H
