#include <qr/info.h>
#include <qr/mask.h>
#include <qr/matrix.h>

static int mask_pattern_0(size_t i, size_t j) { return (i + j) % 2 == 0; }
static int mask_pattern_1(size_t i, size_t j) { (void) j; return i % 2 == 0; }
static int mask_pattern_2(size_t i, size_t j) { (void) i; return j % 3 == 0; }
static int mask_pattern_3(size_t i, size_t j) { return (i + j) % 3 == 0; }
static int mask_pattern_4(size_t i, size_t j) { return ((i / 2) + (j / 3)) % 2 == 0; }
static int mask_pattern_5(size_t i, size_t j) { return ((i * j) % 2) + ((i * j) % 3) == 0; }
static int mask_pattern_6(size_t i, size_t j) { return (((i * j) % 2) + ((i * j) % 3)) % 2 == 0; }
static int mask_pattern_7(size_t i, size_t j) { return (((i + j) % 2) + ((i * j) % 3)) % 2 == 0; }

static int (* const MASK_PREDICATES[QR_MASK_PATTERN_COUNT])(size_t i, size_t j) =
{
    mask_pattern_0, mask_pattern_1, mask_pattern_2, mask_pattern_3,
    mask_pattern_4, mask_pattern_5, mask_pattern_6, mask_pattern_7,
};

static const int N[4] = { 3, 3, 40, 10 };

static int
feature_1_evaluation(const qr_code *qr)
{
    // adjacent modules in row in same color
    int points = 0;
    size_t i, j, run_row, run_column;
    qr_module_state color_row, color_column;

    for (i = 0; i < qr->side_length; ++i)
    {
        run_row = run_column = 0;
        for (j = 0; j < qr->side_length; ++j)
        {
            if (qr_module_get(qr, i, j) != color_row)
            {
                color_row = qr_module_get(qr, i, j);
                if (run_row >= 5)
                    points += N[0] + run_row - 5;
                run_row = 0;
            }
            if (qr_module_get(qr, j, i) != color_column)
            {
                color_column = qr_module_get(qr, j, i);
                if (run_column >= 5)
                    points += N[0] + run_column - 5;
                run_column = 0;
            }

            ++run_row;
            ++run_column;
        }
    }

    return points;
}

static int
feature_2_evaluation(const qr_code *qr)
{
    // block of modules in same color
    int points = 0;
    size_t i, j;
    qr_module_state m[4];

    for (i = 0; i < qr->side_length - 1; ++i)
    {
        for (j = 0; j < qr->side_length - 1; ++j)
        {
            m[0] = qr_module_get(qr, i, j);
            m[1] = qr_module_get(qr, i, j + 1);
            m[2] = qr_module_get(qr, i + 1, j);
            m[3] = qr_module_get(qr, i + 1, j + 1);

            if (m[0] == m[1] && m[1] == m[2] && m[2] == m[3])
                points += N[1];
        }
    }

    return points;
}

static int
feature_3_evaluation(const qr_code *qr)
{
    // 1:1:3:1:1 ratio (dark:light:dark:light:dark) pattern in row/column, preceded or followed by light area 4 modules wide
    int points = 0;
    size_t i, j;
    int pattern_row, pattern_column;
    int preceded_row, preceded_column;
    int followed_row, followed_column;

    for (i = 0; i < qr->side_length; ++i)
    {
        for (j = 0; j < qr->side_length - 6; ++j)
        {
            pattern_row =
                qr_module_get(qr, i, j + 0) == QR_MODULE_DARK &&
                qr_module_get(qr, i, j + 1) == QR_MODULE_LIGHT &&
                qr_module_get(qr, i, j + 2) == QR_MODULE_DARK &&
                qr_module_get(qr, i, j + 3) == QR_MODULE_DARK &&
                qr_module_get(qr, i, j + 4) == QR_MODULE_DARK &&
                qr_module_get(qr, i, j + 5) == QR_MODULE_LIGHT &&
                qr_module_get(qr, i, j + 6) == QR_MODULE_DARK;

            preceded_row = j >= 4 &&
                qr_module_get(qr, i, j - 0) == QR_MODULE_LIGHT &&
                qr_module_get(qr, i, j - 1) == QR_MODULE_LIGHT &&
                qr_module_get(qr, i, j - 2) == QR_MODULE_LIGHT &&
                qr_module_get(qr, i, j - 3) == QR_MODULE_LIGHT;

            followed_row = j < qr->side_length - 10 &&
                qr_module_get(qr, i, j + 7) == QR_MODULE_LIGHT &&
                qr_module_get(qr, i, j + 8) == QR_MODULE_LIGHT &&
                qr_module_get(qr, i, j + 9) == QR_MODULE_LIGHT &&
                qr_module_get(qr, i, j + 10) == QR_MODULE_LIGHT;


            pattern_column =
                qr_module_get(qr, j + 0, i) == QR_MODULE_DARK &&
                qr_module_get(qr, j + 1, i) == QR_MODULE_LIGHT &&
                qr_module_get(qr, j + 2, i) == QR_MODULE_DARK &&
                qr_module_get(qr, j + 3, i) == QR_MODULE_DARK &&
                qr_module_get(qr, j + 4, i) == QR_MODULE_DARK &&
                qr_module_get(qr, j + 5, i) == QR_MODULE_LIGHT &&
                qr_module_get(qr, j + 6, i) == QR_MODULE_DARK;

            preceded_row = j >= 4 &&
                qr_module_get(qr, j - 0, i) == QR_MODULE_LIGHT &&
                qr_module_get(qr, j - 1, i) == QR_MODULE_LIGHT &&
                qr_module_get(qr, j - 2, i) == QR_MODULE_LIGHT &&
                qr_module_get(qr, j - 3, i) == QR_MODULE_LIGHT;

            followed_row = j < qr->side_length - 10 &&
                qr_module_get(qr, j + 7, i) == QR_MODULE_LIGHT &&
                qr_module_get(qr, j + 8, i) == QR_MODULE_LIGHT &&
                qr_module_get(qr, j + 9, i) == QR_MODULE_LIGHT &&
                qr_module_get(qr, j + 10, i) == QR_MODULE_LIGHT;

            if ((pattern_row && (preceded_row || followed_row)) || (pattern_column && (preceded_column || followed_column)))
                points += N[2];
        }
    }

    return points;
}

static int
feature_4_evaluation(const qr_code *qr)
{
    // proportion of dark modules in entire symbol
    size_t i, j, dark_modules = 0;

    for (i = 0; i < qr->side_length; ++i)
    {
        for (j = 0; j < qr->side_length; ++j)
        {
            if (qr_module_get(qr, i, j) == QR_MODULE_DARK)
                ++dark_modules;
        }
    }

    int percentage = (dark_modules * 100) / (qr->side_length * qr->side_length);
    int deviation = percentage - 50;
    if (deviation < 0) deviation = -deviation;
    return N[3] * (deviation / 5);
}

int
qr_mask_evaluate(const qr_code *qr)
{
    return
        feature_1_evaluation(qr) +
        feature_2_evaluation(qr) +
        feature_3_evaluation(qr) +
        feature_4_evaluation(qr);
}

void
qr_mask_apply_pattern(qr_code *qr, size_t mask_pattern)
{
    if (mask_pattern >= QR_MASK_PATTERN_COUNT) return;

    size_t i, j;

    for (i = 0; i < qr->side_length; ++i)
    {
        for (j = 0; j < qr->side_length; ++j)
        {
            if (qr_module_is_reserved(qr, i, j) || !MASK_PREDICATES[mask_pattern](i, j)) continue;

            qr_module_set(qr, i, j, qr_module_get(qr, i, j) ^ 1);
        }
    }
}

void
qr_mask_apply(qr_code *qr)
{
    int score, best_score = -1;
    size_t mask, best_mask;

    for (mask = 0; mask < QR_MASK_PATTERN_COUNT; ++mask)
    {
        qr->mask = mask;
        qr_mask_apply_pattern(qr, mask);
        qr_format_info_apply(qr);

        score = qr_mask_evaluate(qr);

        if (score < best_score)
        {
            best_score = score;
            best_mask = mask;
        }

        qr_mask_apply_pattern(qr, mask);
    }

    qr->mask = best_mask;
    qr_mask_apply_pattern(qr, best_mask);
    qr_format_info_apply(qr);
}
