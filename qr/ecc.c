#include <assert.h>
#include <qr/ecc.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define GF_SIZE 256
#define PRIMITIVE 0x11D

word gf_log[GF_SIZE];
static word gf_antilog[(GF_SIZE * 2) - 2];
static int gf_tables_initialized = 0;

static void
gf_init_log_antilog(void)
{
	if (gf_tables_initialized) return;

	size_t i;
	word x = 1;

	for (i = 0; i < GF_SIZE - 1; ++i)
	{
		gf_antilog[i] = x;
		gf_antilog[i + (GF_SIZE - 1)] = x;
		gf_log[x] = i;
		x = (x << 1) ^ ((x & 0x80) ? PRIMITIVE : 0);
	}

	gf_tables_initialized = 1;
}

static inline word
gf_mul(word a, word b)
{
	if (a == 0 || b == 0) return 0;
	return gf_antilog[gf_log[a] + gf_log[b]];
}

static inline word
gf_add(word a, word b)
{
	return a ^ b;
}

static void
generator_polynomial(word *poly, size_t degree)
{
	size_t i, j;

	for (i = 0; i < degree; ++i)
		poly[i] = 0;
	poly[degree] = 1;

	for (i = 0; i < degree; ++i)
	{
		word coef = gf_antilog[i];

		for (j = 0; j < degree; ++j)
			poly[j] = gf_add(poly[j + 1], gf_mul(poly[j], coef));
		poly[degree] = gf_mul(poly[degree], coef);
	}
}

static void
ecc_generate(const word *data, size_t data_length, word *ecc, size_t ecc_length, word g[ecc_length])
{
	size_t i, j;
	word feedback;

	for (i = 0; i < ecc_length; ++i)
		ecc[i] = 0;

	for (i = 0; i < data_length; ++i)
	{
		feedback = gf_add(data[i], ecc[0]);
		for (j = 0; j < ecc_length - 1; ++j)
			ecc[j] = gf_add(ecc[j + 1], gf_mul(feedback, g[j]));
		ecc[ecc_length - 1] = gf_mul(feedback, g[ecc_length - 1]);
	}
}

#define BLOCK_TYPES_PER_VERSION 2
static const size_t BLOCK_COUNT[QR_EC_LEVEL_COUNT][QR_VERSION_COUNT][BLOCK_TYPES_PER_VERSION] =
{
	{ // L
		{   1,   0 }, {   1,   0 }, {   1,   0 }, {   1,   0 }, {   1,   0 }, {   2,   0 }, {   2,   0 }, {   2,   0 }, {   2,   0 }, {   2,   2 },
		{   4,   0 }, {   2,   2 }, {   4,   0 }, {   3,   1 }, {   5,   1 }, {   5,   1 }, {   1,   5 }, {   5,   1 }, {   3,   4 }, {   3,   5 },
		{   4,   4 }, {   2,   7 }, {   4,   5 }, {   6,   4 }, {   8,   4 }, {  10,   2 }, {   8,   4 }, {   3,  10 }, {   7,   7 }, {   5,  10 },
		{  13,   3 }, {  17,   0 }, {  17,   1 }, {  13,   6 }, {  12,   7 }, {   6,  14 }, {  17,   4 }, {   4,  18 }, {  20,   4 }, {  19,   6 },
	},
	{ // M
		{   1,   0 }, {   1,   0 }, {   1,   0 }, {   2,   0 }, {   2,   0 }, {   4,   0 }, {   4,   0 }, {   2,   2 }, {   3,   2 }, {   4,   1 },
		{   1,   4 }, {   6,   2 }, {   8,   1 }, {   4,   5 }, {   5,   5 }, {   7,   3 }, {  10,   1 }, {   9,   4 }, {   3,  11 }, {   3,  13 },
		{  17,   0 }, {  17,   0 }, {   4,  14 }, {   6,  14 }, {   8,  13 }, {  19,   4 }, {  22,   3 }, {   3,  23 }, {  21,   7 }, {  19,  10 },
		{   2,  29 }, {  10,  23 }, {  14,  21 }, {  14,  23 }, {  12,  26 }, {   6,  34 }, {  29,  14 }, {  13,  32 }, {  40,   7 }, {  18,  31 },
	},
	{ // Q
		{   1,   0 }, {   1,   0 }, {   2,   0 }, {   2,   0 }, {   2,   2 }, {   4,   0 }, {   2,   4 }, {   4,   2 }, {   4,   4 }, {   6,   2 },
		{   4,   4 }, {   4,   6 }, {   8,   4 }, {  11,   5 }, {   5,   7 }, {  15,   2 }, {   1,  15 }, {  17,   1 }, {  17,   4 }, {  15,   5 },
		{  17,   6 }, {   7,  16 }, {  11,  14 }, {  11,  16 }, {   7,  22 }, {  28,   6 }, {   8,  26 }, {   4,  31 }, {   1,  37 }, {  15,  25 },
		{  42,   1 }, {  10,  35 }, {  29,  19 }, {  44,   7 }, {  39,  14 }, {  46,  10 }, {  49,  10 }, {  48,  14 }, {  43,  22 }, {  34,  34 },
	},
	{ // H
		{   1,   0 }, {   1,   0 }, {   2,   0 }, {   4,   0 }, {   2,   2 }, {   4,   0 }, {   4,   1 }, {   4,   2 }, {   4,   4 }, {   6,   2 },
		{   3,   8 }, {   7,   4 }, {  12,   4 }, {  11,   5 }, {  11,   7 }, {   3,  13 }, {   2,  17 }, {   2,  19 }, {   9,  16 }, {  15,  10 },
		{  19,   6 }, {  34,   0 }, {  16,  14 }, {  30,   2 }, {  22,  13 }, {  33,   4 }, {  12,  28 }, {  11,  31 }, {  19,  26 }, {  23,  25 },
		{  23,  28 }, {  19,  35 }, {  11,  46 }, {  59,   1 }, {  22,  41 }, {   2,  64 }, {  24,  46 }, {  42,  32 }, {  10,  67 }, {  20,  61 },
	}
};

static const size_t TOTAL_CODEWORD_COUNT[QR_EC_LEVEL_COUNT][QR_VERSION_COUNT][BLOCK_TYPES_PER_VERSION] =
{
	{ // L
		{  26,   0 }, {  44,   0 }, {  70,   0 }, { 100,   0 }, { 134,   0 }, {  86,   0 }, {  98,   0 }, { 121,   0 }, { 146,   0 }, {  86,  87 },
		{ 101,   0 }, { 116, 117 }, { 133,   0 }, { 145, 146 }, { 109, 110 }, { 122, 123 }, { 135, 136 }, { 150, 151 }, { 141, 142 }, { 135, 136 },
		{ 144, 145 }, { 139, 140 }, { 151, 152 }, { 147, 148 }, { 132, 133 }, { 142, 143 }, { 152, 153 }, { 147, 148 }, { 146, 147 }, { 145, 146 },
		{ 145, 146 }, { 145,   0 }, { 145, 146 }, { 145, 146 }, { 151, 152 }, { 151, 152 }, { 152, 153 }, { 152, 153 }, { 147, 148 }, { 148, 149 },
	},
	{ // M
		{  26,   0 }, {  44,   0 }, {  70,   0 }, {  50,   0 }, {  67,   0 }, {  43,   0 }, {  49,   0 }, {  60,  61 }, {  58,  59 }, {  69,  70 },
		{  80,  81 }, {  58,  59 }, {  59,  60 }, {  64,  65 }, {  65,  66 }, {  73,  74 }, {  74,  75 }, {  69,  70 }, {  70,  71 }, {  67,  68 },
		{  68,   0 }, {  74,   0 }, {  75,  76 }, {  73,  74 }, {  75,  76 }, {  74,  75 }, {  73,  74 }, {  73,  74 }, {  73,  74 }, {  75,  76 },
		{  74,  75 }, {  74,  75 }, {  74,  75 }, {  74,  75 }, {  75,  76 }, {  75,  76 }, {  74,  75 }, {  74,  75 }, {  75,  76 }, {  75,  76 },
	},
	{ // Q
		{  26,   0 }, {  44,   0 }, {  35,   0 }, {  50,   0 }, {  33,  34 }, {  43,   0 }, {  32,  33 }, {  40,  41 }, {  36,  37 }, {  43,  44 },
		{  50,  51 }, {  46,  47 }, {  44,  45 }, {  36,  37 }, {  54,  55 }, {  43,  44 }, {  50,  51 }, {  50,  51 }, {  47,  48 }, {  54,  55 },
		{  50,  51 }, {  54,  55 }, {  54,  55 }, {  54,  55 }, {  54,  55 }, {  50,  51 }, {  53,  54 }, {  54,  55 }, {  53,  54 }, {  54,  55 },
		{  54,  55 }, {  54,  55 }, {  54,  55 }, {  54,  55 }, {  54,  55 }, {  54,  55 }, {  54,  55 }, {  54,  55 }, {  54,  55 }, {  54,  55 },
	},
	{ // H
		{  26,   0 }, {  44,   0 }, {  35,   0 }, {  25,   0 }, {  33,  34 }, {  43,   0 }, {  39,  40 }, {  40,  41 }, {  36,  37 }, {  43,  44 },
		{  36,  37 }, {  42,  43 }, {  33,  34 }, {  36,  37 }, {  36,  37 }, {  45,  46 }, {  42,  43 }, {  42,  43 }, {  39,  40 }, {  43,  44 },
		{  46,  47 }, {  37,   0 }, {  45,  46 }, {  46,  47 }, {  45,  46 }, {  46,  47 }, {  45,  46 }, {  45,  46 }, {  45,  46 }, {  45,  46 },
		{  45,  46 }, {  45,  46 }, {  45,  46 }, {  46,  47 }, {  45,  46 }, {  45,  46 }, {  45,  46 }, {  45,  46 }, {  45,  46 }, {  45,  46 },
	}
};

static const size_t DATA_CODEWORD_COUNT[QR_EC_LEVEL_COUNT][QR_VERSION_COUNT][BLOCK_TYPES_PER_VERSION] =
{
	{ // L
		{  19,   0 }, {  34,   0 }, {  55,   0 }, {  80,   0 }, { 108,   0 }, {  68,   0 }, {  78,   0 }, {  97,   0 }, { 116,   0 }, {  68,  69 },
		{  81,   0 }, {  92,  93 }, { 107,   0 }, { 115, 116 }, {  87,  88 }, {  98,  99 }, { 107, 108 }, { 120, 121 }, { 113, 114 }, { 107, 108 },
		{ 116, 117 }, { 111, 112 }, { 121, 122 }, { 117, 118 }, { 106, 107 }, { 114, 115 }, { 122, 123 }, { 117, 118 }, { 116, 117 }, { 115, 116 },
		{ 115, 116 }, { 115,   0 }, { 115, 116 }, { 115, 116 }, { 121, 122 }, { 121, 122 }, { 122, 123 }, { 122, 123 }, { 117, 118 }, { 118, 119 },
	},
	{ // M
		{  16,   0 }, {  28,   0 }, {  44,   0 }, {  32,   0 }, {  43,   0 }, {  27,   0 }, {  31,   0 }, {  38,  39 }, {  36,  37 }, {  43,  44 },
		{  50,  51 }, {  36,  37 }, {  37,  38 }, {  40,  41 }, {  41,  42 }, {  45,  46 }, {  46,  47 }, {  43,  44 }, {  44,  45 }, {  41,  42 },
		{  42,   0 }, {  46,   0 }, {  47,  48 }, {  45,  46 }, {  47,  48 }, {  46,  47 }, {  45,  46 }, {  45,  46 }, {  45,  46 }, {  47,  48 },
		{  46,  47 }, {  46,  47 }, {  46,  47 }, {  46,  47 }, {  47,  48 }, {  47,  48 }, {  46,  47 }, {  46,  47 }, {  47,  48 }, {  47,  48 },
	},
	{ // Q
		{  13,   0 }, {  22,   0 }, {  17,   0 }, {  24,   0 }, {  15,  16 }, {  19,   0 }, {  14,  15 }, {  18,  19 }, {  16,  17 }, {  19,  20 },
		{  22,  23 }, {  20,  21 }, {  20,  21 }, {  16,  17 }, {  24,  25 }, {  19,  20 }, {  22,  23 }, {  22,  23 }, {  21,  22 }, {  24,  25 },
		{  22,  23 }, {  24,  25 }, {  24,  25 }, {  24,  25 }, {  24,  25 }, {  22,  23 }, {  23,  24 }, {  24,  25 }, {  23,  24 }, {  24,  25 },
		{  24,  25 }, {  24,  25 }, {  24,  25 }, {  24,  25 }, {  24,  25 }, {  24,  25 }, {  24,  25 }, {  24,  25 }, {  24,  25 }, {  24,  25 },
	},
	{ // H
		{   9,   0 }, {  16,   0 }, {  13,   0 }, {   9,   0 }, {  11,  12 }, {  15,   0 }, {  13,  14 }, {  14,  15 }, {  12,  13 }, {  15,  16 },
		{  12,  13 }, {  14,  15 }, {  11,  12 }, {  12,  13 }, {  12,  13 }, {  15,  16 }, {  14,  15 }, {  14,  15 }, {  13,  14 }, {  15,  16 },
		{  16,  17 }, {  13,   0 }, {  15,  16 }, {  16,  17 }, {  15,  16 }, {  16,  17 }, {  15,  16 }, {  15,  16 }, {  15,  16 }, {  15,  16 },
		{  15,  16 }, {  15,  16 }, {  15,  16 }, {  16,  17 }, {  15,  16 }, {  15,  16 }, {  15,  16 }, {  15,  16 }, {  15,  16 }, {  15,  16 },
	}
};

static const size_t TOTAL_DATA_CODEWORD_COUNT[QR_EC_LEVEL_COUNT][QR_VERSION_COUNT] =
{
	{ // L
		  19,   34,   55,   80,  108,  136,  156,  194,  232,  274,
		 324,  370,  428,  461,  523,  589,  647,  721,  795,  861,
		 932, 1006, 1094, 1174, 1276, 1370, 1468, 1531, 1631, 1735,
		1843, 1955, 2071, 2191, 2306, 2434, 2566, 2702, 2812, 2956,
	},
	{ // M
		  16,   28,   44,   64,   86,  108,  124,  154,  182,  216,
		 254,  290,  334,  365,  415,  453,  507,  563,  627,  669,
		 714,  782,  860,  914, 1000, 1062, 1128, 1193, 1267, 1373,
		1455, 1541, 1631, 1725, 1812, 1914, 1992, 2102, 2216, 2334,
	},
	{ // Q
		  13,   22,   34,   48,   62,   76,   88,  110,  132,  154,
		 180,  206,  244,  261,  295,  325,  367,  397,  445,  485,
		 512,  568,  614,  664,  718,  754,  808,  871,  911,  985,
		1033, 1115, 1171, 1231, 1286, 1354, 1426, 1502, 1582, 1666,
	},
	{ // H
		   9,   16,   26,   36,   46,   60,   66,   86,  100,  122,
		 140,  158,  180,  197,  223,  253,  283,  313,  341,  385,
		 406,  442,  464,  514,  538,  596,  628,  661,  701,  745,
		 793,  845,  901,  961,  986, 1054, 1096, 1142, 1222, 1276,
	}
};

void
qr_ec_encode(qr_code *qr)
{
	gf_init_log_antilog();

	size_t i, j, data_length, ecc_length;
	word *data = qr->codewords;
	word *ecc = qr->codewords + TOTAL_DATA_CODEWORD_COUNT[qr->level][qr->version];

	for (i = 0; i < BLOCK_TYPES_PER_VERSION; ++i)
	{
		data_length = DATA_CODEWORD_COUNT[qr->level][qr->version][i];
		ecc_length = TOTAL_CODEWORD_COUNT[qr->level][qr->version][i] - data_length;
		word generator[ecc_length + 1];
		generator_polynomial(generator, ecc_length);

		for (j = 0; j < BLOCK_COUNT[qr->level][qr->version][i]; ++j)
		{
			ecc_generate(data, data_length, ecc, ecc_length, generator + 1);
			data += data_length;
			ecc += ecc_length;
		}
	}

	assert(data - qr->codewords == (long int) TOTAL_DATA_CODEWORD_COUNT[qr->level][qr->version] && "Sum of data codewords in blocks do not match expected number of data codewords");
	assert(ecc - qr->codewords == (long int) qr->codeword_count && "Number of generated ec codewords do not match the expected number of codewords");
}

static word *
interleave_words(const size_t codeword_count[BLOCK_TYPES_PER_VERSION], const size_t block_count[BLOCK_TYPES_PER_VERSION], word *in, word *out)
{
	size_t i, block, codeword;
	size_t block_offsets[BLOCK_TYPES_PER_VERSION], max_codeword_count = 0;

	for (i = 0; i < BLOCK_TYPES_PER_VERSION; ++i)
	{
		block_offsets[i] = i ? block_offsets[i - 1] + (codeword_count[i - 1] * block_count[i - 1]) : 0;
		if (codeword_count[i] > max_codeword_count)
			max_codeword_count = codeword_count[i];
	}

	for (codeword = 0; codeword < max_codeword_count; ++codeword)
	{
		for (i = 0; i < BLOCK_TYPES_PER_VERSION; ++i)
		{
			if (codeword >= codeword_count[i]) continue;

			for (block = 0; block < block_count[i]; ++block)
				*(out++) = in[(block * codeword_count[i]) + codeword + block_offsets[i]];
		}
	}

	return out;
}

void
qr_interleave_codewords(qr_code *qr)
{
	word final_message[qr->codeword_count], *word_ptr = final_message;
	const size_t *data_codeword_count = DATA_CODEWORD_COUNT[qr->level][qr->version];
	const size_t *block_count = BLOCK_COUNT[qr->level][qr->version];
	size_t i, ecc_codeword_count[BLOCK_TYPES_PER_VERSION];

	for (i = 0; i < BLOCK_TYPES_PER_VERSION; ++i)
		ecc_codeword_count[i] = TOTAL_CODEWORD_COUNT[qr->level][qr->version][i] - data_codeword_count[i];

	word_ptr = interleave_words(data_codeword_count, block_count, qr->codewords, word_ptr);
	word_ptr = interleave_words(ecc_codeword_count, block_count, qr->codewords + TOTAL_DATA_CODEWORD_COUNT[qr->level][qr->version], word_ptr);

	assert(word_ptr == final_message + qr->codeword_count && "Length of interleaved message does not match length of original message");

	for (i = 0; i < qr->codeword_count; ++i)
		qr->codewords[i] = final_message[i];
}
