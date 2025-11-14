/**
 * @file matrix.c
 * @brief Test cases for QR code matrix operations
 * 
 * This file contains test cases for the QR code matrix module, including
 * module manipulation, reserved module detection, and codeword placement.
 */

#include <test/base.h>
#include <qr/types.h>
#include <qr/matrix.h>
#include <string.h>
#include <stdlib.h>

// Include the source file directly to test static functions
#include "../qr/matrix.c"

/**
 * @brief Creates a test QR code with the specified version and size
 * 
 * @param version QR code version (1-40)
 * @param size Side length of the QR code matrix
 * @return qr_code* Pointer to the created QR code, or NULL on failure
 */
static qr_code *create_test_qr(unsigned version, size_t size) {
	qr_code *qr = calloc(1, sizeof(qr_code));
	if (!qr) return NULL;

	qr->version = version;
	qr->side_length = size;
	qr->matrix = calloc(size * size, sizeof(int));
	
	if (!qr->matrix) {
		free(qr);
		return NULL;
	}
	
	return qr;
}

/**
 * @brief Frees resources allocated for a test QR code
 * 
 * @param qr Pointer to the QR code to free
 */
static void free_test_qr(qr_code *qr) {
	if (qr) {
		free(qr->matrix);
		free(qr->codewords);
		free(qr);
	}
}

/**
 * @brief Test codeword placement with all bits set to 1
 * 
 * Verifies that when all codeword bits are 1, all non-reserved modules
 * are set to QR_MODULE_DARK after placement.
 * 
 * @return 0 on success, non-zero on failure
 */
TEST(codeword_placement) {
	const size_t size = 21;  // Version 1 QR code
	const size_t num_codewords = 26;  // Version 1-H has 26 codewords
	
	// Create test QR code
	qr_code *qr = create_test_qr(0, size);
	if (!qr) return 1;

	// Initialize all matrix modules to QR_MODULE_LIGHT (0)
	memset(qr->matrix, 0, size * size * sizeof(int));

	// Allocate and initialize test codewords (all bits set to 1)
	qr->codewords = calloc(num_codewords, sizeof(word));
	if (!qr->codewords) {
		free_test_qr(qr);
		return 2;
	}
	qr->codeword_count = num_codewords;
	
	// Set all codeword bits to 1
	memset(qr->codewords, 0xFF, num_codewords * sizeof(word));

	// Place codewords in the matrix
	qr_place_codewords(qr);

	// Verify the results
	for (size_t i = 0; i < size; i++) {
		for (size_t j = 0; j < size; j++) {
			if (qr_module_is_reserved(qr, i, j)) {
				// Reserved modules should remain QR_MODULE_LIGHT (0)
				if (qr_module_get(qr, i, j) != QR_MODULE_LIGHT) {
					free_test_qr(qr);
					return 3;  // Reserved module was modified
				}
			} else {
				// Non-reserved modules should be QR_MODULE_DARK (1)
				if (qr_module_get(qr, i, j) != QR_MODULE_DARK) {
					free_test_qr(qr);
					return 4;  // Non-reserved module not set to dark
				}
			}
		}
	}

	free_test_qr(qr);
	return 0;
}

/**
 * @brief Test module get/set operations
 * 
 * Verifies that module states can be set and retrieved correctly.
 * 
 * @return 0 on success, non-zero on failure
 */
TEST(module_get_set) {
	const size_t size = 21;  // Version 1 QR code
	qr_code *qr = create_test_qr(0, size);
	if (!qr) return 1;

	// Test setting and getting module states
	for (size_t i = 0; i < size; i++) {
		for (size_t j = 0; j < size; j++) {
			// Alternate between dark and light modules
			qr_module_state expected = (i + j) % 2 ? QR_MODULE_DARK : QR_MODULE_LIGHT;
			qr_module_set(qr, i, j, expected);
			qr_module_state actual = qr_module_get(qr, i, j);
			
			if (actual != expected) {
				free_test_qr(qr);
				return 2;
			}
		}
	}

	free_test_qr(qr);
	return 0;
}

/**
 * @brief Test reserved module detection
 * 
 * Verifies that reserved modules (finder patterns, timing patterns, etc.)
 * are correctly identified.
 * 
 * @return 0 on success, non-zero on failure
 */
TEST(reserved_module_detection) {
	const size_t size = 21;  // Version 1 QR code
	qr_code *qr = create_test_qr(0, size);
	if (!qr) return 1;

	// Test finder pattern positions (top-left corner)
	for (int i = 0; i < 7; i++) {
		for (int j = 0; j < 7; j++) {
			if (!qr_module_is_reserved(qr, i, j)) {
				free_test_qr(qr);
				return 2;  // Should be reserved
			}
		}
	}

	// Test timing pattern (row 6, columns 8-12)
	for (int j = 8; j < 13; j++) {
		if (!qr_module_is_reserved(qr, 6, j)) {
			free_test_qr(qr);
			return 3;  // Should be reserved
		}
	}

	// Test format information (top-left corner)
	if (!qr_module_is_reserved(qr, 8, 8)) {  // First format information module
		free_test_qr(qr);
		return 4;  // Should be reserved
	}

	// Test non-reserved area (outside finder patterns and timing patterns)
	if (qr_module_is_reserved(qr, 9, 8)) {  // First data module
		free_test_qr(qr);
		return 5;  // Should not be reserved
	}

	free_test_qr(qr);
	return 0;
}

/**
 * @brief Test edge cases for module access
 * 
 * Verifies that the module access functions handle edge cases correctly.
 * 
 * @return 0 on success, non-zero on failure
 */
TEST(module_edge_cases) {
	const size_t size = 21;  // Version 1 QR code
	qr_code *qr = create_test_qr(0, size);
	if (!qr) return 1;

	// Test setting and getting the last module
	qr_module_set(qr, size-1, size-1, QR_MODULE_DARK);
	if (qr_module_get(qr, size-1, size-1) != QR_MODULE_DARK) {
		free_test_qr(qr);
		return 2;
	}

	// Test that out-of-bounds access doesn't crash
	// (The functions should be safe to call with any indices)
	qr_module_set(qr, size, size, QR_MODULE_DARK);  // Should not crash
	(void)qr_module_get(qr, size, size);			// Should not crash
	(void)qr_module_is_reserved(qr, size, size);	// Should not crash

	free_test_qr(qr);
	return 0;
}
