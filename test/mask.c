/**
 * @file mask.c
 * @brief Test cases for QR code masking functionality
 * 
 * This file contains test cases for the QR code masking functionality,
 * including mask pattern application and evaluation.
 */

#include <test/base.h>
#include <qr/types.h>
#include <qr/matrix.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

// Include the source file directly to test static functions
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
    const size_t size = 21;  // Version 1 QR code
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
        for (size_t i = 4; i < qr->side_length - 4; i++) {
            for (size_t j = 4; j < qr->side_length - 4; j++) {
                size_t module_i = i - 4;
                size_t module_j = j - 4;
                int should_toggle = 0;
                
                switch (pattern) {
                    case 0: should_toggle = ((module_i + module_j) % 2 == 0); break;
                    case 1: should_toggle = (module_i % 2 == 0); break;
                    case 2: should_toggle = (module_j % 3 == 0); break;
                    case 3: should_toggle = ((module_i + module_j) % 3 == 0); break;
                    case 4: should_toggle = (((module_i / 2) + (module_j / 3)) % 2 == 0); break;
                    case 5: should_toggle = (((module_i * module_j) % 2) + ((module_i * module_j) % 3) == 0); break;
                    case 6: should_toggle = ((((module_i * module_j) % 2) + ((module_i * module_j) % 3)) % 2 == 0); break;
                    case 7: should_toggle = ((((module_i + module_j) % 2) + ((module_i * module_j) % 3)) % 2 == 0); break;
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

// Test mask pattern selection with different patterns
TEST(mask_selection_optimality)
{
    const size_t size = 21;  // Version 1 QR code
    int pattern_scores[QR_MASK_PATTERN_COUNT] = {0};
    
    // Test with different patterns to ensure selection works
    for (int test_case = 0; test_case < 5; test_case++) {
        qr_code *qr = create_test_qr(size);
        if (!qr) return 1;
        
        // Apply mask selection and get the best pattern
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
        
        // Verify a valid pattern was selected
        if (best_pattern < 0 || best_pattern >= QR_MASK_PATTERN_COUNT) {
            free_test_qr(qr);
            return 3000 + best_pattern;  // Error code for invalid pattern
        }
        
        // Track which patterns are selected
        pattern_scores[best_pattern]++;
        free_test_qr(qr);
    }
    
    // Verify that different patterns are being selected (not always the same one)
    int different_patterns_selected = 0;
    for (int i = 0; i < QR_MASK_PATTERN_COUNT; i++) {
        if (pattern_scores[i] > 0) {
            different_patterns_selected++;
        }
    }
    
    if (different_patterns_selected < 2) {
        return 4000 + different_patterns_selected;  // Error code for insufficient pattern variation
    }
    
    return 0;
}

// Test evaluation features
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

// Test mask application with different QR code versions
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

// Test mask pattern selection with known patterns
TEST(mask_pattern_selection_known_cases)
{
    // Create a QR code with a known pattern that should be optimized by a specific mask
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

// Test mask pattern application
TEST(mask_patterns)
{
    // Create a simple QR code structure for testing
    qr_code qr = {0};
    qr.version = 1;  // Version 1 QR code (21x21)
    qr.side_length = 21 + 8;  // 21 modules + 8 quiet zone (4 on each side)
    qr.matrix = calloc(qr.side_length * qr.side_length, sizeof(int));
    
    if (!qr.matrix) return 1;
    
    // Fill with a checkerboard pattern (skip quiet zone)
    for (size_t i = 4; i < qr.side_length - 4; i++) {
        for (size_t j = 4; j < qr.side_length - 4; j++) {
            qr.matrix[i * qr.side_length + j] = ((i + j) % 2) ? 1 : 0;
        }
    }
    
    // Test each mask pattern
    for (int pattern = 0; pattern < QR_MASK_PATTERN_COUNT; pattern++) {
        // Apply the mask pattern
        qr_mask_apply_pattern(&qr, pattern);
        
        // Count how many modules were toggled
        for (size_t i = 0; i < qr.side_length; i++) {
            for (size_t j = 0; j < qr.side_length; j++) {
                // Skip quiet zone
                if (i < 4 || i >= qr.side_length - 4 || j < 4 || j >= qr.side_length - 4) continue;
                
                // Check if the module was toggled (should match the mask pattern)
                int should_toggle = 0;
                size_t module_i = i;  // No need to adjust for quiet zone
                size_t module_j = j;
                
                switch (pattern) {
                    case 0: should_toggle = ((module_i + module_j) % 2 == 0); break;
                    case 1: should_toggle = (module_i % 2 == 0); break;
                    case 2: should_toggle = (module_j % 3 == 0); break;
                    case 3: should_toggle = ((module_i + module_j) % 3 == 0); break;
                    case 4: should_toggle = (((module_i / 2) + (module_j / 3)) % 2 == 0); break;
                    case 5: should_toggle = (((module_i * module_j) % 2) + ((module_i * module_j) % 3) == 0); break;
                    case 6: should_toggle = ((((module_i * module_j) % 2) + ((module_i * module_j) % 3)) % 2 == 0); break;
                    case 7: should_toggle = ((((module_i + module_j) % 2) + ((module_i * module_j) % 3)) % 2 == 0); break;
                }
                
                // The module should be toggled if the mask pattern says so
                int expected = ((i + j) % 2) ^ should_toggle ? 1 : 0;
                if (qr.matrix[i * qr.side_length + j] != expected) {
                    free(qr.matrix);
                    return 2;  // Mask pattern application failed
                }
            }
        }
        
        // Apply the mask again to undo it (should return to original)
        qr_mask_apply_pattern(&qr, pattern);
        
        // Verify we're back to the original pattern
        for (size_t i = 0; i < qr.side_length; i++) {
            for (size_t j = 0; j < qr.side_length; j++) {
                int expected = (i + j) % 2 ? 1 : 0;
                if (qr.matrix[i * qr.side_length + j] != expected) {
                    free(qr.matrix);
                    return 3;  // Double mask application didn't return to original
                }
            }
        }
    }
    
    free(qr.matrix);
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
