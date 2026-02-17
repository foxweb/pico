# Contributing to Pico HTTP Server

Thank you for your interest in contributing to Pico HTTP Server! This document provides guidelines for contributing to the project.

## Code of Conduct

- Be respectful and considerate in all interactions
- Provide constructive feedback
- Focus on what is best for the community
- Show empathy towards other community members

## How to Contribute

### Reporting Bugs

Before creating a bug report, please check existing issues to avoid duplicates.

**When reporting bugs, include:**
- Operating system and version
- Compiler version (gcc/clang)
- Steps to reproduce the issue
- Expected vs actual behavior
- Any relevant error messages or logs

### Suggesting Enhancements

Enhancement suggestions are welcome! Please provide:
- Clear description of the feature
- Use case and motivation
- Example implementation if possible
- Potential impact on existing functionality

### Pull Requests

1. **Fork the repository** and create your branch from `master`
2. **Write clear, concise commit messages**
   - Use present tense ("Add feature" not "Added feature")
   - First line should be 50 chars or less
   - Add detailed description after blank line if needed
3. **Follow the existing code style**
   - Use 2-space indentation
   - Keep functions focused and small
   - Add comments for complex logic
4. **Test your changes**
   - Ensure the server compiles without warnings
   - Test basic functionality (GET, POST, static files)
   - Check for memory leaks with valgrind
5. **Update documentation** if adding new features
6. **Ensure no security vulnerabilities** are introduced

## Development Setup

### Prerequisites

- GCC or Clang
- GNU Make
- Git
- Optional: valgrind for memory leak detection

### Building

```sh
git clone <repository-url>
cd pico
make
```

### Testing

```sh
# Start server
./server 8000

# In another terminal, test endpoints
curl http://localhost:8000/
curl http://localhost:8000/test
curl -X POST -d "test data" http://localhost:8000/

# Memory leak check
valgrind --leak-check=full ./server 8000
```

## Code Style Guidelines

### C Code

- **Indentation**: 2 spaces (no tabs)
- **Line length**: Try to keep under 80 characters
- **Braces**: K&R style
  ```c
  if (condition) {
    // code
  } else {
    // code
  }
  ```
- **Naming**:
  - Functions: `snake_case`
  - Variables: `snake_case`
  - Macros/Constants: `UPPER_CASE`
  - Types: `snake_case_t`
- **Comments**: Use `//` for single-line, `/* */` for multi-line
- **Doxygen**: Add documentation for public functions

### Example

```c
/**
 * @brief Parse HTTP request headers
 * 
 * @param buffer Raw request buffer
 * @param size Buffer size
 * @return Number of headers parsed
 */
int parse_headers(const char *buffer, size_t size) {
  if (!buffer || size == 0) {
    return -1;
  }
  
  // Implementation here
  return 0;
}
```

## Security Considerations

When contributing, please ensure:

- **No buffer overflows**: Always use size-limited functions (snprintf, strncpy)
- **Input validation**: Validate all user input
- **No format string vulnerabilities**: Use proper printf formatting
- **Path traversal protection**: Validate file paths
- **NULL pointer checks**: Check all pointer returns
- **Resource cleanup**: Free allocated memory, close file descriptors

## Testing Checklist

Before submitting a PR:

- [ ] Code compiles without warnings (`make`)
- [ ] No memory leaks (check with `valgrind`)
- [ ] GET requests work correctly
- [ ] POST requests work correctly
- [ ] Static file serving works
- [ ] 404 errors handled properly
- [ ] Path traversal attacks blocked
- [ ] Graceful shutdown (Ctrl+C) works
- [ ] Documentation updated
- [ ] Comments added for complex code

## Questions?

If you have questions, feel free to:
- Open an issue for discussion
- Ask in pull request comments
- Check existing documentation

## License

By contributing, you agree that your contributions will be licensed under the MIT License.
