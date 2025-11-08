#include <qr/info.h>
#include <qr/mask.h>
#include <qr/matrix.h>

static const unsigned ECL_MAP[QR_EC_LEVEL_COUNT] =
{
    [QR_EC_LEVEL_L]=8,
    [QR_EC_LEVEL_M]=0,
    [QR_EC_LEVEL_Q]=24,
    [QR_EC_LEVEL_H]=16,
};

static const unsigned FORMAT_INFO_MAP[QR_EC_LEVEL_COUNT * QR_MASK_PATTERN_COUNT] =
{
    0x5412, 0x5125, 0x5E7C, 0x5B4B, 0x45F9, 0x40CE, 0x4F97, 0x4AA0,
    0x77C4, 0x72F3, 0x7DAA, 0x789D, 0x662F, 0x6318, 0x6C41, 0x6976,
    0x1689, 0x13BE, 0x1CE7, 0x19D0, 0x0762, 0x0355, 0x0D0C, 0x083B,
    0x355F, 0x3068, 0x3F31, 0x3A06, 0x24B4, 0x2183, 0x2EDA, 0x2BED,
};

void
qr_format_info_apply(qr_code *qr)
{
    unsigned format_info = FORMAT_INFO_MAP[ECL_MAP[qr->ec_level] + qr->mask];

    // upper left
    qr_module_set(qr, 0, 8, (format_info >> 0) & 1);
    qr_module_set(qr, 1, 8, (format_info >> 1) & 1);
    qr_module_set(qr, 2, 8, (format_info >> 2) & 1);
    qr_module_set(qr, 3, 8, (format_info >> 3) & 1);
    qr_module_set(qr, 4, 8, (format_info >> 4) & 1);
    qr_module_set(qr, 5, 8, (format_info >> 5) & 1);
    qr_module_set(qr, 7, 8, (format_info >> 6) & 1);
    qr_module_set(qr, 8, 8, (format_info >> 7) & 1);
    qr_module_set(qr, 8, 7, (format_info >> 8) & 1);
    qr_module_set(qr, 8, 5, (format_info >> 9) & 1);
    qr_module_set(qr, 8, 4, (format_info >> 10) & 1);
    qr_module_set(qr, 8, 3, (format_info >> 11) & 1);
    qr_module_set(qr, 8, 2, (format_info >> 12) & 1);
    qr_module_set(qr, 8, 1, (format_info >> 13) & 1);
    qr_module_set(qr, 8, 0, (format_info >> 14) & 1);

    // upper right
    qr_module_set(qr, 8, qr->side_length - 1, (format_info >> 0) & 1);
    qr_module_set(qr, 8, qr->side_length - 2, (format_info >> 1) & 1);
    qr_module_set(qr, 8, qr->side_length - 3, (format_info >> 2) & 1);
    qr_module_set(qr, 8, qr->side_length - 4, (format_info >> 3) & 1);
    qr_module_set(qr, 8, qr->side_length - 5, (format_info >> 4) & 1);
    qr_module_set(qr, 8, qr->side_length - 6, (format_info >> 5) & 1);
    qr_module_set(qr, 8, qr->side_length - 7, (format_info >> 6) & 1);
    qr_module_set(qr, 8, qr->side_length - 8, (format_info >> 7) & 1);

    // lower left
    qr_module_set(qr, qr->side_length - 8, 8, QR_MODULE_DARK);
    qr_module_set(qr, qr->side_length - 7, 8, (format_info >> 8) & 1);
    qr_module_set(qr, qr->side_length - 6, 8, (format_info >> 9) & 1);
    qr_module_set(qr, qr->side_length - 5, 8, (format_info >> 10) & 1);
    qr_module_set(qr, qr->side_length - 4, 8, (format_info >> 11) & 1);
    qr_module_set(qr, qr->side_length - 3, 8, (format_info >> 12) & 1);
    qr_module_set(qr, qr->side_length - 2, 8, (format_info >> 13) & 1);
    qr_module_set(qr, qr->side_length - 1, 8, (format_info >> 14) & 1);
}

static const unsigned VERSION_INFO_MAP[QR_VERSION_COUNT] = {
    0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x07C94, 0x085BC, 0x09A99, 0x0A4D3,
    0x0BBF6, 0x0C762, 0x0D847, 0x0E60D, 0x0F928, 0x10B78, 0x1145D, 0x12A17, 0x13532, 0x149A6,
    0x15683, 0x168C9, 0x177EC, 0x18EC4, 0x191E1, 0x1AFAB, 0x1B08E, 0x1CC1A, 0x1D33F, 0x1ED75,
    0x1F250, 0x209D5, 0x216F0, 0x228BA, 0x2379F, 0x24B0B, 0x2542E, 0x26A64, 0x27541, 0x28C69,
};

void
qr_version_info_apply(qr_code *qr)
{
    unsigned version_info = VERSION_INFO_MAP[qr->version];

    // upper right
    qr_module_set(qr, 0, qr->side_length - 11, (version_info >> 0) & 1);
    qr_module_set(qr, 0, qr->side_length - 10, (version_info >> 1) & 1);
    qr_module_set(qr, 0, qr->side_length -  9, (version_info >> 2) & 1);
    qr_module_set(qr, 1, qr->side_length - 11, (version_info >> 3) & 1);
    qr_module_set(qr, 1, qr->side_length - 10, (version_info >> 4) & 1);
    qr_module_set(qr, 1, qr->side_length -  9, (version_info >> 5) & 1);
    qr_module_set(qr, 2, qr->side_length - 11, (version_info >> 6) & 1);
    qr_module_set(qr, 2, qr->side_length - 10, (version_info >> 7) & 1);
    qr_module_set(qr, 2, qr->side_length -  9, (version_info >> 8) & 1);
    qr_module_set(qr, 3, qr->side_length - 11, (version_info >> 9) & 1);
    qr_module_set(qr, 3, qr->side_length - 10, (version_info >> 10) & 1);
    qr_module_set(qr, 3, qr->side_length -  9, (version_info >> 11) & 1);
    qr_module_set(qr, 4, qr->side_length - 11, (version_info >> 12) & 1);
    qr_module_set(qr, 4, qr->side_length - 10, (version_info >> 13) & 1);
    qr_module_set(qr, 4, qr->side_length -  9, (version_info >> 14) & 1);
    qr_module_set(qr, 5, qr->side_length - 11, (version_info >> 15) & 1);
    qr_module_set(qr, 5, qr->side_length - 10, (version_info >> 16) & 1);
    qr_module_set(qr, 5, qr->side_length -  9, (version_info >> 17) & 1);

    // lower left
    qr_module_set(qr, qr->side_length - 11, 0, (version_info >> 0) & 1);
    qr_module_set(qr, qr->side_length - 10, 0, (version_info >> 1) & 1);
    qr_module_set(qr, qr->side_length -  9, 0, (version_info >> 2) & 1);
    qr_module_set(qr, qr->side_length - 11, 1, (version_info >> 3) & 1);
    qr_module_set(qr, qr->side_length - 10, 1, (version_info >> 4) & 1);
    qr_module_set(qr, qr->side_length -  9, 1, (version_info >> 5) & 1);
    qr_module_set(qr, qr->side_length - 11, 2, (version_info >> 6) & 1);
    qr_module_set(qr, qr->side_length - 10, 2, (version_info >> 7) & 1);
    qr_module_set(qr, qr->side_length -  9, 2, (version_info >> 8) & 1);
    qr_module_set(qr, qr->side_length - 11, 3, (version_info >> 9) & 1);
    qr_module_set(qr, qr->side_length - 10, 3, (version_info >> 10) & 1);
    qr_module_set(qr, qr->side_length -  9, 3, (version_info >> 11) & 1);
    qr_module_set(qr, qr->side_length - 11, 4, (version_info >> 12) & 1);
    qr_module_set(qr, qr->side_length - 10, 4, (version_info >> 13) & 1);
    qr_module_set(qr, qr->side_length -  9, 4, (version_info >> 14) & 1);
    qr_module_set(qr, qr->side_length - 11, 5, (version_info >> 15) & 1);
    qr_module_set(qr, qr->side_length - 10, 5, (version_info >> 16) & 1);
    qr_module_set(qr, qr->side_length -  9, 5, (version_info >> 17) & 1);
}
