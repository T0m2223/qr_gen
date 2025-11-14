#include <qr/matrix.h>
#include <qr/patterns.h>
#include <qr/types.h>
#include <stddef.h>

static void
add_finder_pattern_at(qr_code *qr, size_t i, size_t j)
{
	size_t di, dj;

	for (di = 0; di < 7; ++di)
		for (dj = 0; dj < 7; ++dj)
			qr_module_set(qr, i + di, j + dj, QR_MODULE_DARK);

	for (di = 1; di < 6; ++di)
		for (dj = 1; dj < 6; ++dj)
			qr_module_set(qr, i + di, j + dj, QR_MODULE_LIGHT);

	for (di = 2; di < 5; ++di)
		for (dj = 2; dj < 5; ++dj)
			qr_module_set(qr, i + di, j + dj, QR_MODULE_DARK);
}

void
qr_finder_patterns_apply(qr_code *qr)
{
	add_finder_pattern_at(qr, 0, 0);
	add_finder_pattern_at(qr, qr->side_length - 7, 0);
	add_finder_pattern_at(qr, 0, qr->side_length - 7);
}

void
qr_separators_apply(qr_code *qr)
{
	size_t i;

	for (i = 0; i < 8; ++i)
	{
		// upper left
		qr_module_set(qr, i, 8, QR_MODULE_LIGHT);
		qr_module_set(qr, 8, i, QR_MODULE_LIGHT);

		// upper right
		qr_module_set(qr, i, qr->side_length - 8, QR_MODULE_LIGHT);
		qr_module_set(qr, 8, qr->side_length - 8 + i, QR_MODULE_LIGHT);

		// lower left
		qr_module_set(qr, qr->side_length - 8 + i, 8, QR_MODULE_LIGHT);
		qr_module_set(qr, qr->side_length - 8, i, QR_MODULE_LIGHT);
	}
}

void
qr_timing_patterns_apply(qr_code *qr)
{
	size_t i;

	for (i = 8; i < qr->side_length - 8; ++i)
	{
		qr_module_set(qr, i, 6, (i % 2) ^ 1);
		qr_module_set(qr, 6, i, (i % 2) ^ 1);
	}
}

static void
add_alignment_pattern_at(qr_code *qr, size_t i, size_t j)
{
	size_t di, dj;

	for (di = 0; di < 5; ++di)
		for (dj = 0; dj < 5; ++dj)
			qr_module_set(qr, i + di, j + dj, QR_MODULE_DARK);

	for (di = 1; di < 4; ++di)
		for (dj = 1; dj < 4; ++dj)
			qr_module_set(qr, i + di, j + dj, QR_MODULE_LIGHT);

	qr_module_set(qr, i + 2, j + 2, QR_MODULE_DARK);
}

#define MAX_ALIGNMENT_ENTRIES 7
#define E 2
static size_t ALIGNMENT_CENTER_MODULE[QR_VERSION_COUNT][MAX_ALIGNMENT_ENTRIES] =
{
	{   E,   E,   E,   E,   E,   E,   E },
	{   6,  18,   E,   E,   E,   E,   E },
	{   6,  22,   E,   E,   E,   E,   E },
	{   6,  26,   E,   E,   E,   E,   E },
	{   6,  30,   E,   E,   E,   E,   E },
	{   6,  34,   E,   E,   E,   E,   E },
	{   6,  22,  38,   E,   E,   E,   E },
	{   6,  24,  42,   E,   E,   E,   E },
	{   6,  26,  46,   E,   E,   E,   E },
	{   6,  28,  50,   E,   E,   E,   E },
	{   6,  30,  54,   E,   E,   E,   E },
	{   6,  32,  58,   E,   E,   E,   E },
	{   6,  34,  62,   E,   E,   E,   E },
	{   6,  26,  46,  66,   E,   E,   E },
	{   6,  26,  48,  70,   E,   E,   E },
	{   6,  26,  50,  74,   E,   E,   E },
	{   6,  30,  54,  78,   E,   E,   E },
	{   6,  30,  56,  82,   E,   E,   E },
	{   6,  30,  58,  86,   E,   E,   E },
	{   6,  34,  62,  90,   E,   E,   E },
	{   6,  28,  50,  72,  94,   E,   E },
	{   6,  26,  50,  74,  98,   E,   E },
	{   6,  30,  54,  78, 102,   E,   E },
	{   6,  28,  54,  80, 106,   E,   E },
	{   6,  32,  58,  84, 110,   E,   E },
	{   6,  30,  58,  86, 114,   E,   E },
	{   6,  34,  62,  90, 118,   E,   E },
	{   6,  26,  50,  74,  98, 122,   E },
	{   6,  30,  54,  78, 102, 126,   E },
	{   6,  26,  52,  78, 104, 130,   E },
	{   6,  30,  56,  82, 108, 134,   E },
	{   6,  34,  60,  86, 112, 138,   E },
	{   6,  30,  58,  86, 114, 142,   E },
	{   6,  34,  62,  90, 118, 146,   E },
	{   6,  30,  54,  78, 102, 126, 150 },
	{   6,  24,  50,  76, 102, 128, 154 },
	{   6,  28,  54,  80, 106, 132, 158 },
	{   6,  32,  58,  84, 110, 136, 162 },
	{   6,  26,  54,  82, 110, 138, 166 },
	{   6,  30,  58,  86, 114, 142, 170 },
};

void
qr_alignment_patterns_apply(qr_code *qr)
{
	size_t entry_a, entry_b, i, j;
	int in_finder_upper_left, in_finder_upper_right, in_finder_lower_left;

	for (entry_a = 0; entry_a < MAX_ALIGNMENT_ENTRIES; ++entry_a)
	{
		for (entry_b = 0; entry_b < MAX_ALIGNMENT_ENTRIES; ++entry_b)
		{
			i = ALIGNMENT_CENTER_MODULE[qr->version][entry_a] - 2;
			j = ALIGNMENT_CENTER_MODULE[qr->version][entry_b] - 2;

			in_finder_upper_left = i < 8 && j < 8;
			in_finder_upper_right = i < 8 && j >= qr->side_length - 12;
			in_finder_lower_left = i >= qr->side_length - 12 && j < 8;

			if (!i || !j || in_finder_upper_left || in_finder_upper_right || in_finder_lower_left)
				continue;

			add_alignment_pattern_at(qr, i, j);
		}
	}
}

int
qr_is_in_alignment_patterns(const qr_code *qr, size_t i_, size_t j_)
{
	size_t entry_a, entry_b, i, j;
	int in_finder_upper_left, in_finder_upper_right, in_finder_lower_left;

	for (entry_a = 0; entry_a < MAX_ALIGNMENT_ENTRIES; ++entry_a)
	{
		for (entry_b = 0; entry_b < MAX_ALIGNMENT_ENTRIES; ++entry_b)
		{
			i = ALIGNMENT_CENTER_MODULE[qr->version][entry_a] - 2;
			j = ALIGNMENT_CENTER_MODULE[qr->version][entry_b] - 2;

			in_finder_upper_left = i < 8 && j < 8;
			in_finder_upper_right = i < 8 && j >= qr->side_length - 12;
			in_finder_lower_left = i >= qr->side_length - 12 && j < 8;

			if (!i || !j || in_finder_upper_left || in_finder_upper_right || in_finder_lower_left)
				continue;

			if (i_ >= i && i_ <= i + 4 && j_ >= j && j_ <= j + 4)
				return 1;
		}
	}

	return 0;
}
