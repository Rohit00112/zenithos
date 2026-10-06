# Zenith OS — Development Guide

## Getting Started

1. Clone the repository
2. Run `./scripts/setup-dev.sh` to install dependencies
3. Run `make all` to build all components
4. Run `make vm` to test in QEMU

## Development Workflow

### Working on the compositor

```bash
cd desktop/zenith-compositor
meson setup builddir
meson compile -C builddir

# Test standalone (requires a running Wayland session or use WLR_BACKENDS=headless)
WLR_BACKENDS=headless ./builddir/zenith-compositor
```

### Working on the panel

```bash
cd desktop/zenith-panel
meson setup builddir
meson compile -C builddir

# Panel requires a running Wayland compositor
./builddir/zenith-panel
```

### Working on the CLI

```bash
cd cli
cargo build
cargo run -- system status
cargo run -- hardware info
cargo test
cargo clippy
```

## Code Style

- **C:** C11, `snake_case`, `zenith_` prefix for public API
- **Rust:** Edition 2021, standard `rustfmt`, `clippy` clean
- **Shell:** `#!/bin/bash`, `set -euo pipefail`
- **Config:** TOML format

See [../../AI_CONTEXT/DEVELOPMENT_RULES.md](../../AI_CONTEXT/DEVELOPMENT_RULES.md) for full coding standards.

## Architecture

See [../architecture/overview.md](../architecture/overview.md) for the system architecture.

## Testing

```bash
make test              # Run all tests
make test-unit         # Unit tests only
make test-integration  # Integration tests only
```

## Commit Messages

Format: `component: short description`

Examples:
```
compositor: add workspace switching
panel: implement clock widget
osctl: add system status command
build: fix ISO generation on Ubuntu
```
