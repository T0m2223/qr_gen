#ifndef QR_QR_H
#define QR_QR_H

#include <qr/types.h>

qr_code *qr_create(qr_ec_level level, qr_encoding_mode mode, unsigned version);
void qr_encode_bytes(qr_code *qr, const char *message, size_t n);
void qr_destroy(qr_code *qr);

#endif // QR_QR_H
