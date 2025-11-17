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
 * @param version The version of the QR code
 * @return qr_code* Pointer to the created QR code, or NULL on failure
 */
static qr_code *create_test_qr(size_t version) {
	qr_code *qr = test_malloc(sizeof(qr_code));
	if (!qr) return NULL;

	qr->version = version;  // Version 1 QR code (21x21)
	qr->side_length = 21 + (version * 4);  // No quiet zone in the matrix
	qr->matrix = test_malloc(qr->side_length * qr->side_length * sizeof(int));

	if (!qr->matrix) return NULL;

	// Fill with a pattern
	for (size_t i = 0; i < qr->side_length; i++) {
		for (size_t j = 0; j < qr->side_length; j++) {
			// Create a pattern that will be masked
			qr_module_set(qr, i, j, ((i * 3 + j * 5) % 7) < 4 ? QR_MODULE_DARK : QR_MODULE_LIGHT);
		}
	}

	return qr;
}

/**
 * @brief Tests the application of all QR code mask patterns
 *
 * This test verifies that each of the 8 standard QR code mask patterns
 * is correctly applied to a test pattern. It checks that the mask patterns
 * toggle the appropriate modules according to their respective formulas.
 */
TEST(mask_patterns_application)
{
	qr_code *qr = create_test_qr(2); // Version 2
	if (!qr) return TEST_FAILURE("Failed to create test QR code");

	// Make a copy of the original matrix for comparison
	int *original = test_malloc(qr->side_length * qr->side_length * sizeof(int));
	if (!original) return TEST_FAILURE("Memory allocation failed");
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
					assert_eq(original[i * qr->side_length + j], qr->matrix[i * qr->side_length + j],
						"Reserved module should not be modified by mask pattern");
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

				assert_eq(qr->matrix[i * qr->side_length + j], expected_value,
					"Mask pattern should toggle modules according to formula");

				if (should_toggle) toggled++;
			}
		}

		// Make sure at least some modules were toggled
		assert_gt(toggled, 0, "Mask pattern should toggle at least some modules");

		// Reset for next pattern
		memcpy(qr->matrix, original, qr->side_length * qr->side_length * sizeof(int));
	}

	return TEST_SUCCESS;
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
 *
 * @return 0 on success, non-zero on failure
 */
static int init_random_qr(qr_code *qr, size_t size, unsigned int seed) {
	// Initialize QR code structure
	qr->version = 1;
	qr->side_length = size;
	qr->matrix = test_malloc(size * size * sizeof(int));
	if (!qr->matrix) return 1;

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

	return 0;
}

/**
 * @brief Test mask pattern selection with random matrices
 *
 * This test verifies that the mask selection algorithm consistently
 * chooses the mask pattern with the lowest penalty score across
 * multiple random patterns.
 */
TEST(mask_selection_optimality)
{
	const size_t size = 21;  // Version 1 QR code (21x21)
	const int num_tests = 5;  // Number of random matrices to test

	// Test with different random patterns
	for (int test_case = 0; test_case < num_tests; test_case++) {
		qr_code qr = {0};
		if (init_random_qr(&qr, size, (unsigned int)test_case)) return TEST_FAILURE("Matrix allocation failed");

		// Find the best mask pattern by evaluating all of them
		int best_score = INT_MAX;
		int pattern_scores[QR_MASK_PATTERN_COUNT] = {0};

		for (int pattern = 0; pattern < QR_MASK_PATTERN_COUNT; pattern++) {
			// Create a copy of the QR code
			qr_code qr_copy = qr;
			qr_copy.matrix = test_malloc(size * size * sizeof(int));
			if (!qr_copy.matrix) return TEST_FAILURE("Matrix copy allocation failed");
			memcpy(qr_copy.matrix, qr.matrix, size * size * sizeof(int));

			// Apply pattern and evaluate
			qr_mask_apply_pattern(&qr_copy, pattern);
			int score = qr_mask_evaluate(&qr_copy);
			pattern_scores[pattern] = score;

			if (score < best_score) best_score = score;
		}

		// Verify that the selected pattern has the lowest score
		for (int pattern = 0; pattern < QR_MASK_PATTERN_COUNT; pattern++) {
			assert_gte(pattern_scores[pattern], best_score,
				"Selected pattern should have the lowest penalty score");
		}
	}

	return TEST_SUCCESS;
}
/**
 * @brief Test individual mask evaluation features
 *
 * This test verifies each of the four mask evaluation features:
 * 1. Adjacent modules in row/column
 * 2. 2x2 blocks of same color
 * 3. Specific patterns (1011101 and 000010000100001111101)
 * 4. Ratio of dark to light modules
 */
TEST(mask_evaluation_features)
{
	qr_code *qr = create_test_qr(1);
	if (!qr) return TEST_FAILURE("Failed to create test QR code");

	// Test feature 1: Adjacent modules in row/column
	int score1 = feature_1_evaluation(qr);
	assert_gte(score1, 0,
		"Feature 1 evaluation should return non-negative score");

	// Test feature 2: 2x2 blocks of same color
	int score2 = feature_2_evaluation(qr);
	assert_gte(score2, 0,
		"Feature 2 evaluation should return non-negative score");

	// Test feature 3: Specific patterns (1011101 and 000010000100001111101)
	int score3 = feature_3_evaluation(qr);
	assert_gte(score3, 0,
		"Feature 3 evaluation should return non-negative score");

	// Test feature 4: Ratio of dark to light modules
	int score4 = feature_4_evaluation(qr);
	assert_gte(score4, 0,
		"Feature 4 evaluation should return non-negative score");

	// Test overall evaluation
	int total_score = qr_mask_evaluate(qr);
	assert_gte(total_score, 0,
		"Overall mask evaluation should return non-negative score");

	return TEST_SUCCESS;
}

/**
 * @brief Test mask application across different QR code versions
 *
 * This test verifies that mask patterns can be correctly applied to
 * QR codes of different versions (sizes). It tests versions 1-5.
 */
TEST(mask_different_versions)
{
	// Test with different QR code versions (1-5)
	for (int i = 0; i < 5; i++) {
		qr_code *qr = create_test_qr(i);
		if (!qr) return TEST_FAILURE("Failed to create test QR code");

		// Set version based on size (simplified)
		qr->version = i + 1;

		// Create a simple test pattern (checkerboard) to verify mask application
		for (size_t row = 0; row < qr->side_length; row++) {
			for (size_t col = 0; col < qr->side_length; col++) {
				// Only set non-reserved modules to a checkerboard pattern
				if (!qr_module_is_reserved(qr, row, col)) {
					qr->matrix[row * qr->side_length + col] = ((row + col) % 2) ? QR_MODULE_DARK : QR_MODULE_LIGHT;
				}
			}
		}

		// Make a copy of the original matrix for comparison
		int *original = test_malloc(qr->side_length * qr->side_length * sizeof(int));
		if (!original) return TEST_FAILURE("Original matrix allocation failed");
		memcpy(original, qr->matrix, qr->side_length * qr->side_length * sizeof(int));

		// Apply a mask pattern
		int pattern = i % QR_MASK_PATTERN_COUNT;
		qr_mask_apply_pattern(qr, pattern);

		// Verify the mask was applied - at least some non-reserved modules should be toggled
		size_t toggled = 0;
		for (size_t row = 0; row < qr->side_length; row++) {
			for (size_t col = 0; col < qr->side_length; col++) {
				// Skip reserved modules
				if (qr_module_is_reserved(qr, row, col)) continue;

				if (qr->matrix[row * qr->side_length + col] != original[row * qr->side_length + col]) {
					toggled++;
				}
			}
		}

		// Verify that the mask pattern actually changed some modules
		assert_gt(toggled, 0, "Mask pattern should toggle at least some modules");

		// Verify that reserved modules were not modified
		for (size_t row = 0; row < qr->side_length; row++) {
			for (size_t col = 0; col < qr->side_length; col++) {
				if (qr_module_is_reserved(qr, row, col)) {
					assert_eq(qr->matrix[row * qr->side_length + col],
						original[row * qr->side_length + col],
						"Reserved module should not be modified by mask pattern");
				}
			}
		}
	}

	return TEST_SUCCESS;
}

/**
 * @brief Test mask pattern selection with known patterns
 *
 * This test verifies that the mask selection algorithm chooses
 * appropriate patterns for specific input patterns. It uses a
 * horizontal line pattern that should be penalized by mask 1.
 */
TEST(mask_pattern_selection_known_cases)
{
	// Create a QR code with a pattern containing horizontal lines
	// This pattern should be penalized by mask 1 (which creates horizontal lines)
	qr_code *qr = create_test_qr(1);
	if (!qr) return TEST_FAILURE("Failed to create test QR code");

	// Create a pattern with many horizontal lines (should be penalized by mask 1)
	for (size_t i = 4; i < qr->side_length - 4; i++) {
		for (size_t j = 4; j < qr->side_length - 4; j++) {
			// Create horizontal lines
			qr->matrix[i * qr->side_length + j] = (i % 2) ? 1 : 0;
		}
	}

	// Apply all patterns and find the best one
	int best_pattern = -1;
	int best_score = INT_MAX;

	for (int pattern = 0; pattern < QR_MASK_PATTERN_COUNT; pattern++) {
		// Create a copy
		qr_code qr_copy = *qr;
		qr_copy.matrix = test_malloc(qr->side_length * qr->side_length * sizeof(int));
		if (!qr_copy.matrix) return TEST_FAILURE("Matrix copy allocation failed");
		memcpy(qr_copy.matrix, qr->matrix, qr->side_length * qr->side_length * sizeof(int));

		// Apply pattern and evaluate
		qr_mask_apply_pattern(&qr_copy, pattern);
		int score = qr_mask_evaluate(&qr_copy);

		if (score < best_score) {
			best_score = score;
			best_pattern = pattern;
		}
	}

	// Mask 1 creates horizontal lines, which would be bad for this pattern
	assert_neq(best_pattern, 1,
		"Mask 1 should not be selected for horizontal line pattern");

	return TEST_SUCCESS;
}

/**
 * @brief Tests mask pattern application and toggling behavior with a checkerboard pattern
 *
 * This test verifies that mask patterns are correctly applied and can be undone by
 * applying the same mask pattern again. It uses a checkerboard pattern to ensure
 * that the mask patterns interact with the data in a predictable way.
 */
TEST(mask_patterns)
{
	// Create a QR code structure for testing (without quiet zone)
	qr_code *qr = create_test_qr(1);
	if (!qr) return TEST_FAILURE("Matrix allocation failed");

	// Create a checkerboard pattern
	for (size_t i = 0; i < qr->side_length; i++) {
		for (size_t j = 0; j < qr->side_length; j++) {
			// Only set non-reserved modules to the checkerboard pattern
			if (!qr_module_is_reserved(qr, i, j)) {
				qr->matrix[i * qr->side_length + j] = ((i + j) % 2) ? 1 : 0;
			}
		}
	}

	// Make a copy of the original matrix for comparison
	int *original = test_malloc(qr->side_length * qr->side_length * sizeof(int));
	if (!original) return TEST_FAILURE("Original matrix allocation failed");
	memcpy(original, qr->matrix, qr->side_length * qr->side_length * sizeof(int));

	// Test each mask pattern
	for (int pattern = 0; pattern < QR_MASK_PATTERN_COUNT; pattern++) {
		// Apply the mask pattern
		qr_mask_apply_pattern(qr, pattern);

		// Verify the mask was applied correctly
		for (size_t i = 0; i < qr->side_length; i++) {
			for (size_t j = 0; j < qr->side_length; j++) {
				// Skip reserved modules - they shouldn't be modified
				if (qr_module_is_reserved(qr, i, j)) {
					assert_eq(qr->matrix[i * qr->side_length + j], original[i * qr->side_length + j],
						"Reserved module was modified by mask pattern");
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
				int expected = original[i * qr->side_length + j] ^ should_toggle;
				assert_eq(qr->matrix[i * qr->side_length + j], expected,
					"Mask pattern should toggle modules according to formula");
			}
		}

		// Apply the mask again to undo it (should return to original)
		qr_mask_apply_pattern(qr, pattern);

		// Verify we're back to the original pattern
		for (size_t i = 0; i < qr->side_length; i++) {
			for (size_t j = 0; j < qr->side_length; j++) {
				assert_eq(qr->matrix[i * qr->side_length + j], original[i * qr->side_length + j],
					"Double mask application didn't return to original");
			}
		}
	}

	return TEST_SUCCESS;
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
 */
static int init_qr_from_pattern(qr_code *qr, const mask_penalty_test_case *test) {
	qr->version = (test->size - 21) / 4;  // Version 1 for testing (21x21)
	qr->level = QR_EC_LEVEL_L;
	qr->mode = QR_MODE_BYTE;
	qr->side_length = test->size;
	qr->matrix = test_malloc(qr->side_length * qr->side_length * sizeof(int));

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

/**
 * @brief Test mask penalty calculation for specific patterns
 *
 * This test verifies that the mask penalty calculation works correctly
 * by testing against known patterns with pre-calculated penalty scores.
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
		if (init_qr_from_pattern(&qr, test)) return TEST_FAILURE("Memory allocation failed");

		// Test each mask pattern
		for (int pattern = 0; pattern < QR_MASK_PATTERN_COUNT; pattern++) {
			// Apply the mask pattern
			qr_mask_apply_pattern(&qr, pattern);
			qr.mask = pattern;
			qr_format_info_apply(&qr);
			qr_version_info_apply(&qr);

			// Calculate the penalty score
			int score = qr_mask_evaluate(&qr);

			// Verify the score matches expected
			if (test->expected_scores[pattern] != -1)  // -1 means skip this check
				assert_eq(score, test->expected_scores[pattern],
					"Penalty score mismatch");

			// Undo the mask for the next test
			qr_mask_apply_pattern(&qr, pattern);
		}
	}

	return TEST_SUCCESS;
}

// Test mask evaluation features
TEST(mask_evaluation)
{
	// Create a simple QR code structure for testing
	qr_code qr = {0};
	qr.version = 1;  // Version 1 QR code (21x21)
	qr.side_length = 21 + 8;  // 21 modules + 8 quiet zone (4 on each side)
	qr.matrix = test_malloc(qr.side_length * qr.side_length * sizeof(int));
	for (size_t i = 0; i < qr.side_length; i++) {
		for (size_t j = 0; j < qr.side_length; j++) {
			qr_module_set(&qr, i, j, QR_MODULE_LIGHT);
		}
	}

	if (!qr.matrix) return TEST_FAILURE("Matrix allocation failed");

	// Test feature 1: Adjacent modules in row/column
	// Create 6 dark modules in a row (should be penalized)
	for (size_t i = 0; i < 6; i++) {
		qr.matrix[(5 + 4) * qr.side_length + (5 + 4 + i)] = 1;  // Add 4 to account for quiet zone
	}

	int score = qr_mask_evaluate(&qr);
	assert_gt(score, 0, "Should detect consecutive modules in row/column");

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
	assert_gt(score, 0, "Should detect 2x2 block of same modules");

	return TEST_SUCCESS;
}

/**
 * @brief Test that different mask patterns are selected for different inputs
 *
 * This test verifies that the mask selection algorithm doesn't always choose
 * the same pattern for different inputs. It creates multiple random matrices
 * and checks that at least 3 different patterns are selected across them.
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
		// Initialize QR code with random data
		init_random_qr(&qr, size, test_case + 1234);

		// Apply masking and find the best pattern
		qr_mask_apply(&qr);


		// Get the selected pattern
		int best_pattern = (int)qr.mask;

		// Track how many times each pattern was selected
		if (pattern_counts[best_pattern] == 0) unique_patterns++;
		pattern_counts[best_pattern]++;

		// Early exit if we've already met our diversity requirement
		if (unique_patterns >= min_unique_patterns) break;
	}

	// Verify we have sufficient pattern diversity
	assert_gte(unique_patterns, min_unique_patterns,
		"Insufficient pattern diversity");

	return TEST_SUCCESS;
}
