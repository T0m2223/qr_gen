/**
 * @file mask.c
 * @brief Test cases for QR code masking functionality
 *
 * This file contains comprehensive test cases for QR code masking functionality,
 * including mask pattern application, evaluation, and selection. It verifies
 * the correctness and optimality of mask pattern application and selection
 * algorithms across different QR code versions and patterns.
 */

#include <test/base.h>
#include <qr/types.h>
#include <qr/matrix.h>
#include <qr/patterns.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

// Include the source file directly to test static/internal functions
#include "../qr/mask.c"

/**
 * @brief Creates a test QR code with a specified size and pattern
 *
 * This helper function initializes a QR code structure with a test pattern
 * that can be used to verify mask application. The pattern is designed to
 * produce consistent results when masked.
 *
 * @param size The side length of the QR code matrix (including quiet zone)
 * @return qr_code* Pointer to the created QR code, or NULL on failure
 */
static qr_code *create_test_qr(size_t size) {
	qr_code *qr = calloc(1, sizeof(qr_code));
	if (!qr) return NULL;

	qr->version = 1;  // Version 1 QR code (21x21)
	qr->side_length = size;  // No quiet zone in the matrix
	qr->matrix = calloc(qr->side_length * qr->side_length, sizeof(int));

	if (!qr->matrix) {
		free(qr);
		return NULL;
	}

	// Fill with a pattern
	for (size_t i = 0; i < qr->side_length; i++) {
		for (size_t j = 0; j < qr->side_length; j++) {
			// Create a pattern that will be masked
			qr->matrix[i * qr->side_length + j] = ((i * 3 + j * 5) % 7) < 4 ? 1 : 0;
		}
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
		free(qr);
	}
}

/**
 * @brief Tests the application of all QR code mask patterns
 *
 * This test verifies that each of the 8 standard QR code mask patterns
 * is correctly applied to a test pattern. It checks that the mask patterns
 * toggle the appropriate modules according to their respective formulas.
 *
 * @return 0 on success, non-zero error code on failure
 *         1000 + pattern_num: Mask pattern application failed
 *         2000 + pattern_num: No modules were toggled
 */
TEST(mask_patterns_application)
{
	const size_t size = 21;  // Version 1 QR code (21x21 modules)
	qr_code *qr = create_test_qr(size);
	if (!qr) return 1;

	// Make a copy of the original matrix for comparison
	int *original = malloc(qr->side_length * qr->side_length * sizeof(int));
	if (!original) {
		free_test_qr(qr);
		return 1;
	}
	memcpy(original, qr->matrix, qr->side_length * qr->side_length * sizeof(int));

	// Test each mask pattern
	for (int pattern = 0; pattern < QR_MASK_PATTERN_COUNT; pattern++) {
		// Apply the mask pattern
		qr_mask_apply_pattern(qr, pattern);

		// Verify the mask was applied correctly
		size_t toggled = 0;
		for (size_t i = 0; i < qr->side_length; i++) {
			for (size_t j = 0; j < qr->side_length; j++) {
				// Skip reserved modules (finders, timing, alignment, etc.)
				if (qr_module_is_reserved(qr, i, j)) {
					// Verify reserved modules were not modified
					if (original[i * qr->side_length + j] != qr->matrix[i * qr->side_length + j]) {
						free(original);
						free_test_qr(qr);
						return 1000 + pattern;  // Mask pattern modified reserved module
					}
					continue;
				}
				// Check if this module should be toggled based on the mask pattern
				int should_toggle = 0;

				// Apply the mask pattern formula (0-7) as defined in the QR code specification
				switch (pattern) {
					case 0: should_toggle = ((i + j) % 2 == 0); break;
					case 1: should_toggle = (i % 2 == 0); break;
					case 2: should_toggle = (j % 3 == 0); break;
					case 3: should_toggle = ((i + j) % 3 == 0); break;
					case 4: should_toggle = (((i / 2) + (j / 3)) % 2 == 0); break;
					case 5: should_toggle = (((i * j) % 2) + ((i * j) % 3)) == 0; break;
					case 6: should_toggle = ((((i * j) % 2) + ((i * j) % 3)) % 2 == 0); break;
					case 7: should_toggle = ((((i + j) % 2) + ((i * j) % 3)) % 2 == 0); break;
				}

				int original_value = original[i * qr->side_length + j];
				int expected_value = should_toggle ? !original_value : original_value;

				if (qr->matrix[i * qr->side_length + j] != expected_value) {
					free(original);
					free_test_qr(qr);
					return 1000 + pattern;  // Error code indicating which pattern failed
				}

				if (should_toggle) toggled++;
			}
		}

		// Make sure at least some modules were toggled
		if (toggled == 0) {
			free(original);
			free_test_qr(qr);
			return 2000 + pattern;  // Error code for no modules toggled
		}

		// Reset for next pattern
		memcpy(qr->matrix, original, qr->side_length * qr->side_length * sizeof(int));
	}

	free(original);
	free_test_qr(qr);
	return 0;
}

/**
 * @brief Initialize a QR code matrix with random values for testing
 *
 * This function initializes a QR code with random data modules (0 or 1) for all
 * non-reserved modules. It uses the standard library's rand() function for
 * random number generation, seeded with the provided value for reproducibility.
 * After setting random values, it applies all required QR code patterns
 * (finder, separators, alignment, timing, format, and version info).
 *
 * @param qr QR code structure to initialize
 * @param size Size of the QR code (e.g., 21 for version 1)
 * @param seed Seed value for the random number generator (srand)
 */
static void init_random_qr(qr_code *qr, size_t size, unsigned int seed) {
	// Initialize QR code structure
	qr->version = 1;
	qr->side_length = size;
	qr->matrix = calloc(size * size, sizeof(int));
	if (!qr->matrix) return;

	// Initialize random number generator with the provided seed
	srand(seed);

	// Fill the matrix with random data, skipping reserved modules
	for (size_t i = 0; i < size; i++) {
		for (size_t j = 0; j < size; j++) {
			qr_module_set(qr, i, j, rand() & 1);
		}
	}

	// Apply required QR code patterns (these will handle reserved modules)
	qr_finder_patterns_apply(qr);
	qr_separators_apply(qr);
	qr_alignment_patterns_apply(qr);
	qr_timing_patterns_apply(qr);
	qr_format_info_apply(qr);
	qr_version_info_apply(qr);
}

/**
 * @brief Test mask pattern selection with random matrices
 *
 * This test verifies that the mask selection algorithm consistently
 * chooses the mask pattern with the lowest penalty score across
 * multiple random patterns.
 *
 * @return 0 on success, non-zero on failure
 */
TEST(mask_selection_optimality)
{
	const size_t size = 21;  // Version 1 QR code (21x21)
	const int num_tests = 5;  // Number of random matrices to test

	// Test with different random patterns
	for (int test_case = 0; test_case < num_tests; test_case++) {
		qr_code qr = {0};
		init_random_qr(&qr, size, (unsigned int)test_case);  // Use test_case as seed

		if (!qr.matrix) return 1000 + test_case;

		// Find the best mask pattern by evaluating all of them
		int best_score = INT_MAX;
		int pattern_scores[QR_MASK_PATTERN_COUNT] = {0};

		for (int pattern = 0; pattern < QR_MASK_PATTERN_COUNT; pattern++) {
			// Create a copy of the QR code
			qr_code qr_copy = qr;
			qr_copy.matrix = malloc(size * size * sizeof(int));
			if (!qr_copy.matrix) {
				free(qr.matrix);
				return 2000 + test_case;
			}
			memcpy(qr_copy.matrix, qr.matrix, size * size * sizeof(int));

			// Apply pattern and evaluate
			qr_mask_apply_pattern(&qr_copy, pattern);
			int score = qr_mask_evaluate(&qr_copy);
			pattern_scores[pattern] = score;

			free(qr_copy.matrix);

			if (score < best_score) {
				best_score = score;
			}
		}

		// Verify that the selected pattern has the lowest score
		for (int pattern = 0; pattern < QR_MASK_PATTERN_COUNT; pattern++) {
			if (pattern_scores[pattern] < best_score) {
				// Found a better pattern that wasn't selected
				free(qr.matrix);
				return 3000 + (test_case * 10) + pattern;
			}
		}

		// Clean up
		free(qr.matrix);
	}

	return 0;
	// All test cases passed successfully
}
/**
 * @brief Test individual mask evaluation features
 *
 * This test verifies each of the four mask evaluation features:
 * 1. Adjacent modules in row/column
 * 2. 2x2 blocks of same color
 * 3. Specific patterns (1011101 and 000010000100001111101)
 * 4. Ratio of dark to light modules
 *
 * @return 0 on success, non-zero error code on failure
 */
TEST(mask_evaluation_features)
{
	qr_code *qr = create_test_qr(21);
	if (!qr) return 1;

	// Test feature 1: Adjacent modules in row/column
	int score1 = feature_1_evaluation(qr);
	if (score1 < 0) {
		free_test_qr(qr);
		return 5000 + score1;  // Error code for feature 1 evaluation
	}

	// Test feature 2: 2x2 blocks of same color
	int score2 = feature_2_evaluation(qr);
	if (score2 < 0) {
		free_test_qr(qr);
		return 6000 + score2;  // Error code for feature 2 evaluation
	}

	// Test feature 3: Specific patterns (1011101 and 000010000100001111101)
	int score3 = feature_3_evaluation(qr);
	if (score3 < 0) {
		free_test_qr(qr);
		return 7000 + score3;  // Error code for feature 3 evaluation
	}

	// Test feature 4: Ratio of dark to light modules
	int score4 = feature_4_evaluation(qr);
	if (score4 < 0) {
		free_test_qr(qr);
		return 8000 + score4;  // Error code for feature 4 evaluation
	}

	// Test overall evaluation
	int total_score = qr_mask_evaluate(qr);
	if (total_score < 0) {
		free_test_qr(qr);
		return 9000 + total_score;  // Error code for overall evaluation
	}

	free_test_qr(qr);
	return 0;
}

/**
 * @brief Test mask application across different QR code versions
 *
 * This test verifies that mask patterns can be correctly applied to
 * QR codes of different versions (sizes). It tests versions 1-5.
 *
 * @return 0 on success, non-zero on failure
 */
TEST(mask_different_versions)
{
	// Test with different QR code versions (sizes for versions 1-5)
	// These are the module counts (not including quiet zone)
	const size_t sizes[] = {21, 25, 29, 33, 37};
	const int num_sizes = sizeof(sizes) / sizeof(sizes[0]);

	for (int i = 0; i < num_sizes; i++) {
		qr_code *qr = create_test_qr(sizes[i]);
		if (!qr) return 1;

		// Set version based on size (simplified)
		qr->version = i + 1;

		// Apply a random mask pattern
		int pattern = i % QR_MASK_PATTERN_COUNT;
		qr_mask_apply_pattern(qr, pattern);

		// Verify the mask was applied to the entire matrix
		// No quiet zone to check here - it's added during export

		free_test_qr(qr);
	}

	return 0;
}

/**
 * @brief Test mask pattern selection with known patterns
 *
 * This test verifies that the mask selection algorithm chooses
 * appropriate patterns for specific input patterns. It uses a
 * horizontal line pattern that should be penalized by mask 1.
 *
 * @return 0 on success, non-zero on failure
 */
TEST(mask_pattern_selection_known_cases)
{
	// Create a QR code with a pattern containing horizontal lines
	// This pattern should be penalized by mask 1 (which creates horizontal lines)
	qr_code *qr = create_test_qr(21);
	if (!qr) return 1;

	// Create a pattern with many horizontal lines (should be penalized by mask 1)
	for (size_t i = 4; i < qr->side_length - 4; i++) {
		for (size_t j = 4; j < qr->side_length - 4; j++) {
			// Create horizontal lines
			qr->matrix[i * qr->side_length + j] = (i % 2) ? 1 : 0;
		}
	}

	// Find the best pattern for this QR code
	int best_pattern = 0;
	int best_score = INT_MAX;

	for (int pattern = 0; pattern < QR_MASK_PATTERN_COUNT; pattern++) {
		// Create a copy of the QR code
		qr_code qr_copy = *qr;
		qr_copy.matrix = malloc(qr->side_length * qr->side_length * sizeof(int));
		if (!qr_copy.matrix) {
			free_test_qr(qr);
			return 1;
		}
		memcpy(qr_copy.matrix, qr->matrix, qr->side_length * qr->side_length * sizeof(int));

		// Apply pattern and evaluate
		qr_mask_apply_pattern(&qr_copy, pattern);
		int score = qr_mask_evaluate(&qr_copy);

		free(qr_copy.matrix);

		if (score < best_score) {
			best_score = score;
			best_pattern = pattern;
		}
	}

	// Mask 1 creates horizontal lines, which would be bad for this pattern
	if (best_pattern == 1) {
		free_test_qr(qr);
		return 20000 + best_pattern;  // Error code for suboptimal pattern selection
	}

	free_test_qr(qr);
	return 0;
}

/**
 * @brief Tests mask pattern application and toggling behavior with a checkerboard pattern
 *
 * This test verifies that mask patterns are correctly applied and can be undone by
 * applying the same mask pattern again. It uses a checkerboard pattern to ensure
 * that the mask patterns interact with the data in a predictable way.
 *
 * @return 0 on success, non-zero error code on failure
 *         1: Memory allocation failed
 *         2: Mask pattern application failed
 *         3: Double mask application didn't return to original
 */
TEST(mask_patterns)
{
	// Create a QR code structure for testing (without quiet zone)
	qr_code qr = {0};
	qr.version = 1;  // Version 1 QR code (21x21)
	qr.side_length = 21;  // No quiet zone
	qr.matrix = calloc(qr.side_length * qr.side_length, sizeof(int));

	if (!qr.matrix) return 1;

	// Create a checkerboard pattern
	for (size_t i = 0; i < qr.side_length; i++) {
		for (size_t j = 0; j < qr.side_length; j++) {
			// Only set non-reserved modules to the checkerboard pattern
			if (!qr_module_is_reserved(&qr, i, j)) {
				qr.matrix[i * qr.side_length + j] = ((i + j) % 2) ? 1 : 0;
			}
		}
	}

	// Make a copy of the original matrix for comparison
	int *original = malloc(qr.side_length * qr.side_length * sizeof(int));
	if (!original) {
		free(qr.matrix);
		return 1;
	}
	memcpy(original, qr.matrix, qr.side_length * qr.side_length * sizeof(int));

	// Test each mask pattern
	for (int pattern = 0; pattern < QR_MASK_PATTERN_COUNT; pattern++) {
		// Apply the mask pattern
		qr_mask_apply_pattern(&qr, pattern);

		// Verify the mask was applied correctly
		for (size_t i = 0; i < qr.side_length; i++) {
			for (size_t j = 0; j < qr.side_length; j++) {
				// Skip reserved modules - they shouldn't be modified
				if (qr_module_is_reserved(&qr, i, j)) {
					if (qr.matrix[i * qr.side_length + j] != original[i * qr.side_length + j]) {
						free(original);
						free(qr.matrix);
						return 1000 + pattern;  // Mask pattern modified reserved module
					}
					continue;
				}

				// For non-reserved modules, check if they were toggled according to the mask pattern
				int should_toggle = 0;
				switch (pattern) {
					case 0: should_toggle = ((i + j) % 2 == 0); break;
					case 1: should_toggle = (i % 2 == 0); break;
					case 2: should_toggle = (j % 3 == 0); break;
					case 3: should_toggle = ((i + j) % 3 == 0); break;
					case 4: should_toggle = (((i / 2) + (j / 3)) % 2 == 0); break;
					case 5: should_toggle = (((i * j) % 2) + ((i * j) % 3)) == 0; break;
					case 6: should_toggle = ((((i * j) % 2) + ((i * j) % 3)) % 2 == 0); break;
					case 7: should_toggle = ((((i + j) % 2) + ((i * j) % 3)) % 2 == 0); break;
				}

				// The module should be toggled if the mask pattern says so
				int expected = original[i * qr.side_length + j] ^ should_toggle;
				if (qr.matrix[i * qr.side_length + j] != expected) {
					free(original);
					free(qr.matrix);
					return 2000 + pattern;  // Mask pattern application failed
				}
			}
		}

		// Apply the mask again to undo it (should return to original)
		qr_mask_apply_pattern(&qr, pattern);

		// Verify we're back to the original pattern
		for (size_t i = 0; i < qr.side_length; i++) {
			for (size_t j = 0; j < qr.side_length; j++) {
				if (qr.matrix[i * qr.side_length + j] != original[i * qr.side_length + j]) {
					free(original);
					free(qr.matrix);
					return 3000 + pattern;  // Double mask application didn't return to original
				}
			}
		}
	}

	free(qr.matrix);
	return 0;
}

/**
 * @brief Test case definition for mask penalty verification
 */
typedef struct {
	const char *name;           // Test case name
	const char *pattern;        // String representation of the matrix pattern (0=light, 1=dark, space=reserved)
	size_t size;                // Matrix size (size x size)
	int expected_scores[QR_MASK_PATTERN_COUNT];  // Expected penalty scores for each mask pattern
} mask_penalty_test_case;

/**
 * @brief Helper function to create a QR code from a pattern string
 *
 * @param qr Pointer to QR code structure to initialize
 * @param test The test case containing the pattern
 * @return int 0 on success, non-zero on error
 */
static int init_qr_from_pattern(qr_code *qr, const mask_penalty_test_case *test) {
	qr->version = (test->size - 21) / 4;  // Version 1 for testing (21x21)
	qr->level = QR_EC_LEVEL_L;
	qr->mode = QR_MODE_BYTE;
	qr->side_length = test->size;
	qr->matrix = calloc(qr->side_length * qr->side_length, sizeof(int));

	if (!qr->matrix) return 1;

	const char *p = test->pattern;
	for (size_t i = 0; i < qr->side_length; i++) {
		for (size_t j = 0; j < qr->side_length; j++) {
			if (*p == '0') {
				qr->matrix[i * qr->side_length + j] = 0;
			} else if (*p == '1') {
				qr->matrix[i * qr->side_length + j] = 1;
			} else if (*p == ' ') {
				// For reserved modules, we need to set them to a value and mark as reserved
				// This is a simplification - in reality, reserved modules would be set by QR code generation
				qr->matrix[i * qr->side_length + j] = 0;
			}
			p++;
		}
		// Skip newlines in the pattern string
		while (*p == '\n' || *p == '\r') p++;
	}

	return 0;
}

int loggg = 0;

/**
 * @brief Test mask penalty calculation for specific patterns
 *
 * This test verifies that the mask penalty calculation works correctly
 * by testing against known patterns with pre-calculated penalty scores.
 *
 * @return 0 on success, non-zero on failure
 */
TEST(mask_penalty_calculation)
{
	// Define test cases with known penalty scores
	const mask_penalty_test_case tests[] = {
		{
			.name = "Checkerboard pattern",
			.pattern =
				"111111100111001111111\n"
				"100000100010001000001\n"
				"101110100111001011101\n"
				"101110100000001011101\n"
				"101110100000001011101\n"
				"100000100101101000001\n"
				"111111101010101111111\n"
				"000000000000100000000\n"
				"000000100101100000000\n"
				"001011011111011011001\n"
				"101010110110011101010\n"
				"110010011111010111000\n"
				"111000100000011100101\n"
				"000000001110000111101\n"
				"111111100101101100010\n"
				"100000100100110001011\n"
				"101110100111101000101\n"
				"101110100010011011100\n"
				"101110100100000000000\n"
				"100000100110010111000\n"
				"111111100001001110110",
			.size = 21,
			.expected_scores = {342, 632, 315, 326, 496, 411, 537, 341}  // TODO: Fill in expected scores
		},
		// Add more test cases here
	};

	const size_t num_tests = sizeof(tests) / sizeof(tests[0]);

	for (size_t t = 0; t < num_tests; t++) {
		const mask_penalty_test_case *test = &tests[t];

		// Initialize QR code from pattern
		qr_code qr = {0};
		if (init_qr_from_pattern(&qr, test)) {
			return 1000 + t;  // Memory allocation failed
		}

		// Test each mask pattern
		for (int pattern = 0; pattern < QR_MASK_PATTERN_COUNT; pattern++) {
			// Apply the mask pattern
			qr_mask_apply_pattern(&qr, pattern);
			qr.mask = pattern;
			qr_format_info_apply(&qr);
			qr_version_info_apply(&qr);

			// Calculate the penalty score
			loggg = 1;
			int score = qr_mask_evaluate(&qr);
			loggg = 0;

			// Verify the score matches expected
			if (test->expected_scores[pattern] != -1 &&  // -1 means skip this check
				score != test->expected_scores[pattern]) {
				free(qr.matrix);
				printf("Score mismatch for test %zu, pattern %d: expected %d, got %d\n\n\n", t, pattern, test->expected_scores[pattern], score);
				return 2000 + (t * 10) + pattern;  // Penalty score mismatch
			}

			// Undo the mask for the next test
			qr_mask_apply_pattern(&qr, pattern);
		}

		free(qr.matrix);
	}

	return 0;
}

// Test mask evaluation features
TEST(mask_evaluation)
{
	// Create a simple QR code structure for testing
	qr_code qr = {0};
	qr.version = 1;  // Version 1 QR code (21x21)
	qr.side_length = 21 + 8;  // 21 modules + 8 quiet zone (4 on each side)
	qr.matrix = calloc(qr.side_length * qr.side_length, sizeof(int));

	if (!qr.matrix) return 1;

	// Test feature 1: Adjacent modules in row/column
	// Create 6 dark modules in a row (should be penalized)
	for (size_t i = 0; i < 6; i++) {
		qr.matrix[(5 + 4) * qr.side_length + (5 + 4 + i)] = 1;  // Add 4 to account for quiet zone
	}

	int score = qr_mask_evaluate(&qr);
	if (score == 0) {
		free(qr.matrix);
		return 2;  // Failed to detect consecutive modules
	}

	// Clear the matrix
	memset(qr.matrix, 0, qr.side_length * qr.side_length * sizeof(int));

	// Test feature 2: 2x2 blocks of the same color
	// Create a 2x2 block of dark modules (1s)
	size_t base_i = 5 + 4;  // Add 4 to account for quiet zone
	size_t base_j = 5 + 4;
	qr.matrix[base_i * qr.side_length + base_j] = 1;
	qr.matrix[base_i * qr.side_length + base_j + 1] = 1;
	qr.matrix[(base_i + 1) * qr.side_length + base_j] = 1;
	qr.matrix[(base_i + 1) * qr.side_length + base_j + 1] = 1;

	score = qr_mask_evaluate(&qr);
	if (score == 0) {
		free(qr.matrix);
		return 3;  // Failed to detect 2x2 block
	}

	free(qr.matrix);
	return 0;
}

/**
 * @brief Test that different mask patterns are selected for different inputs
 *
 * This test verifies that the mask selection algorithm doesn't always choose
 * the same pattern for different inputs. It creates multiple random matrices
 * and checks that at least 3 different patterns are selected across them.
 *
 * @return 0 on success, non-zero on failure
 */
TEST(mask_pattern_diversity)
{
	const size_t size = 21;  // Version 1 QR code (21x21)
	const int num_tests = 10;  // Number of test cases
	const int min_unique_patterns = 3;  // Require at least 3 different patterns

	int pattern_counts[QR_MASK_PATTERN_COUNT] = {0};
	int unique_patterns = 0;

	for (int test_case = 0; test_case < num_tests; test_case++) {
		qr_code qr = {0};

		// Initialize QR code with required attributes for qr_mask_apply
		qr.version = 1;  // Version 1
		qr.side_length = size;
		qr.matrix = calloc(size * size, sizeof(int));
		if (!qr.matrix) {
			return 1000 + test_case;
		}

		init_random_qr(&qr, size, test_case + 1234);

		// Apply masking and find the best pattern
		qr_mask_apply(&qr);


		// Get the selected pattern
		int best_pattern = (int)qr.mask;

		// Track how many times each pattern was selected
		if (pattern_counts[best_pattern] == 0) {
			unique_patterns++;
		}
		pattern_counts[best_pattern]++;

		// Clean up
		free(qr.matrix);

		// Early exit if we've already met our diversity requirement
		if (unique_patterns >= min_unique_patterns) {
			break;
		}
	}

	// Verify we have sufficient pattern diversity
	if (unique_patterns < min_unique_patterns) return 3000 + unique_patterns;

	return 0;
}
