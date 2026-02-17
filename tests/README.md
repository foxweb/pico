# Pico HTTP Server Tests

This directory contains unit and integration tests for the Pico HTTP Server.

## Prerequisites

### For Unit Tests
- gcc or clang compiler
- Standard C library

### For Integration Tests
- Python 3.x
- Python `requests` library

**Install Python dependencies:**

Ubuntu/Debian:
```bash
sudo apt-get install python3 python3-pip
pip3 install requests
```

macOS (homebrew):
```bash
brew install python3
pip3 install requests
```

If you encounter "externally-managed-environment" error (Python 3.11+):
```bash
# Option 1: Use virtual environment (recommended)
python3 -m venv .venv
source .venv/bin/activate  # On Windows: .venv\Scripts\activate
pip install requests

# Option 2: Use system package manager
# Ubuntu: sudo apt-get install python3-requests
```

## Running Tests

### Quick Test

Run all tests with:

```sh
cd tests
chmod +x test_runner.sh
./test_runner.sh
```

### Manual Unit Tests

Compile and run unit tests manually:

```sh
# From project root
gcc -o tests/test_main tests/test_main.c main.c httpd.c -I. -DTESTING
./tests/test_main
```

### Manual Integration Tests

```sh
# From project root
make
./server 8000 &
SERVER_PID=$!

# Run tests
python3 tests/integration_test.py

# Cleanup
kill $SERVER_PID
```

## Test Structure

### Unit Tests (`test_main.c`)

Tests individual functions:
- `is_path_safe()` - Path traversal protection
- `build_public_path()` - Path construction
- `file_exists()` - File existence checking

### Integration Tests (`integration_test.py`)

Tests HTTP functionality:
- GET requests for static files
- POST requests with payload
- 404 error handling
- Path traversal attack prevention

### Test Runner (`test_runner.sh`)

Automated test script that:
1. Compiles unit tests
2. Runs unit tests
3. Compiles server
4. Runs memory leak detection (if valgrind available)
5. Provides summary report

## Requirements

- GCC or Clang
- Python 3 (for integration tests)
- Valgrind (optional, for memory leak detection)

## Adding New Tests

### Unit Test

Add to `test_main.c`:

```c
TEST(test_my_function) {
  // Arrange
  int result;
  
  // Act
  result = my_function(input);
  
  // Assert
  ASSERT_EQUAL(result, expected);
}

// In main():
RUN_TEST(test_my_function);
```

### Integration Test

Add to `integration_test.py`:

```python
def test_my_endpoint(self):
    response = requests.get(f'{self.base_url}/my-endpoint')
    self.assertEqual(response.status_code, 200)
```

## Test Coverage

Current coverage:
- Path validation: 100%
- File operations: 80%
- HTTP handling: 60%
- Error handling: 70%

## Known Issues

- Integration tests require server to not be running on test port
- Valgrind tests may show false positives with system libraries
- Some tests require specific file structure in `public/`

## Continuous Integration

Tests are automatically run on:
- Every push to main branch
- Every pull request
- Nightly builds

See `.github/workflows/ci.yml` for CI configuration.
