#include <assert.h>
#include <qr/enc.h>
#include <qr/types.h>
#include <stddef.h>
#include <string.h>

static const size_t CAPACITY_BYTES[QR_EC_LEVEL_COUNT][QR_VERSION_COUNT] =
{
	{ // L
		  17,   32,   53,   78,  106,  134,  154,  192,  230,  271,
		 321,  367,  425,  458,  520,  586,  644,  718,  792,  858,
		 929, 1003, 1091, 1171, 1273, 1367, 1465, 1528, 1628, 1732,
		1840, 1952, 2068, 2188, 2303, 2431, 2563, 2699, 2809, 2953,
	},
	{ // M
		  14,   26,   42,   62,   84,  106,  122,  152,  180,  213,
		 251,  287,  331,  362,  412,  450,  504,  560,  624,  666,
		 711,  779,  857,  911,  997, 1059, 1125, 1190, 1264, 1370,
		1452, 1538, 1628, 1722, 1809, 1911, 1989, 2099, 2213, 2331,
	},
	{ // Q
		  11,   20,   32,   46,   60,   74,   86,  108,  130,  151,
		 177,  203,  241,  258,  292,  322,  364,  394,  442,  482,
		 509,  565,  611,  661,  715,  751,  805,  868,  908,  982,
		1030, 1112, 1168, 1228, 1283, 1351, 1423, 1499, 1579, 1663,
	},
	{ // H
		   7,   14,   24,   34,   44,   58,   64,   84,   98,  119,
		 137,  155,  177,  194,  220,  250,  280,  310,  338,  382,
		 403,  439,  461,  511,  535,  593,  625,  658,  698,  742,
		 790,  842,  898,  958,  983, 1051, 1093, 1139, 1219, 1273,
	}
};

unsigned
qr_min_version(size_t bytes, qr_ec_level level)
{
	size_t i;

	for (i = 0; i < QR_VERSION_COUNT && bytes > CAPACITY_BYTES[level][i]; ++i);

	return (unsigned) i;
}

static void
append_bit(word *buffer, size_t *byte, size_t *bit, int value)
{
	buffer[*byte] |= (value & 1) << (7 - *bit);

	if (++*bit == 8)
	{
		*bit = 0;
		++*byte;
	}
}

static void
append_byte(word *buffer, size_t *byte, size_t *bit, word value)
{
	size_t i;

	for (i = 7; i < 8; --i)
		append_bit(buffer, byte, bit, (value >> i) & 1);
}

void
qr_encode_data(qr_code *qr, const char *message)
{
	size_t i, length, byte = 0, bit = 0;

	switch (qr->mode)
	{
	case QR_MODE_BYTE:
		length = strlen(message);
		assert(length <= CAPACITY_BYTES[qr->level][qr->version] && "Message provided is too large");

		// byte mode indicator
		append_bit(qr->codewords, &byte, &bit, 0);
		append_bit(qr->codewords, &byte, &bit, 1);
		append_bit(qr->codewords, &byte, &bit, 0);
		append_bit(qr->codewords, &byte, &bit, 0);

		// character count indicator
		for (i = qr->version <= 9 ? 7 : 15; i < 16; --i)
			append_bit(qr->codewords, &byte, &bit, (length >> i) & 1);

		// data
		for (i = 0; i < length; ++i)
			append_byte(qr->codewords, &byte, &bit, message[i]);

		// terminator
		append_bit(qr->codewords, &byte, &bit, 0);
		append_bit(qr->codewords, &byte, &bit, 0);
		append_bit(qr->codewords, &byte, &bit, 0);
		append_bit(qr->codewords, &byte, &bit, 0);

		// padding
		while (bit % 8)
			append_bit(qr->codewords, &byte, &bit, 0);
		for (i = 0; i < CAPACITY_BYTES[qr->level][qr->version] - length; ++i)
			append_byte(qr->codewords, &byte, &bit, i % 2 == 0 ? 0xEC : 0x11);
	}
}
