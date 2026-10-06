#!/bin/bash
# Zenith OS — Clean Build Artifacts
# SPDX-License-Identifier: GPL-3.0-or-later
set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")/.."
make clean
