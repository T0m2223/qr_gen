#ifndef QR_ENC_H
#define QR_ENC_H

#include <qr/types.h>
#include <stddef.h>
#include <stdint.h>

unsigned qr_min_version(size_t bytes, qr_ec_level level);
void qr_encode_data(qr_code *qr, const char *message);

#endif // QR_ENC_H
