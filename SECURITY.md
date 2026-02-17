# Security Policy

## Purpose

This document outlines the security policy for Pico HTTP Server, including how to report vulnerabilities and what to expect.

## Supported Versions

| Version | Supported          |
| ------- | ------------------ |
| Latest  | :white_check_mark: |
| < 1.0   | :x:                |

## Security Status

**⚠️ Important: Development/Educational Use Only**

Pico HTTP Server is a minimal HTTP server designed for **development, learning, and prototyping**. While we've addressed many security issues, it should **NOT** be used in production environments without thorough security review and hardening.

## Known Limitations

### Current Security Features

✅ Path traversal attack prevention
✅ Buffer overflow protection
✅ Format string vulnerability fixes
✅ NULL pointer dereference protection
✅ Input validation for HTTP requests
✅ Graceful shutdown handling

### Known Limitations

⚠️ No HTTPS/TLS support
⚠️ No authentication or authorization
⚠️ No rate limiting or DoS protection
⚠️ Fork-based model vulnerable to fork bomb attacks
⚠️ No HTTP header size limits enforcement
⚠️ No protection against slowloris attacks
⚠️ Minimal HTTP protocol compliance
⚠️ No security headers (HSTS, CSP, etc.)

## Reporting a Vulnerability

### How to Report

If you discover a security vulnerability, please report it by:

1. **Opening a GitHub Issue** with the label "security"
2. Include:
   - Description of the vulnerability
   - Steps to reproduce
   - Potential impact
   - Suggested fix (if any)

### What to Expect

- **Response Time**: We aim to respond within 48-72 hours
- **Updates**: You'll receive updates on the status of your report
- **Fix Timeline**: Varies by severity
  - Critical: Immediate attention
  - High: Within 1 week
  - Medium: Within 2-4 weeks
  - Low: Next release

### Public Disclosure

- We request you allow us time to fix the issue before public disclosure
- Suggested timeline: 90 days from initial report
- We'll credit you in the security advisory unless you prefer to remain anonymous

## Security Best Practices

If you choose to use Pico HTTP Server, follow these guidelines:

### For Development

1. **Network Isolation**: Run only on localhost or isolated networks
2. **Firewall**: Use firewall rules to restrict access
3. **Regular Updates**: Keep the codebase updated
4. **Code Review**: Review any custom code for security issues

### NOT Recommended For

- Production web servers
- Internet-facing applications
- Applications handling sensitive data
- High-traffic websites
- Applications requiring authentication

### If You Must Use in Production

Consider these additional measures:

1. **Reverse Proxy**: Put behind nginx or Apache with security hardening
2. **HTTPS Termination**: Use reverse proxy for TLS/SSL
3. **Authentication**: Implement at reverse proxy level
4. **Rate Limiting**: Implement at firewall/proxy level
5. **Monitoring**: Set up intrusion detection
6. **Sandboxing**: Run in a container or VM with limited privileges
7. **Updates**: Monitor for security updates
8. **Audit**: Conduct professional security audit

## Security Testing

### Tools Used

We test with:
- **Valgrind**: Memory leak detection
- **GCC/Clang sanitizers**: Address, undefined behavior detection
- **Manual review**: Code auditing
- **Siege**: Load testing

### Recommended Testing

Contributors should test with:

```sh
# Compile with sanitizers
gcc -fsanitize=address,undefined -g -o server main.c httpd.c

# Memory leak check
valgrind --leak-check=full ./server 8000

# Load testing
siege -c 100 -r 10 http://localhost:8000/
```

## Security Contact

For urgent security issues, please use GitHub Issues with "security" label.

## Vulnerability History

### Addressed in Recent Updates

1. **Path Traversal (Critical)**
   - Status: Fixed
   - Description: Could access files outside public directory
   - Fix: Added path validation with `is_path_safe()`

2. **Buffer Overflow (High)**
   - Status: Fixed
   - Description: Writing past buffer when `rcvd == BUF_SIZE`
   - Fix: Added bounds checking

3. **Format String (High)**
   - Status: Fixed
   - Description: Payload could contain format specifiers
   - Fix: Use `fwrite()` instead of `printf()` with user data

4. **NULL Pointer Dereference (Medium)**
   - Status: Fixed
   - Description: `strtok()` results used without checking
   - Fix: Added NULL checks throughout parsing

5. **Race Condition (Medium)**
   - Status: Fixed
   - Description: Parent closing socket while child using it
   - Fix: Only child closes client socket

6. **Memory Leak (Low)**
   - Status: Fixed
   - Description: Shared memory not freed on shutdown
   - Fix: Added cleanup handler with `munmap()`

## Attribution

We appreciate responsible disclosure from security researchers. Contributors will be acknowledged in:
- CHANGELOG.md
- Security advisories
- Project credits (unless you prefer anonymity)

## License

This security policy is licensed under CC0 1.0 Universal.
