# CI, Static Analysis, and Deploy

The repository uses GitHub Actions to keep the host-buildable embedded project reviewable.

## CI Gates

The `CI, Static Analysis, and Deploy` workflow runs on pushes, pull requests, and manual dispatches.

Build and smoke gates:

- configure CMake with C++17 and host-side tests enabled,
- build `field_node_core`, `nrf52840_bacnet_field_node`, and `field_node_tests`,
- run CTest,
- run CLI smoke checks for status, BACnet objects, alarms, and telemetry.

## Static Code Analysis

Static analysis is centralized in:

```bash
./scripts/run_static_analysis.sh
```

The script runs:

- `cppcheck` with C++17 warning, style, performance, and portability checks,
- `clang-tidy` using the repository `.clang-tidy` profile.

The workflow installs `cppcheck` and `clang-tidy` on the Ubuntu runner before running the script.

## Deploy

Pushes to `main` deploy a generated static evidence site to GitHub Pages after build/test and static analysis pass.

The deploy job runs:

```bash
python3 tools/generate_pages_site.py
```

The generated `site/` directory includes:

- project overview,
- validation and commissioning documentation,
- BACnet object-map notes,
- Draw.io PNG exports and editable sources,
- example evidence files.
