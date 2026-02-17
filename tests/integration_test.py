#!/usr/bin/env python3
"""
Integration tests for Pico HTTP Server

These tests verify that the HTTP server correctly handles various requests
and edge cases. The server must be running before executing these tests.
"""

import sys
import time
import subprocess
import requests
import os
import signal

class Colors:
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    END = '\033[0m'

class PicoServerTest:
    def __init__(self, port=9998):
        self.port = port
        self.base_url = f'http://localhost:{port}'
        self.server_process = None
        self.tests_passed = 0
        self.tests_failed = 0

    def start_server(self):
        """Start the Pico server in background"""
        print(f"{Colors.BLUE}Starting server on port {self.port}...{Colors.END}")
        try:
            # Start server process
            self.server_process = subprocess.Popen(
                ['./server', str(self.port)],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE
            )
            # Wait for server to start
            time.sleep(1)

            # Check if server is running
            if self.server_process.poll() is not None:
                print(f"{Colors.RED}Server failed to start{Colors.END}")
                return False

            print(f"{Colors.GREEN}Server started successfully{Colors.END}\n")
            return True
        except Exception as e:
            print(f"{Colors.RED}Failed to start server: {e}{Colors.END}")
            return False

    def stop_server(self):
        """Stop the Pico server"""
        if self.server_process:
            print(f"\n{Colors.BLUE}Stopping server...{Colors.END}")
            self.server_process.send_signal(signal.SIGINT)
            self.server_process.wait(timeout=5)
            print(f"{Colors.GREEN}Server stopped{Colors.END}")

    def run_test(self, test_name, test_func):
        """Run a single test and track results"""
        try:
            print(f"  Running: {test_name}...", end=' ')
            test_func()
            print(f"{Colors.GREEN}PASSED{Colors.END}")
            self.tests_passed += 1
        except AssertionError as e:
            print(f"{Colors.RED}FAILED{Colors.END}")
            print(f"    Error: {e}")
            self.tests_failed += 1
        except Exception as e:
            print(f"{Colors.RED}ERROR{Colors.END}")
            print(f"    Error: {e}")
            self.tests_failed += 1

    # ==================== Tests ====================

    def test_get_root(self):
        """Test GET / returns 200 OK"""
        response = requests.get(f'{self.base_url}/')
        assert response.status_code == 200, f"Expected 200, got {response.status_code}"
        assert len(response.text) > 0, "Response body is empty"

    def test_get_nonexistent_file(self):
        """Test GET /nonexistent returns 404"""
        response = requests.get(f'{self.base_url}/nonexistent_file_xyz.html')
        assert response.status_code == 404, f"Expected 404, got {response.status_code}"

    def test_path_traversal_blocked(self):
        """Test that path traversal attacks are blocked"""
        urls = [
            f'{self.base_url}/../../../etc/passwd',
            f'{self.base_url}/../../etc/passwd',
            f'{self.base_url}/../etc/passwd',
        ]

        for url in urls:
            response = requests.get(url)
            assert response.status_code == 404, \
                f"Path traversal not blocked: {url} returned {response.status_code}"
            assert 'Access denied' in response.text or 'File not found' in response.text, \
                f"Expected security message in response"

    def test_post_request(self):
        """Test POST request with data"""
        data = "test payload data"
        response = requests.post(f'{self.base_url}/', data=data)
        assert response.status_code in [200, 201], \
            f"Expected 200 or 201, got {response.status_code}"
        assert 'POST' in response.text or 'bytes' in response.text, \
            "Response doesn't mention POST or bytes"

    def test_post_empty_data(self):
        """Test POST request with empty data"""
        response = requests.post(f'{self.base_url}/', data='')
        assert response.status_code in [200, 201], \
            f"Expected 200 or 201, got {response.status_code}"

    def test_user_agent_header(self):
        """Test that User-Agent header is received"""
        headers = {'User-Agent': 'PicoTestClient/1.0'}
        response = requests.get(f'{self.base_url}/test', headers=headers)
        assert response.status_code == 200, f"Expected 200, got {response.status_code}"
        # The /test endpoint should show headers
        assert 'User-Agent' in response.text, "User-Agent not in response"

    def test_multiple_requests(self):
        """Test server handles multiple sequential requests"""
        for i in range(5):
            response = requests.get(f'{self.base_url}/')
            assert response.status_code == 200, \
                f"Request {i+1} failed with status {response.status_code}"

    def test_custom_header(self):
        """Test custom headers are received"""
        headers = {'X-Custom-Header': 'TestValue123'}
        response = requests.get(f'{self.base_url}/test', headers=headers)
        assert response.status_code == 200, f"Expected 200, got {response.status_code}"

    def test_query_string(self):
        """Test query string handling"""
        response = requests.get(f'{self.base_url}/test?key=value&foo=bar')
        assert response.status_code == 200, f"Expected 200, got {response.status_code}"

    def test_url_encoded_path(self):
        """Test URL-encoded characters in path"""
        response = requests.get(f'{self.base_url}/test%20path')
        # Should return 404 since the file doesn't exist, but should handle the encoding
        assert response.status_code in [200, 404], \
            f"Unexpected status code: {response.status_code}"

    def test_directory_listing(self):
        """Test directory listing shows files"""
        response = requests.get(f'{self.base_url}/testdir/')
        assert response.status_code == 200, f"Expected 200, got {response.status_code}"
        # Check for table headers
        assert 'Name</th>' in response.text, "Directory listing should have Name column"
        assert 'Last Modified</th>' in response.text, "Directory listing should have Last Modified column"
        assert 'Size</th>' in response.text, "Directory listing should have Size column"
        # Check for parent directory link
        assert '[Parent Directory]' in response.text, "Should have parent directory link"
        # Check for test files
        assert 'file1.txt' in response.text, "Should list file1.txt"
        assert 'file2.txt' in response.text, "Should list file2.txt"
        assert 'subdir/' in response.text, "Should list subdir/"

    def test_subdirectory_index_html(self):
        """Test subdirectory with index.html shows index instead of listing"""
        response = requests.get(f'{self.base_url}/testdir/subdir/')
        assert response.status_code == 200, f"Expected 200, got {response.status_code}"
        # Should serve index.html, not directory listing
        assert 'Subdir Index' in response.text, "Should serve index.html content"
        assert 'Index of' not in response.text, "Should NOT show directory listing"

    def test_directory_file_sizes(self):
        """Test directory listing shows file sizes"""
        response = requests.get(f'{self.base_url}/testdir/')
        assert response.status_code == 200, f"Expected 200, got {response.status_code}"
        # Check for size display (files should show size, dirs should show -)
        assert ' B</td>' in response.text or ' KB</td>' in response.text, \
            "Should show file sizes"

    def test_directory_dates(self):
        """Test directory listing shows modification dates"""
        response = requests.get(f'{self.base_url}/testdir/')
        assert response.status_code == 200, f"Expected 200, got {response.status_code}"
        # Check for date format (YYYY-MM-DD HH:MM:SS)
        import re
        assert re.search(r'\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}', response.text), \
            "Should show modification dates in proper format"

    # ==================== Test Runner ====================

    def run_all_tests(self):
        """Run all integration tests"""
        print(f"\n{Colors.BLUE}{'='*50}{Colors.END}")
        print(f"{Colors.BLUE}Pico HTTP Server Integration Tests{Colors.END}")
        print(f"{Colors.BLUE}{'='*50}{Colors.END}\n")

        # Start server
        if not self.start_server():
            print(f"{Colors.RED}Cannot run tests without server{Colors.END}")
            return False

        try:
            # Run tests
            print("Running tests:\n")

            self.run_test("GET root endpoint", self.test_get_root)
            self.run_test("GET nonexistent file (404)", self.test_get_nonexistent_file)
            self.run_test("Path traversal protection", self.test_path_traversal_blocked)
            self.run_test("POST request with data", self.test_post_request)
            self.run_test("POST request empty data", self.test_post_empty_data)
            self.run_test("User-Agent header", self.test_user_agent_header)
            self.run_test("Multiple sequential requests", self.test_multiple_requests)
            self.run_test("Custom header handling", self.test_custom_header)
            self.run_test("Query string handling", self.test_query_string)
            self.run_test("URL-encoded path", self.test_url_encoded_path)
            self.run_test("Directory listing", self.test_directory_listing)
            self.run_test("Subdirectory with index.html", self.test_subdirectory_index_html)
            self.run_test("Directory file sizes", self.test_directory_file_sizes)
            self.run_test("Directory modification dates", self.test_directory_dates)

            # Summary
            print(f"\n{Colors.BLUE}{'='*50}{Colors.END}")
            print(f"{Colors.BLUE}Test Summary{Colors.END}")
            print(f"{Colors.BLUE}{'='*50}{Colors.END}")
            print(f"Tests passed: {Colors.GREEN}{self.tests_passed}{Colors.END}")
            print(f"Tests failed: {Colors.RED}{self.tests_failed}{Colors.END}")

            if self.tests_failed == 0:
                print(f"\n{Colors.GREEN}All tests passed!{Colors.END}")
                return True
            else:
                print(f"\n{Colors.RED}Some tests failed{Colors.END}")
                return False

        finally:
            # Stop server
            self.stop_server()

def main():
    """Main entry point"""
    # Check if server binary exists
    if not os.path.exists('./server'):
        print(f"{Colors.RED}Error: ./server not found{Colors.END}")
        print("Please compile the server first with: make")
        sys.exit(1)

    # Run tests
    tester = PicoServerTest()
    success = tester.run_all_tests()

    # Exit with appropriate code
    sys.exit(0 if success else 1)

if __name__ == '__main__':
    main()
