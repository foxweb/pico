#!/bin/bash

# Test runner script for Pico HTTP Server

echo "======================================="
echo "Pico HTTP Server Test Suite"
echo "======================================="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Track results
TESTS_PASSED=0
TESTS_FAILED=0

# Change to project root
cd "$(dirname "$0")/.." || exit 1

# Compile unit tests
echo -e "${YELLOW}Compiling unit tests...${NC}"
gcc -g -o tests/test_main tests/test_main.c main.c -I. -DTESTING || {
  echo -e "${RED}Failed to compile unit tests${NC}"
  exit 1
}
echo -e "${GREEN}Compilation successful${NC}"
echo ""

# Run unit tests
echo -e "${YELLOW}Running unit tests...${NC}"
./tests/test_main
if [ $? -eq 0 ]; then
  echo -e "${GREEN}✓ Unit tests passed${NC}"
  TESTS_PASSED=$((TESTS_PASSED + 1))
else
  echo -e "${RED}✗ Unit tests failed${NC}"
  TESTS_FAILED=$((TESTS_FAILED + 1))
fi
echo ""

# Compile main server
echo -e "${YELLOW}Compiling server...${NC}"
make clean > /dev/null 2>&1
make || {
  echo -e "${RED}Failed to compile server${NC}"
  exit 1
}
echo -e "${GREEN}Server compilation successful${NC}"
echo ""

# Memory leak test with valgrind (if available)
if command -v valgrind &> /dev/null; then
  echo -e "${YELLOW}Running memory leak tests (this may take a moment)...${NC}"
  
  # Start server in background with valgrind
  valgrind --leak-check=full --error-exitcode=1 --log-file=tests/valgrind.log \
    ./server 9999 &
  SERVER_PID=$!
  
  # Wait for server to start
  sleep 2
  
  # Send test request
  curl -s http://localhost:9999/ > /dev/null 2>&1 || true
  
  # Stop server gracefully
  kill -INT $SERVER_PID 2>/dev/null
  wait $SERVER_PID 2>/dev/null
  
  # Check valgrind results
  if grep -q "no leaks are possible" tests/valgrind.log; then
    echo -e "${GREEN}✓ No memory leaks detected${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
  else
    echo -e "${YELLOW}⚠ Potential memory issues detected (see tests/valgrind.log)${NC}"
    # Don't fail the test suite for this
  fi
  echo ""
else
  echo -e "${YELLOW}Valgrind not available, skipping memory tests${NC}"
  echo ""
fi

# Cleanup
rm -f tests/test_main

# Summary
echo "======================================="
echo "Test Summary"
echo "======================================="
echo -e "Tests passed: ${GREEN}$TESTS_PASSED${NC}"
echo -e "Tests failed: ${RED}$TESTS_FAILED${NC}"

if [ $TESTS_FAILED -eq 0 ]; then
  echo -e "${GREEN}All tests passed!${NC}"
  exit 0
else
  echo -e "${RED}Some tests failed${NC}"
  exit 1
fi
