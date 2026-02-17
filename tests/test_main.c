#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <limits.h>
#include <unistd.h>

// Simple test framework
#define TEST(name) void name()
#define RUN_TEST(name) do { \
  printf("Running %s...", #name); \
  name(); \
  printf(" PASSED\n"); \
  tests_passed++; \
} while(0)

#define ASSERT_TRUE(expr) do { \
  if (!(expr)) { \
    fprintf(stderr, "\n  FAILED: %s (line %d)\n", #expr, __LINE__); \
    exit(1); \
  } \
} while(0)

#define ASSERT_FALSE(expr) ASSERT_TRUE(!(expr))
#define ASSERT_EQUAL(a, b) ASSERT_TRUE((a) == (b))
#define ASSERT_NOT_NULL(ptr) ASSERT_TRUE((ptr) != NULL)
#define ASSERT_NULL(ptr) ASSERT_TRUE((ptr) == NULL)

// Test counter
static int tests_passed = 0;

// Include functions to test (from main.c)
// We'll declare them here since they're in main.c

extern int is_path_safe(const char *path, const char *base_dir);
extern int file_exists(const char *file_name);
extern int build_public_path(char *dest, size_t dest_size, const char *relative_path);

// Mock public dir for tests
#define TEST_PUBLIC_DIR "./public"

// ==================== Tests ====================

TEST(test_is_path_safe_valid_path) {
  // Test valid path within public directory
  ASSERT_TRUE(is_path_safe("./public/index.html", "./public"));
}

TEST(test_is_path_safe_traversal_attack) {
  // Test path traversal attempts
  ASSERT_FALSE(is_path_safe("./public/../etc/passwd", "./public"));
  ASSERT_FALSE(is_path_safe("../etc/passwd", "./public"));
}

TEST(test_is_path_safe_absolute_path) {
  // Test absolute paths (should be blocked)
  ASSERT_FALSE(is_path_safe("/etc/passwd", "./public"));
}

TEST(test_build_public_path_normal) {
  char dest[256];
  int result = build_public_path(dest, sizeof(dest), "/index.html");
  
  ASSERT_EQUAL(result, 0);
  ASSERT_TRUE(strcmp(dest, "./public/index.html") == 0);
}

TEST(test_build_public_path_buffer_too_small) {
  char dest[10]; // Too small
  int result = build_public_path(dest, sizeof(dest), "/very_long_path_that_wont_fit.html");
  
  ASSERT_TRUE(result < 0); // Should fail
}

TEST(test_build_public_path_root) {
  char dest[256];
  int result = build_public_path(dest, sizeof(dest), "/");
  
  ASSERT_EQUAL(result, 0);
  ASSERT_TRUE(strcmp(dest, "./public/") == 0);
}

TEST(test_file_exists_real_file) {
  // Test with Makefile (should exist)
  int exists = file_exists("Makefile");
  ASSERT_TRUE(exists);
}

TEST(test_file_exists_nonexistent_file) {
  // Test with non-existent file
  int exists = file_exists("/nonexistent_file_12345.txt");
  ASSERT_FALSE(exists);
}

// ==================== Test Runner ====================

int main() {
  printf("Running Unit Tests\n");
  printf("==================\n\n");
  
  // Path safety tests
  RUN_TEST(test_is_path_safe_valid_path);
  RUN_TEST(test_is_path_safe_traversal_attack);
  RUN_TEST(test_is_path_safe_absolute_path);
  
  // Build path tests
  RUN_TEST(test_build_public_path_normal);
  RUN_TEST(test_build_public_path_buffer_too_small);
  RUN_TEST(test_build_public_path_root);
  
  // File exists tests
  RUN_TEST(test_file_exists_real_file);
  RUN_TEST(test_file_exists_nonexistent_file);
  
  printf("\n==================\n");
  printf("All %d tests passed!\n", tests_passed);
  
  return 0;
}
