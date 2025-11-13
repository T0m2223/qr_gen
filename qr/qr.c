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

extern void log_(const char *fmt, ...);

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
    log_("Encoding message............");
    qr_encode_data(qr, message);
    log_("OK\n");

    // 2. ecc
    log_("Encoding error correction...");
    qr_ec_encode(qr);
    log_("OK\n");

    // 3. block
    log_("Interleaving codewords......");
    qr_interleave_codewords(qr);
    log_("OK\n");

    // 4. matrix
    log_("Generating matrix...........");
    qr_place_codewords(qr);
    qr_finder_patterns_apply(qr);
    qr_separators_apply(qr);
    qr_timing_patterns_apply(qr);
    qr_alignment_patterns_apply(qr);
    log_("OK\n");

    // 5. masking
    log_("Masking.....................");
    qr_mask_apply(qr);
    log_("OK\n");

    // 6. info
    log_("Applying meta information...");
    qr_format_info_apply(qr);
    qr_version_info_apply(qr);
    log_("OK\n");
}

void
qr_svg_print(qr_code *qr, FILE *stream)
{
    size_t i, j;
    char *color;
    char *fmt_str =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" "
        "width=\"%zu\" height=\"%zu\" viewBox=\"0 0 %zu %zu\" "
        "shape-rendering=\"crispEdges\">\n";

    fprintf(stream, fmt_str, qr->side_length, qr->side_length, qr->side_length, qr->side_length);
    fprintf(stream, "<rect width=\"100%%\" height=\"100%%\" fill=\"white\"/>\n");

    for (i = 0; i < qr->side_length; ++i)
    {
        for (j = 0; j < qr->side_length; ++j)
        {
            color = qr_module_get(qr, i, j) ? "black" : "white";
            fprintf(stream, "<rect x=\"%zu\" y=\"%zu\" width=\"%d\" height=\"%d\" fill=\"%s\"/>\n", j, i, 1, 1, color);
        }
    }

    fprintf(stream, "</svg>\n");
}
