# Contributing to FastQTools

Thank you for considering contributing to FastQTools! To keep things simple (KISS) and efficient, please follow the guidelines below.

## Getting Started

- Use recent CMake (>= 3.20) and a C++20 compiler (GCC 11+ / Clang 12+).
- Preferred build flow uses Conan or vcpkg toolchains.
- Run a full build and tests locally before opening a PR.

## Development Workflow

1. Fork the repo and create a feature branch.
2. Keep PRs small and focused. One change per PR.
3. Ensure formatting/lint pass:
   - `./scripts/format.sh`
   - `./scripts/lint.sh`
4. Run tests: `./scripts/test.sh` (or `ctest`).
5. Add/update documentation when behavior changes.

## Commit Messages

- Use conventional commits where possible, e.g.:
  - `feat: ...`, `fix: ...`, `chore: ...`, `docs: ...`, `test: ...`.
- Keep subject concise; add details in body if necessary.

## Code Style

- Follow `.clang-format` and `.clang-tidy`.
- Prefer modern C++ (RAII, smart pointers, const-correctness).
- Keep public headers minimal and stable.

## Tests

- Add unit tests for new features and bug fixes.
- Keep tests fast and deterministic.

## Security

- See `SECURITY.md` for reporting vulnerabilities.

## License

- By contributing, you agree that your contributions will be licensed under the project license (MIT unless otherwise specified).
