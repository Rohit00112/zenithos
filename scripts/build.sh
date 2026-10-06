#!/bin/bash
# Zenith OS — Main Build Script
#
# Convenience wrapper around make.
#
# Usage:
#   ./scripts/build.sh          # Build all components
#   ./scripts/build.sh iso      # Build ISO
#   ./scripts/build.sh clean    # Clean
#
# SPDX-License-Identifier: GPL-3.0-or-later

set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")/.."

TARGET="${1:-all}"

echo "========================================="
echo " Zenith OS Build"
echo " Target: $TARGET"
echo "========================================="

make "$TARGET"
