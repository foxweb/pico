# Changelog

All notable changes to Pico HTTP Server will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- **HEAD method support** - Check resource existence without downloading content
  - `HEAD()` macro for defining HEAD routes
  - Example HEAD routes for `/`, `/test`, and static files
  - Returns HTTP headers without message body (RFC 7231 compliant)
- Enhanced `/test` endpoint with comprehensive system information:
  - Current date/time with timezone
  - Server uptime calculation
  - OS information (name, version, architecture, hostname)
  - Compiler information (type, version)
  - Compilation date/time
  - C standard version
  - Process IDs (PID, PPID)
  - Server configuration (max connections, buffer size)
  - HTTP request headers
- Use `SOMAXCONN` for `listen()` backlog instead of hardcoded value
- Path traversal attack prevention with `is_path_safe()` function
- Buffer overflow protection in request handling
- NULL pointer checks after `strtok()` calls
- Error handling for all system calls (malloc, mmap, recv)
- Graceful shutdown with signal handlers (SIGINT, SIGTERM)
- Helper functions to reduce code duplication (`build_public_path`, `serve_static_file`)
- Comprehensive Doxygen documentation for all public functions
- Complete README with security warnings, examples, and API reference
- CONTRIBUTING.md with contribution guidelines
- SECURITY.md with security policy
- GitHub Actions CI/CD configuration

### Changed
- Simplified GitHub Actions workflow to Ubuntu-only (removed macOS builds)
- Updated GitHub Actions workflow to use v4 of `actions/checkout` and `actions/upload-artifact`
- Replaced `signal()` with `sigaction()` for more reliable signal handling
- Replaced unsafe `sprintf()` with `snprintf()` throughout codebase
- Fixed format string vulnerability in POST handler
- Moved static `reqhdr` array from header to implementation file
- Improved error messages and logging

### Fixed
- Buffer overflow when `rcvd == BUF_SIZE`
- Race condition between parent and child processes closing sockets
- Memory leak: added `munmap()` for shared memory cleanup
- Format string vulnerability in payload printing
- NULL pointer dereference in request parsing
- Unsafe pointer arithmetic in header parsing
- Invalid error handling (execution continuing after errors)
- Typo in error message: "upexpectedly" → "unexpectedly"
- Unit tests: Added conditional compilation for `main()` function
- Unit tests: Fixed file path in `test_file_exists_real_file`
- Unit tests: Updated compilation to include `httpd.c`

### Security
- Fixed critical path traversal vulnerability (CVE-TBD)
- Fixed buffer overflow vulnerabilities
- Fixed format string vulnerability
- Added input validation throughout request handling
- Improved bounds checking in all string operations

## [1.0.0] - Initial Release

### Added
- Basic HTTP server with fork-based concurrency
- GET and POST request handling
- Static file serving
- Request header parsing
- Simple routing with macros
- Support for up to 1000 concurrent connections

[Unreleased]: https://github.com/yourusername/pico/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/yourusername/pico/releases/tag/v1.0.0
