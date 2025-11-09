#include <qr/info.h>
#include <qr/mask.h>
#include <qr/matrix.h>
#include <qr/patterns.h>
#include <qr/qr.h>
#include <assert.h>
#include <stdlib.h>

qr_code *
qr_create(qr_ec_level level, qr_encoding_mode mode, unsigned version)
{
    qr_code *qr = malloc(sizeof(qr_code));

    qr->level = level;
    qr->mode = mode;
    qr->version = version;
    qr->side_length = 21 + (qr->version * 4);
    qr->matrix = malloc((qr->side_length * qr->side_length) * sizeof(*qr->matrix));

    // TODO
    qr->n_data_codewords = 3;
    qr->n_ec_codewords = 3;
    //

    qr->data_codewords = malloc(qr->n_data_codewords * sizeof(*qr->data_codewords));
    qr->ec_codewords = malloc(qr->n_ec_codewords * sizeof(*qr->ec_codewords));

    return qr;
}

void
qr_destroy(qr_code *qr)
{
    free(qr->ec_codewords);
    free(qr->data_codewords);
    free(qr->matrix);
    free(qr);
}

void
qr_encode_bytes(qr_code *qr, const char *message, size_t n)
{
    assert(n <= qr->n_data_codewords && "Message provided is too large");

    size_t i;
    for (i = 0; i < n; ++i)
        qr->data_codewords[i] = message[i];
    for (; i < qr->n_data_codewords; ++i)
        qr->data_codewords[i] = 0;

    // 1. enc

    // 2. ecc

    // 3. block

    // 4. matrix
    qr_finder_patterns_apply(qr);
    qr_separators_apply(qr);
    qr_timing_patterns_apply(qr);
    qr_alignment_patterns_apply(qr);
    qr_place_codewords(qr);

    // 5. masking
    // version info necessary for mask evaluation
    qr_version_info_apply(qr);
    qr_mask_apply(qr);

    // 6. info
    qr_format_info_apply(qr);
    qr_version_info_apply(qr);
}
