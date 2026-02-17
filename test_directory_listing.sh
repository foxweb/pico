#!/bin/bash
# Test script for directory listing feature

PORT=9994
SERVER_PID=""

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo "=========================================="
echo "  Directory Listing Test Suite"
echo "=========================================="
echo ""

# Create test structure
echo -e "${BLUE}Setting up test directories...${NC}"
mkdir -p public/testdir public/testdir/subdir public/testdir/emptydir
echo "Test file 1" > public/testdir/file1.txt
echo "Test file 2" > public/testdir/file2.txt
echo "<h1>Subdir Index</h1>" > public/testdir/subdir/index.html
echo "Large file content" > public/testdir/largefile.txt
for i in {1..100}; do echo "Line $i with some content" >> public/testdir/largefile.txt; done
echo -e "${GREEN}✓ Test structure created${NC}\n"

# Start server
echo -e "${BLUE}Starting server on port $PORT...${NC}"
./server $PORT >/dev/null 2>&1 &
SERVER_PID=$!
sleep 1

if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo -e "${RED}✗ Failed to start server${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Server started (PID: $SERVER_PID)${NC}\n"

# Test function
test_http() {
    local url=$1
    local test_name=$2
    local expected=$3
    
    echo -e "${BLUE}Test:${NC} $test_name"
    echo -e "  URL: $url"
    
    response=$(curl -s "$url")
    
    if echo "$response" | grep -q "$expected"; then
        echo -e "  ${GREEN}✓ PASS${NC} - Found: $expected"
    else
        echo -e "  ${RED}✗ FAIL${NC} - Expected to find: $expected"
        echo -e "  ${YELLOW}Response:${NC}"
        echo "$response" | head -5
    fi
    echo ""
}

# Run tests
echo "=========================================="
echo "  Running Tests"
echo "=========================================="
echo ""

test_http "http://localhost:$PORT/testdir/" \
    "Directory listing - table headers" \
    "Name</th>"

test_http "http://localhost:$PORT/testdir/" \
    "Directory listing - file1.txt" \
    "file1.txt"

test_http "http://localhost:$PORT/testdir/" \
    "Directory listing - file2.txt" \
    "file2.txt"

test_http "http://localhost:$PORT/testdir/" \
    "Directory listing - subdirectory" \
    "subdir/"

test_http "http://localhost:$PORT/testdir/" \
    "Directory listing - parent link" \
    "Parent Directory"

test_http "http://localhost:$PORT/testdir/" \
    "Directory listing - file sizes" \
    " B</td>"

test_http "http://localhost:$PORT/testdir/" \
    "Directory listing - dates" \
    "2026-"

test_http "http://localhost:$PORT/testdir/subdir/" \
    "Subdirectory with index.html" \
    "Subdir Index"

test_http "http://localhost:$PORT/testdir/emptydir/" \
    "Empty directory listing" \
    "Index of"

# Visual test
echo "=========================================="
echo "  Visual Inspection"
echo "=========================================="
echo ""
echo -e "${YELLOW}Directory listing HTML:${NC}"
curl -s "http://localhost:$PORT/testdir/" | head -40
echo ""
echo "..."
echo ""

# Stop server
echo "=========================================="
echo -e "${BLUE}Stopping server...${NC}"
kill $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null
echo -e "${GREEN}✓ Server stopped${NC}"
echo ""

echo "=========================================="
echo "  Test Suite Complete"
echo "=========================================="
