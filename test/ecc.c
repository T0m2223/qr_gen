/**
 * @file ecc.c
 * @brief Test cases for Error Correction Code (ECC) functionality
 *
 * This file contains test cases for the QR code error correction functionality,
 * including Galois Field arithmetic, generator polynomial creation, and codeword interleaving.
 */

#include <test/base.h>
#include <qr/types.h>
#include <stdio.h>

// Include the source file directly to test static functions
#include "../qr/ecc.c"
#include "../qr/qr.c"

/**
 * @brief Initialize the test environment
 *
 * Sets up the Galois Field log/antilog tables required for ECC calculations.
 * This function is automatically called before each test case.
 */
BEFORE() {
	gf_init_log_antilog();
	return TEST_SUCCESS;
}

/**
 * @brief Test basic Galois Field (2^8) arithmetic operations
 *
 * Verifies the implementation of Galois Field (GF) arithmetic operations:
 * - Addition (which is XOR in GF(2^8))
 * - Multiplication using log/antilog tables
 * - Edge cases with zero and one
 */
TEST(gf_arithmetic) {
	// Test multiplication
	assert_equal(gf_mul(2, 3), 6, "Galois Field multiplication of 2 and 3");
	assert_equal(gf_mul(0, 5), 0, "Galois Field multiplication with zero");
	assert_equal(gf_mul(7, 1), 7, "Galois Field multiplication with one");

	// Test addition (XOR in GF)
	assert_equal(gf_add(5, 3), 6, "Galois Field addition of 5 and 3");
	assert_equal(gf_add(0, 4), 4, "Galois Field addition with zero");

	// Test multiplication with log/antilog tables
	assert_equal(gf_mul(0x03, 0x0E), 18, "Galois Field multiplication of 0x03 and 0x0E");
	assert_equal(gf_mul(0x1A, 0x0B), 254, "Galois Field multiplication of 0x1A and 0x0B");

	return TEST_SUCCESS;
}

/**
 * @brief Test the generator polynomial creation
 *
 * Verifies that the generator polynomial creation function produces the
 * expected coefficients for given polynomial degrees. The generator polynomial
 * is used in the Reed-Solomon error correction process.
 */
TEST(generator_polynomial) {
	word poly[30];  // Large enough for testing

	// Test case 1: Degree 5 generator polynomial (6 coefficients)
	// g(x) = (x-α^0)(x-α^1)(x-α^2)(x-α^3)(x-α^4)
	generator_polynomial(poly, 5);

	// Expected exponents for the antilog table
	word expected5_exponents[6] = {0, 113, 164, 166, 119, 10};

	for (int i = 0; i <= 5; i++) {
		assert_equal(poly[i], gf_antilog[expected5_exponents[i]], 
			"Generator polynomial coefficient for degree 5");
	}

	// Test case 2: Degree 16 generator polynomial (17 coefficients)
	// Used in version 1-M QR codes
	generator_polynomial(poly, 16);

	word expected16_exponents[17] = {
		0, 120, 104, 107, 109, 102, 161, 76, 3, 91,
		191, 147, 169, 182, 194, 225, 120
	};

	for (int i = 0; i <= 16; i++) {
		assert_equal(poly[i], gf_antilog[expected16_exponents[i]], 
			"Generator polynomial coefficient for degree 16");
	}

	return TEST_SUCCESS;
}

/**
 * Test ECC generation for a simple case.
 * Verifies that the generated error correction codes match expected values.
 */
TEST(ecc_generation) {
	// Simple test case: Version 1-L (7 data codewords, 10 ECC codewords)
	word data[7] = {40, 88, 12, 6, 46, 77, 36};
	word ecc[10] = {0};
	word g[10 + 1] = {0};  // +1 for leading coefficient

	// Generate the generator polynomial for 10 ECC codewords
	generator_polynomial(g, 10);

	// Generate ECC (skip the leading coefficient in g)
	ecc_generate(data, 7, ecc, 10, g + 1);

	// Expected ECC values for the test data
	word expected_ecc[10] = {214, 246, 18, 193, 38, 69, 160, 197, 199, 15};

	// Compare generated ECC with expected values
	for (int i = 0; i < 10; i++) {
		assert_equal(ecc[i], expected_ecc[i], 
			"ECC generation mismatch at position");
	}

	return TEST_SUCCESS;
}

/**
 * Test the consistency of error correction tables.
 * Verifies that:
 * 1. When BLOCK_COUNT is 0, both TOTAL_CODEWORD_COUNT and DATA_CODEWORD_COUNT are 0
 * 2. BLOCK_COUNT * TOTAL_CODEWORD_COUNT matches the total codeword count
 * 3. The difference between TOTAL_CODEWORD_COUNT and DATA_CODEWORD_COUNT is consistent
 */
/**
 * Test the codeword interleaving functionality.
 * Verifies that the interleaving process correctly interleaves codewords from different blocks.
 * Using Version 1-H (version index 0, level H) which has:
 * - 1 block (no interleaving needed)
 * - 9 data codewords
 * - 17 ECC codewords (26 total - 9 data)
 */
TEST(codeword_interleaving_version1_h) {
	// Test case for Version 1-H QR code which has:
	// - 1 block (no interleaving needed)
	// - 9 data codewords
	// - 17 ECC codewords (26 total - 9 data)
	qr_code qr = {
		.level = QR_EC_LEVEL_H,
		.version = 0, // Version 1 (0-based index)
		.codeword_count = 26, // From TOTAL_CODEWORD_COUNT[H][0][0]
		.codewords = NULL,
		.matrix = NULL,
		.side_length = 0,
		.mask = 0,
		.mode = QR_MODE_BYTE
	};

	// Allocate and initialize test codewords
	word *test_codewords = calloc(qr.codeword_count, sizeof(word));
	if (!test_codewords) return TEST_FAILURE("Memory allocation failed");

	// Fill test data: [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26]
	for (size_t i = 0; i < qr.codeword_count; i++) {
		test_codewords[i] = (word) (i + 1);
	}

	qr.codewords = test_codewords;

	// Perform interleaving
	qr_interleave_codewords(&qr);

	// For Version 1-H with 1 block, the codewords should remain in the same order
	for (size_t i = 0; i < qr.codeword_count; i++) {
		assert_equal((int) qr.codewords[i], (int) (i + 1), 
			"Codeword order should remain unchanged for single block");
	}

	free(test_codewords);
	return TEST_SUCCESS;
}

/**
 * Test the codeword interleaving functionality with a more complex case.
 * Using Version 2-M (version index 1, level M) which has:
 * - Block Type 0: 2 blocks, 14 data codewords, 16 total codewords (2 ECC codewords)
 * - Block Type 1: 1 block, 15 data codewords, 17 total codewords (2 ECC codewords)
 */
TEST(codeword_interleaving_version8_m) {
	// Test case for Version 8-M QR code which has:
	// - 2 blocks of Type 0: 38 data + 22 ECC = 60 codewords each
	// - 2 blocks of Type 1: 39 data + 22 ECC = 61 codewords each
	// Total codewords = 2*60 + 2*61 = 242
	qr_code qr = {
		.level = QR_EC_LEVEL_M,
		.version = 7, // Version 8 (0-based index)
		.codeword_count = 242, // 2*60 + 2*61 = 242
		.codewords = NULL,
		.matrix = NULL,
		.side_length = 0,
		.mask = 0,
		.mode = QR_MODE_BYTE
	};

	// Allocate and initialize test codewords
	word *test_codewords = calloc(qr.codeword_count, sizeof(word));
	if (!test_codewords) return TEST_FAILURE("Memory allocation failed");

	// Fill test data:
	// Block 0-0 (type 0, block 0): [1, 2, ..., 60] (38 data + 22 ECC)
	// Block 0-1 (type 0, block 1): [101, 102, ..., 120] (38 data + 22 ECC)
	// Block 1-0 (type 1, block 0): [121, 122, ..., 181] (39 data + 22 ECC)
	// Block 1-1 (type 1, block 1): [182, 183, ..., 242] (39 data + 22 ECC)
	// Fill Block 0-0 (type 0, block 0)
	for (size_t i = 0; i < 242; i++) {
		test_codewords[i] = (word) (i + 1);
	}

	qr.codewords = test_codewords;

	// Perform interleaving
	qr_interleave_codewords(&qr);

	// Expected interleaving order:
	// 1. Data codewords from all blocks, interleaved
	//    - Take first data codeword from each block in order
	//    - Block 0-0: 60 data codewords (1-60)
	//    - Block 0-1: 60 data codewords (61-120)
	//    - Block 1-0: 61 data codewords (121-181)
	//    - Block 1-1: 61 data codewords (182-242)
	// 2. Then ECC codewords from all blocks, interleaved
	//    - Block 0-0: 22 ECC codewords (61-82)
	//    - Block 0-1: 22 ECC codewords (83-104)
	//    - Block 1-0: 22 ECC codewords (105-126)
	//    - Block 1-1: 22 ECC codewords (127-148)
	// Check first data codeword from each block
	assert_equal(qr.codewords[0], 1, "Data codeword value verification");
	assert_equal(qr.codewords[1], 39, "Data codeword value verification");
	assert_equal(qr.codewords[2], 77, "Data codeword value verification");

	// Check second data codeword from each block
	assert_equal(qr.codewords[3], 116, "Data codeword value verification");
	assert_equal(qr.codewords[4], 2, "Data codeword value verification");
	assert_equal(qr.codewords[5], 40, "Data codeword value verification");

	// Check last data codeword from each block
	assert_equal(qr.codewords[148], 38, "Data codeword value verification");
	assert_equal(qr.codewords[149], 76, "Data codeword value verification");
	assert_equal(qr.codewords[152], 115, "Data codeword value verification");
	assert_equal(qr.codewords[153], 154, "Data codeword value verification");

	// Check ECC codewords
	assert_equal(qr.codewords[154], 155, "ECC codeword value verification");
	assert_equal(qr.codewords[155], 177, "ECC codeword value verification");
	assert_equal(qr.codewords[156], 199, "ECC codeword value verification");
	assert_equal(qr.codewords[157], 221, "ECC codeword value verification");
	assert_equal(qr.codewords[158], 156, "ECC codeword value verification");
	assert_equal(qr.codewords[159], 178, "ECC codeword value verification");

	free(test_codewords);
	return TEST_SUCCESS;
}

/**
 * @brief Test the consistency of ECC tables
 *
 * Verifies that the ECC tables (BLOCK_COUNT, TOTAL_CODEWORD_COUNT, DATA_CODEWORD_COUNT)
 * are consistent with each other and follow the QR code specification.
 */
TEST(ecc_table_consistency) {
	for (int level = 0; level < QR_EC_LEVEL_COUNT; level++) {
		for (int version = 0; version < QR_VERSION_COUNT; version++) {
			size_t total_codewords = 0;
			size_t total_data_codewords = 0;

			// Check each block type
			for (int block_type = 0; block_type < BLOCK_TYPES_PER_VERSION; block_type++) {
				size_t block_count = BLOCK_COUNT[level][version][block_type];
				size_t total_cw = TOTAL_CODEWORD_COUNT[level][version][block_type];
				size_t data_cw = DATA_CODEWORD_COUNT[level][version][block_type];

				// Test 1: If block_count is 0, both total_cw and data_cw should be 0
				if (block_count == 0) {
					assert_equal(total_cw, 0, "Total codewords count when block count is 0");
					assert_equal(data_cw, 0, "Data codewords count when block count is 0");
					continue;
				}

				// Test 2: total_cw should be >= data_cw
				assert_greater_than_or_equal(total_cw, data_cw,
					"Total codewords should be >= data codewords");

				total_codewords += block_count * total_cw;
				total_data_codewords += block_count * data_cw;
			}

			// Test 3: Total data codewords should match the precomputed value
			assert_equal(total_data_codewords, TOTAL_DATA_CODEWORD_COUNT[level][version], 
				"Total data codewords should match precomputed value");

			// Test 4: Total codewords should match the version's capacity
			assert_equal(total_codewords, CODEWORD_COUNT[version], 
				"Total codewords should match version capacity");
		}
	}

	return TEST_SUCCESS;
}
