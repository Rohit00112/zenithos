# Development Rules

## Code Standards

### C Code (Compositor, Panel, GTK4 Apps)
- Standard: C11
- Compiler: GCC with `-Wall -Wextra -Werror -Wpedantic`
- Naming: `snake_case` for functions and variables
- Prefix: `zenith_` for public API, `_` prefix for private/static
- Memory: Always check allocations, use cleanup attributes where available
- Error handling: Return error codes, log with `g_warning()`/`g_error()`
- Documentation: Doxygen-style comments for public functions
- Format: `clang-format` with project `.clang-format`

### Rust Code (osctl CLI)
- Edition: 2021
- Linting: `clippy` with no warnings
- Format: `rustfmt`
- Error handling: Use `anyhow` for application errors, `thiserror` for library errors
- CLI: Use `clap` derive API

### Build Files
- Meson for C/GTK components
- Cargo for Rust
- Make for top-level orchestration
- Shell scripts: `#!/bin/bash`, `set -euo pipefail`

## Architecture Rules

1. **GUI and CLI must share the same D-Bus APIs**
   - Never implement logic in the GUI that isn't accessible via CLI
   - Never implement logic in the CLI that modifies system state directly

2. **Privileged operations go through D-Bus services**
   - GUI/CLI → D-Bus request → Service (with polkit) → System call
   - Never run GUI apps as root

3. **Components are separate processes**
   - Compositor, panel, dock, launcher, notifications = separate processes
   - Communication via Wayland protocols + D-Bus

4. **No fake functionality**
   - If a button doesn't work, show "Not implemented yet"
   - Never generate fake statistics or placeholder data that looks real
   - Security scores must be based on real system state

5. **Configuration files are TOML**
   - User config: `~/.config/zenith/*.conf`
   - System config: `/etc/zenith/*.conf`
   - Use TOML format for all Zenith-specific config

## Git Conventions

- Commit messages: `component: short description`
  - e.g., `compositor: add workspace switching`
  - e.g., `osctl: implement system status command`
- Branch naming: `phase-N/feature-name`
- PR titles: Same as commit message format

## File Naming

- C source: `snake_case.c` / `snake_case.h`
- Rust source: `snake_case.rs`
- Meson build: `meson.build`
- Scripts: `kebab-case.sh`
- Configs: `kebab-case.conf`
- Docs: `UPPER_CASE.md` for top-level, `kebab-case.md` for subdirectories
