# ADAN Makefile - Build, test, and development commands

.PHONY: all build test run clean fmt lint check help docker-build docker-run docker-shell push build-asm run-asm clean-asm install uninstall

# Installation paths (follows FHS - Filesystem Hierarchy Standard)
PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
MANDIR ?= $(PREFIX)/share/man/man1
DOCDIR ?= $(PREFIX)/share/doc/adan
EXAMPLESDIR ?= $(PREFIX)/share/adan/examples

# Version info
VERSION = 0.1.0

# Default target
all: fmt lint test build
	@echo "✓ All checks passed!"

# Build the compiler
build:
	@echo "Building ADAN compiler..."
	@cargo build
	@echo "✓ Build complete"

# Build in release mode (optimized)
release:
	@echo "Building ADAN compiler (release mode)..."
	@cargo build --release
	@echo "✓ Release build complete"

# Run all tests
test:
	@echo "Running tests..."
	@cargo test
	@echo "✓ Tests complete"

# Run tests with output
test-verbose:
	@echo "Running tests (verbose)..."
	@cargo test -- --nocapture
	@echo "✓ Tests complete"

# Run specific test
test-one:
	@echo "Usage: make test-one TEST=test_name"
	@cargo test $(TEST)

# Run only parser tests
test-parser:
	@echo "Running parser tests..."
	@cargo test parser_tests
	@echo "✓ Parser tests complete"

# Run only lexer tests
test-lexer:
	@echo "Running lexer tests..."
	@cargo test lexer::tests
	@echo "✓ Lexer tests complete"

# Run the compiler
run:
	@echo "Running ADAN compiler..."
	@cargo run

# Run with a specific file
run-file:
	@echo "Usage: make run-file FILE=script.adn"
	@cargo run $(FILE)

# Compile a specific ADAN script
compile:
	@if [ -z "$(FILE)" ]; then \
		echo "Usage: make compile FILE=script.adn"; \
		exit 1; \
	fi
	@echo "Compiling $(FILE)..."
	@cargo run $(FILE)

# Format code
fmt:
	@echo "Formatting code..."
	@cargo fmt
	@echo "✓ Code formatted"

# Check formatting without modifying
fmt-check:
	@echo "Checking code formatting..."
	@cargo fmt --check
	@echo "✓ Format check complete"

# Run clippy linter
lint:
	@echo "Running clippy linter..."
	@cargo clippy -- -D warnings
	@echo "✓ Lint complete"

# Check code without building
check:
	@echo "Checking code..."
	@cargo check
	@echo "✓ Check complete"

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@cargo clean
	@echo "✓ Clean complete"

# Assembly compilation targets
build-asm:
	@if [ -z "$(FILE)" ]; then \
		echo "Usage: make build-asm FILE=yourfile.asm [OUTPUT=outputname]"; \
		exit 1; \
	fi
	@./build_asm.sh $(FILE) $(OUTPUT)

# Build and run assembly file
run-asm:
	@if [ -z "$(FILE)" ]; then \
		echo "Usage: make run-asm FILE=yourfile.asm"; \
		exit 1; \
	fi
	@./build_asm.sh $(FILE) temp_output
	@./temp_output
	@rm -f temp_output

# Clean assembly build artifacts
clean-asm:
	@echo "Cleaning assembly artifacts..."
	@rm -f *.o *_linux.asm output temp_output
	@echo "✓ Assembly artifacts cleaned"

# Watch for changes and rebuild
watch:
	@echo "Watching for changes..."
	@cargo watch -x build

# Watch and run tests on changes
watch-test:
	@echo "Watching for changes and running tests..."
	@cargo watch -x test

# Generate documentation
doc:
	@echo "Generating documentation..."
	@cargo doc --no-deps --open
	@echo "✓ Documentation generated"

# Generate documentation without opening
doc-build:
	@echo "Generating documentation..."
	@cargo doc --no-deps
	@echo "✓ Documentation generated"

# Run benchmarks (if any)
bench:
	@echo "Running benchmarks..."
	@cargo bench
	@echo "✓ Benchmarks complete"

# Update dependencies
update:
	@echo "Updating dependencies..."
	@cargo update
	@echo "✓ Dependencies updated"

# Check for outdated dependencies
outdated:
	@echo "Checking for outdated dependencies..."
	@cargo outdated
	@echo "✓ Check complete"

# Build Docker image
docker-build:
	@echo "Building Docker image..."
	@docker build -t adan-dev .
	@echo "✓ Docker image built"

# Run Docker container
docker-run:
	@echo "Running Docker container..."
	@docker run -it -v $(PWD):/ADAN-workspace adan-dev

# Run Docker container with bash shell
docker-shell:
	@echo "Opening shell in Docker container..."
	@docker run -it -v $(PWD):/ADAN-workspace adan-dev /bin/bash

# Build inside Docker
docker-test:
	@echo "Running tests inside Docker..."
	@docker run --rm -v $(PWD):/ADAN-workspace adan-dev cargo test

# Git quick push (original command)
push:
	@git add .
	@git commit -m "chore: quickly pushed using Makefile"
	@git push
	@echo "✓ Pushed changes to remote repository"

# Git commit with custom message
commit:
	@if [ -z "$(MSG)" ]; then \
		echo "Usage: make commit MSG=\"your message\""; \
		exit 1; \
	fi
	@git add .
	@git commit -m "$(MSG)"
	@echo "✓ Committed with message: $(MSG)"

# Git commit and push with custom message
commit-push:
	@if [ -z "$(MSG)" ]; then \
		echo "Usage: make commit-push MSG=\"your message\""; \
		exit 1; \
	fi
	@git add .
	@git commit -m "$(MSG)"
	@git push
	@echo "✓ Committed and pushed: $(MSG)"

# Show git status
status:
	@git status

# Show git log
log:
	@git log --oneline -10

# Install cargo-watch (for watch targets)
install-watch:
	@echo "Installing cargo-watch..."
	@cargo install cargo-watch
	@echo "✓ cargo-watch installed"

# Install cargo-outdated
install-outdated:
	@echo "Installing cargo-outdated..."
	@cargo install cargo-outdated
	@echo "✓ cargo-outdated installed"

# Install development tools
install-tools: install-watch install-outdated
	@echo "✓ All development tools installed"

# Create a new example file
new-example:
	@if [ -z "$(NAME)" ]; then \
		echo "Usage: make new-example NAME=example_name"; \
		exit 1; \
	fi
	@mkdir -p examples
	@touch examples/$(NAME).adn
	@echo "// Example: $(NAME)" > examples/$(NAME).adn
	@echo "program main() {" >> examples/$(NAME).adn
	@echo "    // Your code here" >> examples/$(NAME).adn
	@echo "}" >> examples/$(NAME).adn
	@echo "✓ Created examples/$(NAME).adn"

# Run all checks before commit
pre-commit: fmt lint test
	@echo "✓ Pre-commit checks passed!"

# Quick development cycle
dev: fmt check test
	@echo "✓ Development cycle complete!"

# Full CI/CD simulation
ci: clean fmt-check lint test build
	@echo "✓ CI checks passed!"

# Show lines of code
loc:
	@echo "Lines of code (Rust):"
	@find src -name "*.rs" | xargs wc -l | tail -1
	@echo ""
	@echo "Lines of code (by file):"
	@find src -name "*.rs" | xargs wc -l | sort -n

# Show project structure
tree:
	@tree -I 'target|.git' -L 3 || ls -R

# Installation targets
install: release
	@echo "Installing ADAN $(VERSION)..."
	@install -Dm755 target/release/ADAN "$(DESTDIR)$(BINDIR)/adan"
	@install -Dm755 build_asm.sh "$(DESTDIR)$(BINDIR)/adan-build-asm"
	@mkdir -p "$(DESTDIR)$(DOCDIR)"
	@install -Dm644 README.md "$(DESTDIR)$(DOCDIR)/README.md" 2>/dev/null || echo "README.md not found, skipping..."
	@mkdir -p "$(DESTDIR)$(EXAMPLESDIR)"
	@if [ -d examples ]; then \
		cp -r examples/* "$(DESTDIR)$(EXAMPLESDIR)/" 2>/dev/null || true; \
	fi
	@echo ""
	@echo "✓ Installation complete!"
	@echo ""
	@echo "  Binary:    $(BINDIR)/adan"
	@echo "  Helper:    $(BINDIR)/adan-build-asm"
	@echo "  Docs:      $(DOCDIR)"
	@echo "  Examples:  $(EXAMPLESDIR)"
	@echo ""
	@echo "Usage:"
	@echo "  adan --help"
	@echo "  adan --native examples/Math.adn"

uninstall:
	@echo "Uninstalling ADAN..."
	@rm -f "$(DESTDIR)$(BINDIR)/adan"
	@rm -f "$(DESTDIR)$(BINDIR)/adan-build-asm"
	@rm -rf "$(DESTDIR)$(DOCDIR)"
	@rm -rf "$(DESTDIR)$(EXAMPLESDIR)"
	@echo "✓ ADAN has been uninstalled"

# Create a distribution tarball
dist: clean
	@echo "Creating distribution tarball..."
	@mkdir -p dist
	@tar czf dist/adan-$(VERSION).tar.gz \
		--exclude='.git' \
		--exclude='target' \
		--exclude='dist' \
		--exclude='*.o' \
		--exclude='*.asm' \
		--transform 's,^,adan-$(VERSION)/,' \
		.
	@echo "✓ Created dist/adan-$(VERSION).tar.gz"

# Show installation info
install-info:
	@echo "ADAN Installation Information"
	@echo "============================="
	@echo ""
	@echo "Installation paths:"
	@echo "  PREFIX:     $(PREFIX)"
	@echo "  BINDIR:     $(BINDIR)"
	@echo "  DOCDIR:     $(DOCDIR)"
	@echo "  EXAMPLESDIR: $(EXAMPLESDIR)"
	@echo ""
	@echo "To install system-wide (requires root):"
	@echo "  sudo make install"
	@echo ""
	@echo "To install to user directory:"
	@echo "  make install PREFIX=~/.local"
	@echo ""
	@echo "To install to custom location:"
	@echo "  make install PREFIX=/custom/path"
	@echo ""
	@echo "To uninstall:"
	@echo "  sudo make uninstall"

# Display help
help:
	@echo "ADAN Makefile - Available Commands"
	@echo ""
	@echo "Building:"
	@echo "  make build          - Build the compiler"
	@echo "  make release        - Build optimized release version"
	@echo "  make clean          - Clean build artifacts"
	@echo "  make build-asm FILE=file.asm [OUTPUT=name] - Compile assembly (cross-platform)"
	@echo "  make run-asm FILE=file.asm - Compile and run assembly"
	@echo "  make clean-asm      - Clean assembly artifacts"
	@echo ""
	@echo "Installation:"
	@echo "  make install        - Install ADAN system-wide (requires root)"
	@echo "  make uninstall      - Remove ADAN from system"
	@echo "  make install-info   - Show installation information"
	@echo "  make dist           - Create distribution tarball"
	@echo ""
	@echo "Testing:"
	@echo "  make test           - Run all tests"
	@echo "  make test-verbose   - Run tests with output"
	@echo "  make test-parser    - Run only parser tests"
	@echo "  make test-lexer     - Run only lexer tests"
	@echo "  make test-one TEST=name - Run specific test"
	@echo ""
	@echo "Running:"
	@echo "  make run            - Run the compiler"
	@echo "  make run-file FILE=script.adn - Run with specific file"
	@echo "  make compile FILE=script.adn  - Compile ADAN script"
	@echo ""
	@echo "Code Quality:"
	@echo "  make fmt            - Format code"
	@echo "  make fmt-check      - Check formatting"
	@echo "  make lint           - Run clippy linter"
	@echo "  make check          - Check code without building"
	@echo ""
	@echo "Development:"
	@echo "  make watch          - Watch and rebuild on changes"
	@echo "  make watch-test     - Watch and test on changes"
	@echo "  make dev            - Quick dev cycle (fmt + check + test)"
	@echo "  make pre-commit     - Run pre-commit checks"
	@echo "  make all            - Run all checks"
	@echo ""
	@echo "Docker:"
	@echo "  make docker-build   - Build Docker image"
	@echo "  make docker-run     - Run in Docker container"
	@echo "  make docker-shell   - Open shell in Docker"
	@echo "  make docker-test    - Run tests in Docker"
	@echo ""
	@echo "Git:"
	@echo "  make push           - Quick add, commit, and push"
	@echo "  make commit MSG=\"...\" - Commit with custom message"
	@echo "  make commit-push MSG=\"...\" - Commit and push"
	@echo "  make status         - Show git status"
	@echo "  make log            - Show recent commits"
	@echo ""
	@echo "Documentation:"
	@echo "  make doc            - Generate and open documentation"
	@echo "  make doc-build      - Generate documentation"
	@echo ""
	@echo "Utilities:"
	@echo "  make new-example NAME=name - Create new example file"
	@echo "  make loc            - Count lines of code"
	@echo "  make tree           - Show project structure"
	@echo "  make update         - Update dependencies"
	@echo "  make install-tools  - Install development tools"
	@echo "  make help           - Show this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make test-one TEST=test_parse_integer"
	@echo "  make compile FILE=my_script.adn"
	@echo "  make commit MSG=\"feat: add new feature\""
	@echo "  make new-example NAME=fibonacci"