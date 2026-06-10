#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-${ROOT_DIR}/build-static-analysis}"
ALLOW_MISSING_TOOLS="${ALLOW_MISSING_TOOLS:-0}"

missing_tools=()
for tool in cmake cppcheck clang-tidy; do
  if ! command -v "${tool}" >/dev/null 2>&1; then
    missing_tools+=("${tool}")
  fi
done

if [ "${#missing_tools[@]}" -gt 0 ]; then
  printf 'Missing static analysis tools: %s\n' "${missing_tools[*]}" >&2
  if [ "${ALLOW_MISSING_TOOLS}" = "1" ]; then
    printf 'ALLOW_MISSING_TOOLS=1 set; skipping static analysis.\n' >&2
    exit 0
  fi
  exit 127
fi

cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" \
  -DNRF_FIELD_NODE_BUILD_TESTS=ON \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

cppcheck \
  --enable=warning,style,performance,portability \
  --std=c++17 \
  --error-exitcode=1 \
  --inline-suppr \
  --suppress=missingIncludeSystem \
  --suppress=unusedFunction \
  -I "${ROOT_DIR}/include" \
  "${ROOT_DIR}/include" \
  "${ROOT_DIR}/src" \
  "${ROOT_DIR}/tests"

clang-tidy \
  -p "${BUILD_DIR}" \
  "${ROOT_DIR}/src/FieldNode.cpp" \
  "${ROOT_DIR}/src/main.cpp" \
  "${ROOT_DIR}/tests/FieldNodeTests.cpp"
