#include <qr/ecc.h>
#include <qr/enc.h>
#include <qr/info.h>
#include <qr/mask.h>
#include <qr/matrix.h>
#include <qr/patterns.h>
#include <qr/qr.h>
#include <qr/types.h>
#include <stdio.h>
#include <stdlib.h>

static const size_t CODEWORD_COUNT[QR_VERSION_COUNT] =
{
      26,   44,   70,  100,  134,  172,  196,  242,  292,  346,
     404,  466,  532,  581,  655,  733,  815,  901,  991, 1085,
    1156, 1258, 1364, 1474, 1588, 1706, 1828, 1921, 2051, 2185,
    2323, 2465, 2611, 2761, 2876, 3034, 3196, 3362, 3532, 3706,
};

qr_code *
qr_create(qr_ec_level level, qr_encoding_mode mode, unsigned version)
{
    qr_code *qr = malloc(sizeof(qr_code));

    qr->level = level;
    qr->mode = mode;
    qr->version = version;
    qr->side_length = 21 + (qr->version * 4);
    qr->matrix = malloc((qr->side_length * qr->side_length) * sizeof(*qr->matrix));

    qr->codeword_count = CODEWORD_COUNT[qr->version];
    qr->codewords = malloc(qr->codeword_count * sizeof(word));

    return qr;
}

void
qr_destroy(qr_code *qr)
{
    free(qr->codewords);
    free(qr->matrix);
    free(qr);
}

void
qr_encode_message(qr_code *qr, const char *message)
{
    // 1. enc
    printf("Encoding message............"); fflush(stdout);
    qr_encode_data(qr, message);
    printf("OK\n");

    // 2. ecc
    printf("Encoding error correction..."); fflush(stdout);
    qr_ec_encode(qr);
    printf("OK\n");

    // 3. block
    printf("Interleaving codewords......"); fflush(stdout);
    qr_interleave_codewords(qr);
    printf("OK\n");

    // 4. matrix
    printf("Generating matrix..........."); fflush(stdout);
    qr_place_codewords(qr);
    qr_finder_patterns_apply(qr);
    qr_separators_apply(qr);
    qr_timing_patterns_apply(qr);
    qr_alignment_patterns_apply(qr);
    printf("OK\n");

    // 5. masking
    printf("Masking....................."); fflush(stdout);
    qr_mask_apply(qr);
    printf("OK\n");

    // 6. info
    printf("Applying meta information..."); fflush(stdout);
    qr_format_info_apply(qr);
    qr_version_info_apply(qr);
    printf("OK\n");
}
