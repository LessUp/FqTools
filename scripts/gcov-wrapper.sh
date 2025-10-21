#!/usr/bin/env bash
set -e
if command -v llvm-cov-19 >/dev/null 2>&1; then
  exec llvm-cov-19 gcov "$@"
elif command -v llvm-cov >/dev/null 2>&1; then
  exec llvm-cov gcov "$@"
elif command -v gcov >/dev/null 2>&1; then
  exec gcov "$@"
else
  echo "Error: No suitable gcov tool found (llvm-cov or gcov)" >&2
  exit 1
fi
