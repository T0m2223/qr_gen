#include <assert.h>
#include <qr/matrix.h>
#include <qr/patterns.h>
#include <stdio.h>

qr_module_state
qr_module_get(const qr_code *qr, size_t i, size_t j)
{
    return qr->matrix[i * qr->side_length + j] ? QR_MODULE_DARK : QR_MODULE_LIGHT;
}

void
qr_module_set(qr_code *qr, size_t i, size_t j, qr_module_state value)
{
    qr->matrix[i * qr->side_length + j] = value;
}

void
qr_matrix_print(const qr_code *qr, FILE *stream)
{
    size_t i, j;

    // quiet zone
    for (i = 0; i < 4; ++i)
    {
        for (j = 0; j < qr->side_length + 8; ++j)
            fprintf(stream, "\x1b[7m  \x1b[27m");
        fprintf(stream, "\n");
    }

    for (i = 0; i < qr->side_length; ++i)
    {
        // quiet zone
        fprintf(stream, "\x1b[7m        \x1b[27m");

        for (j = 0; j < qr->side_length; ++j)
            fprintf(stream, "%s", qr_module_get(qr, i, j) ? "  " : "\x1b[7m  \x1b[27m");

        // quiet zone
        fprintf(stream, "\x1b[7m        \x1b[27m");

        fprintf(stream, "\n");
    }

    // quiet zone
    for (i = 0; i < 4; ++i)
    {
        for (j = 0; j < qr->side_length + 8; ++j)
            fprintf(stream, "\x1b[7m  \x1b[27m");
        fprintf(stream, "\n");
    }
}

int
qr_module_is_reserved(const qr_code *qr, size_t i, size_t j)
{
    // finder pattern (7) + separator (1)
    int in_finder_upper_left = i < 8 && j < 8;
    int in_finder_upper_right = i < 8 && j >= qr->side_length - 8;
    int in_finder_lower_left = i >= qr->side_length - 8 && j < 8;
    int in_finder = in_finder_upper_left || in_finder_upper_right || in_finder_lower_left;

    int in_timing = i == 6 || j == 6;
    int in_alignment = qr_is_in_alignment_patterns(qr, i, j);

    int in_version_lower_left = i < 6 && j >= qr->side_length - 11;
    int in_version_upper_right = i >= qr->side_length - 11 && j < 6;
    int in_version = qr->version + 1 >= 7 && (in_version_lower_left || in_version_upper_right);

    int in_format_upper_left = i < 9 && j < 9;
    int in_format_upper_right = i < 9 && j >= qr->side_length - 8;
    int in_format_lower_left = i >= qr->side_length - 8 && j < 9;
    int in_format = in_format_upper_left || in_format_upper_right || in_format_lower_left;

    return in_finder || in_timing || in_alignment || in_version || in_format;
}

static void
place_bit(qr_code *qr, size_t *i, size_t *j, int *left, int *up, qr_module_state value)
{
    int exit = 0;

    while (!exit)
    {
        if (!qr_module_is_reserved(qr, *i, *j))
        {
            qr_module_set(qr, *i, *j, value);
            exit = 1;
        }

        if (!*left)
        {
            if ((*up && *i == 0) || (!*up && *i == qr->side_length - 1))
            {
                *up ^= 1;
                *j -= 2;
            }
            else
            {
                *i += *up ? -1 : 1;
            }
            ++*j;
        }
        else
        {
          --*j;
        }

        *left ^= 1;

        // skip vertical timing pattern
        if (*j == 6) --*j;
    }
}

static const size_t REMAINDER_BITS[QR_VERSION_COUNT] =
{
    0, 7, 7, 7, 7, 7, 0, 0, 0, 0,
    0, 0, 0, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 3, 3, 3,
    3, 3, 3, 3, 0, 0, 0, 0, 0, 0,
};

void
qr_place_codewords(qr_code *qr)
{
    size_t word, bit;

    size_t i, j;
    int left = 1, up = 1;
    i = j = qr->side_length - 1;

    for (word = 0; word < qr->codeword_count; ++word)
    {
        for (bit = 7; bit < 8; --bit)
            place_bit(qr, &i, &j, &left, &up, (qr->codewords[word] >> bit) & 1);
    }

    for (bit = 0; bit < REMAINDER_BITS[qr->version]; ++bit)
        place_bit(qr, &i, &j, &left, &up, 0);

    assert(i == qr->side_length - (qr->version + 1 >= 7 ? 11 : 8) && j == 1 && "Codewords do not fill symbol completely");
}
