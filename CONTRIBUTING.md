<div align="center">
	<h1>Contributing to ADAN</h1>
	<p>Help us make ADAN safer, faster, and more reliable for everyone.</p>
</div>

---

We welcome contributions from anyone! Whether you're fixing bugs, adding features, improving documentation, or optimizing performance, your help is appreciated.

## Getting Started

### Prerequisites

- Basic knowledge of C programming
- Familiarity with Git and GitHub
- Understanding of compiler concepts (lexing, parsing, semantic analysis)
- A development environment with GCC/Clang and Make

### Setting Up Your Development Environment

1. Fork the repository on GitHub
2. Clone your fork locally:
   ```bash
   git clone https://github.com/YOUR_USERNAME/ADAN.git
   cd ADAN
   ```
3. Add the upstream repository:
   ```bash
   git remote add upstream https://github.com/Cappucina/ADAN.git
   ```
4. Create a new branch for your work:
   ```bash
   git checkout -b feature/your-feature-name
   ```

## How to Contribute

### Reporting Bugs

Found a bug? Perfect, you can help us fix it by opening an issue.

1. Check existing issues to avoid duplicates
2. Include a clear title and description
3. Provide a minimal code example that reproduces the issue
4. Specify your environment (OS, compiler, etc.)
5. Attach relevant error messages or logs

**Example:**
```
Title: Parser fails on nested array declarations

Description:
The parser crashes when parsing nested arrays like `x::int[5][3];`

Reproduction:
```c
x::int[5][3];
```

Expected: Should parse successfully
Actual: Segmentation fault
```

### Submitting Pull Requests

1. **Before you start:** Open an issue to discuss your changes with maintainers
2. **Keep it focused:** Each PR should address one feature or bug fix
3. **Follow the code style:** Match existing code formatting and conventions
4. **Write clear commits:** Use descriptive commit messages
5. **Test your changes:** Ensure all tests pass
   ```bash
   make clean && make run
   ```
6. **Update documentation:** If you change functionality, update README or comments
7. **Push to your fork** and open a pull request with:
   - Clear title and description
   - Reference to related issues (`Fixes #123`)
   - Summary of changes made

### Code Style Guidelines

- Use consistent indentation (tabs)
- Keep lines reasonably short
- Use meaningful variable and function names
- Add comments for complex logic
- Follow existing patterns in the codebase

**Example:**
```c
// Good
Type analyze_binary_op(ASTNode* binary_node, SymbolTable* table) {
	if (!binary_node || !table) return TYPE_UNKNOWN;
	
	// Get the left and right operand types
	Type left = analyze_expression(binary_node->children[0], table);
	Type right = analyze_expression(binary_node->children[1], table);
	
	// Validate type compatibility
	if (!check_type_compatibility(left, right)) {
		semantic_error(binary_node, "Operand types do not match");
		return TYPE_UNKNOWN;
	}
	
	return left;
}
```

## Development Workflow

### Building the Project

```bash
# Build everything
make

# Run tests
make run

# Clean build artifacts
make clean
```

### Testing

- Lexer tests are in `/tests/lexer_tests.c`
- Parser tests are in `/tests/parser_tests.c`
- Run all tests with `make run`
- Add new tests for new features or bug fixes

### Areas for Contribution

| Area | Status | Help Needed |
|------|--------|------------|
| Lexer | Complete | Optimizations, edge cases |
| Parser | Complete | Optimizations, error recovery |
| Semantic Analysis | Complete | Additional type checks, optimizations |
| Intermediate Representation (IR) | Not Started | Full implementation |
| Code Optimization | Not Started | Full implementation |
| Code Generation | Not Started | Full implementation |
| Linking | Not Started | Full implementation |

## Commit Message Guidelines

Write clear, descriptive commit messages:

```
<type>: <subject>

<body>

<footer>
```

**Types:**
- `feat:` New feature
- `fix:` Bug fix
- `refactor:` Code restructuring without changing behavior
- `optimize:` Performance improvements
- `docs:` Documentation changes
- `test:` Test additions or modifications

**Example:**
```
feat: Add strict type checking for binary operations

Implement comprehensive type validation for arithmetic and logical
operators to prevent implicit type conversions. Rejects operations
on void, null, array, string, and boolean types in invalid contexts.

Fixes #42
```

## Code Review Process

1. A maintainer will review your pull request
2. Changes may be requested for code quality, style, or functionality
3. Address feedback by pushing additional commits
4. Once approved, your PR will be merged

## Questions or Need Help?

- Open a GitHub Discussion for questions
- Check existing issues and PRs for similar topics
- Reach out to the maintainers